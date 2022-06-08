import numpy as np

class Proc:
    def __init__(self,ResPath, CSize=256):
        self.RPath = ResPath
        self.CSize = CSize

        self.Algos = {"Synthetic": ["TwoQ", "LRUK", "LHD", "LRU", "Belady", "AD"],
                      "Bilibili": ["LA",  "LRUAD", "LHDAD", "TwoQ", "LRUK", "LHD", "LRU", "Belady", "AD"],
                      "Wikipedia": ["LA", "LRUAD", "LHDAD", "LRU"]
                      }

        self.Latency = [1, 10, 100, 1000, 2000, 5000, 10000, 20000, 50000, 100000]

    def getRes(self, Name):
        Get = []
        Algos = self.Algos[Name]
        DAs = ["LA", "LRUAD", "LHDAD", "LRU"]

        for l in self.Latency:
            for a in Algos:
                Csize = [self.CSize]
                if Name != "Synthetic" and l == 1000 and a in DAs:
                    Csize = [8, 16, 32, 64, 128, 256, 512]

                for c in Csize:
                    File = a + "Cache_" + str(c) + "c_" + str(l) + "l.txt"
                    #Files.append(File)
                    Path = self.RPath + Name + "/" + File
                    Fname = a + "-" + str(c) + "-" + str(l)
                    DWs = ""
                    with open(Path) as fr:
                        C = 0
                        for f in fr:
                            data = f[:-1].split(":")[1]
                            DWs += data + " "
                            C += 1
                            if C >= 3:
                                break

                    Get.append(Fname + " " + DWs[:-1] + "\n")

        return Get


if __name__ == '__main__':
    Names = ["Synthetic","Bilibili","Wikipedia"]
    
    ResRoot = "./Results/"
    WRoot = "./Example/ProcRes/"
    
    CacheSize = 256

    PC = Proc(ResRoot, CacheSize)
    for name in Names:
        Gs = PC.getRes(name)
        WPath = WRoot + name + ".txt"
        with open(WPath,"a+") as fw:
            for g in Gs:
                fw.write(g)










