import os
import time

class Run:
    def __init__(self, Cmdroot, CSize):
        self.CSize = CSize

        self.Algos = {"Synthetic":["2q","lruk","lhd","lru","belady","aggdelay"],
                      "Bilibili":["la","lru_aggdelay","lhd_aggdelay","2q","lruk","lhd","lru","belady","aggdelay"],
                      "Wikipedia":["la","lru_aggdelay","lhd_aggdelay","lru"]
        }

        self.Latency =  [1,10,100,1000,2000,5000,10000,20000,50000,100000]

        self.CmdRoot = Cmdroot
        self.TracePath = None
        self.OutPath = None

    def setpath(self,trpath,outpath):
        self.TracePath = trpath
        self.OutPath = outpath

    def getCmds(self):
        CMDs = []
        Name = self.TracePath.split("/")[-1]
        Name = Name.split(".")[0]
        Algos = self.Algos[Name]

        for l in self.Latency:
            for a in Algos:
                Cmd = self.CmdRoot + "/cache_" + a + " --trace " + self.TracePath + " --csize " + str(self.CSize) + " --latency " + str(l) + " --outpath " + self.OutPath
                CMDs.append(Cmd)

        return CMDs

    def getSCmds(self):
        CSizes = [8,16,32,64,128,256,512]
        FixL = 1000
        As = ["la","lru_aggdelay","lhd_aggdelay","lru"]
        CMDs = []

        for c in CSizes:
            for a in As:
                Cmd = self.CmdRoot + "/cache_" + a + " --trace " + self.TracePath + " --csize " + str(c) + " --latency " + str(FixL) + " --outpath " + self.OutPath
                CMDs.append(Cmd)

        return CMDs


if __name__ == '__main__':
    CmdRoot = "./Delayed-Source-Code/build/bin/"
    CacheSize = 256  # Cache Size
    GenCmds = Run(CmdRoot, CacheSize)

    TrRoot = "./Example/Traces/"
    OutRoot = "./Results/"

    Names1 = ["Synthetic"]
    Names2 = ["Bilibili"]
    Names3 = ["Wikipedia"]
    TraceName = Names1 + Names2 + Names3

    # -------------------------------
    STime = time.time()

    All = []
    for name in TraceName:
        TrPath = TrRoot + name + ".csv"
        OutPath = OutRoot + name + "/"
        GenCmds.setpath(TrPath,OutPath)

        CMDs = GenCmds.getCmds()
        All += CMDs
        if name != "Synthetic":
            CMDs = GenCmds.getSCmds()
            All += CMDs

    print("* There are {:.0f} commands in total".format(len(All)))
    print("")
    time.sleep(3)

    for c in All:
        print(c)
        os.system(c)

    ETime = time.time()
    print("* Running time: {:.2f} h".format((ETime - STime) / 3600))

