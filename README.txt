CONTRIBUTIONS

Zayn Zaidi - merge_sort and experiments and analysis
Sean Pak - error handling and file I/O

REPORT

Test run with threshold 2097152

real    0m0.379s
user    0m0.365s
sys     0m0.011s
Test run with threshold 1048576

real    0m0.225s
user    0m0.382s
sys     0m0.023s
Test run with threshold 524288

real    0m0.152s
user    0m0.416s
sys     0m0.022s
Test run with threshold 262144

real    0m0.131s
user    0m0.533s
sys     0m0.050s
Test run with threshold 131072

real    0m0.137s
user    0m0.524s
sys     0m0.075s
Test run with threshold 65536

real    0m0.144s
user    0m0.564s
sys     0m0.081s
Test run with threshold 32768

real    0m0.150s
user    0m0.579s
sys     0m0.113s
Test run with threshold 16384

real    0m0.157s
user    0m0.594s
sys     0m0.165s

We see that the runtime decreases significantly as the threshold decreases until threshold 262144. Then the runtime slightly increases with decreased threshold. The runtime decreases because more processes are sorting the data in parallel, leading two each process taking on less of a load. The runtime might start to increase after a certain point due to increased system resource utilization, since more resources might be required to handle the increased frequency of processing. Therefore, there may be a tradeoff between system resource utilization and processing speed.
