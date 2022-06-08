# Delayed-Hits Simulator
Here provides the basic realizations of algorithms under delayed-hits case. The simulator here contains components described in our paper "Towards Latency Awareness for Content Delivery Network Caching".


# Usage
1. Requirement: Ubuntu 18.04, "GCC 7.0+", "CMake 3.10+", "Boost C++ v1.70+"

2. The "./Simulator" contains codes and scripts to run simulations. More details can be found in "./Simulator/README.md"

3. The "./Prototype" includes codes about apache trafficserver and prototypes. More details can be found in "./Prototype/README.md"

4. The "./ProcData" is used to process simulation results and get some results to draw plots. Please go to "./ProcData/README.md" to get more details

5. Folder "./Example" gives the all processed data and a file named "Example-Plots.ipynb" which uses the data to draw plots

# Trace Format
Request traces are expected to be in a space-separated format with 3 columns: request time, content id and content size.

| Time | ID | Size |
|:----:|:----:|:----:|
| 0 | 1 | 1024 |
| 1 | 23 | 1024 |
| 2 | 15 | 2048 |
| 3 | 10 | 1024 |


# Experiments
We have implemented LA-Cache and a bunch of state-of-the-arts in simulators including but not limited to:
- LA-Cache
- LRU-MAD
- LHD-MAD
- LRU
- LHD
- LRU-K
- 2Q
- LFU
- Belady
- Offline Delay

To draw the plots in our paper, you should install Python3.0+ at first. More details can be found in folder "./Example".


# Citation
If you use the simulator or some results in our paper for a published project, please cite our work by using the following bibtex entry

```
@inproceedings{yan2022towards,
  title={Towards Latency Awareness for Content Delivery Network Caching},
  author={Yan, Gang and Li, Jian},
  booktitle={Proc. of USENIX ATC},
  year={2022}
}
```
