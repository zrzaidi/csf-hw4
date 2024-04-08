#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int compare_i64(const void *left_, const void *right_) {
  int64_t left = *(int64_t *)left_;
  int64_t right = *(int64_t *)right_;
  if (left < right) return -1;
  if (left > right) return 1;
  return 0;
}

void seq_sort(int64_t *arr, size_t begin, size_t end) {
  size_t num_elements = end - begin;
  qsort(arr + begin, num_elements, sizeof(int64_t), compare_i64);
}

// Merge the elements in the sorted ranges [begin, mid) and [mid, end),
// copying the result into temparr.
void merge(int64_t *arr, size_t begin, size_t mid, size_t end, int64_t *temparr) {
  int64_t *endl = arr + mid;
  int64_t *endr = arr + end;
  int64_t *left = arr + begin, *right = arr + mid, *dst = temparr;

  for (;;) {
    int at_end_l = left >= endl;
    int at_end_r = right >= endr;

    if (at_end_l && at_end_r) break;

    if (at_end_l)
      *dst++ = *right++;
    else if (at_end_r)
      *dst++ = *left++;
    else {
      int cmp = compare_i64(left, right);
      if (cmp <= 0)
        *dst++ = *left++;
      else
        *dst++ = *right++;
    }
  }
}

void fatal(const char *msg) __attribute__ ((noreturn));

void fatal(const char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

void merge_sort(int64_t *arr, size_t begin, size_t end, size_t threshold) {
  assert(end >= begin);
  size_t size = end - begin;

  if (size <= threshold) {
    seq_sort(arr, begin, end);
    return;
  }

  // recursively sort halves in parallel

  size_t mid = begin + size/2;

  pid_t left_pid, right_pid;

  left_pid = fork();
  if (left_pid == -1) {
    fatal("ERROR: Failed to fork for left child");
  } else if (left_pid == 0) {
    merge_sort(arr, begin, mid, threshold);
    exit(0);
  }

  right_pid = fork();
  if (right_pid == -1) {
    fatal("ERROR: Failed to fork for right child");
  } else if (right_pid == 0) {
    merge_sort(arr, mid, end, threshold);
    exit(0);
  }

  int wstatus;
  pid_t pid_to_wait_for;
  
  pid_to_wait_for = left_pid;
  pid_t actual_pid = waitpid(pid_to_wait_for, &wstatus, 0);
  if (actual_pid == -1) {
    fatal("Failed to wait for left child");
  }
  if (!WIFEXITED(wstatus)) {
    fatal("Left child process exited abnormally");
  }
  if (WEXITSTATUS(wstatus) != 0) {
    fatal("Left child process returned non-zero exit code");
  }

  pid_to_wait_for = right_pid;
  actual_pid = waitpid(pid_to_wait_for, &wstatus, 0);
  if (actual_pid == -1) {
    fatal("Failed to wait for right child");
  }
  if (!WIFEXITED(wstatus)) {
    fatal("Right child process exited abnormally");
  }
  if (WEXITSTATUS(wstatus) != 0) {
    fatal("Right child process returned non-zero exit code");
  }


  // allocate temp array now, so we can avoid unnecessary work
  // if the malloc fails
  int64_t *temp_arr = (int64_t *) malloc(size * sizeof(int64_t));
  if (temp_arr == NULL)
    fatal("malloc() failed");

  // child processes completed successfully, so in theory
  // we should be able to merge their results
  merge(arr, begin, mid, end, temp_arr);

  // copy data back to main array
  for (size_t i = 0; i < size; i++)
    arr[begin + i] = temp_arr[i];

  // now we can free the temp array
  free(temp_arr);

  // success!
}

int main(int argc, char **argv) {
  // check for correct number of command line arguments
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <filename> <sequential threshold>\n", argv[0]);
    return 1;
  }

  // process command line arguments
  const char *filename = argv[1];
  char *end;
  size_t threshold = (size_t) strtoul(argv[2], &end, 10);
  if (end != argv[2] + strlen(argv[2])) {
    // TODO: report an error (threshold value is invalid)
  }

  // TODO: open the file

  // TODO: use fstat to determine the size of the file

  // TODO: map the file into memory using mmap

  // TODO: sort the data!

  // TODO: unmap and close the file

  // TODO: exit with a 0 exit code if sort was successful

  // open the file
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    fatal("ERROR: Failed to open file");
  }

  // use fstat to determine the size of the file
  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    fatal("ERROR: Failed to get file stats");
  }
  size_t file_size_in_bytes = statbuf.st_size;

  // map the file into memory using mmap
  int64_t *data = mmap(NULL, file_size_in_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    fatal("ERROR: ailed to memory map file");
  }

  // sort the data!
  merge_sort(data, 0, file_size_in_bytes / sizeof(int64_t), threshold);

  // unmap and close the file
  if (munmap(data, file_size_in_bytes) == -1) {
    fatal("ERROR: Failed to unmap memory");
  }
  if (close(fd) == -1) {
    fatal("ERROR: Failed to close file");
  }

  // exit with a 0 exit code if sort was successful
  return 0;

}
