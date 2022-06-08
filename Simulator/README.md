# Usages about Simulator

# 1. Installation
- Requirement: Ubuntu 18.04

```
sudo apt-get update
sudo apt-get install g++-[7+]
sudo apt-get install make
sudo snap install cmake --classic 
```
Download Boost C++ (v.1.70+) [here](https://www.boost.org/users/download), and extract the archive to a location. In the top-level directory, run following commands:
```
cd ./boost_1_79_0/
./bootstrap.sh --with-libraries=program_options
sudo ./b2 -j12 toolset=gcc variant=release threading=multi address-model=64 cxxflags=-std=c++17 install
```

Then compiling this code:
```
cd ./Delayed-Source-Code/
mkdir build && cd build
cmake ..
make
```

In the last, you can find the implementations in the path
```
./Delayed-Source-Code/build/bin/
```

# 2. Run Simulator
- We provide the simulators in the folder "./Simulator/simulators/", and you can also generate them according to the above steps
- To use the simulators, you should run commands in the terminal like:
```
./Delayed-Source-Code/build/bin/cache_lru --trace [trace path] --csize [cache size (GB)] --latency [network latency] --outpath [path to save results]
```

This is about "LRU" algorithm, other algorithms have the same usage.


# 3. Easy Running
- To run experiments conveniently, we provide a code named "Runs.py"
- To implement the code, you should install Python3.0+
- Before using it, configure paths to trace, results save and simulators. Trace Path: "TrRoot", Output Path: "OutRoot", and Simulator Path: "CmdRoot"
- Run
```
python3 ./Simulator/Runs.py
```

# 4. Others
- Source Codes about algorithms can be found in folder "./Delayed-Source-Code/caching/"








