# Usages about Python Codes

## "ResProc.py"
- Used to process results computed by "simulators"
- You should configure results path and save path. "ResRoot": path to results, "WRoot": save path for processed results
- The processed results can be seen in folder "./Example/ProcRes/"

## "Burstiness.py"
- Used to estimate burstiness values for requested contents
- You should configure trace path and save path. "TrRoot": path to traces, "BRoot": save path for burstiness results
- The processed results can be seen in folder "./Example/Burst/"

## "ComLats.py"
- Used to calculate caused latency of the requested content
- You should configure trace path, simulation results path and save path.  "WRoot": save path for results, "Root": path to simulation results, "TrPath": path to traces
- The processed results can be seen in folder "./Example/AvgLats/"

## ”Verification.py“
- Used to evaluate latency under different cases ("Theory", "E-synthetic", "E-real")
- You should configure trace path and save path. "TrRoot": path to traces, "VRoot": save path for evaluation results
- The processed results can be seen in folder "./Example/Verify/"

## Others
- "./Example/Example-Plot.ipynb" gives the methods to use processed data to draw plots
- To run this example code, please configure correct path
