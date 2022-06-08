import numpy as np

def load_data(path):
    Datas = []
    with open(path) as fr:
        count = 0
        for f in fr:
            count += 1
            if count > 5:
                data = f[:-1]
                if data != "":
                    Datas.append(int(data))

    return Datas

def load_trace(path):
    IDs = []
    with open(path) as fr:
        for f in fr:
            data = f[:-1].split(";")
            id = int(data[1])
            IDs.append(id)
    return IDs

def estlats(IDs,LTs):
    Latency = {}
    for i in range(len(IDs)):
        Id = IDs[i]
        Lt = LTs[i]
        if Id not in Latency.keys():
            Latency[Id] = [Lt]
        else:
            Latency[Id].append(Lt)

    for ky in Latency.keys():
        Latency[ky] = np.mean(Latency[ky])
    Res = ""
    for ky in Latency.keys():
        Res += str(ky) + " " + str(Latency[ky]) + "\n"
    return Res

def getRes(Name,L=10000,S=512):
    Algos = ["LA", "LRUAD", "LHDAD", "LRU"]
    Root = "./Results/"

    Ls = {}
    for a in Algos:
        Path = Root + Name + "/" + a + "Cache_" + str(S) + "c_" + str(L) + "l.txt"
        Lats = load_data(Path)
        Ls[a] = Lats

    TrPath = "./Example/Traces/" + Name + ".csv"
    IDs = load_trace(TrPath)

    Res = {}
    for a in Algos:
        Res[a] = estlats(IDs,Ls[a])

    return Res

if __name__ == '__main__':
    Names = ["Bilibili", "Wikipedia"]
    CacheSize = [256,512]
    WRoot = "./Example/AvgLats/"

    for name in Names:
        for C in CacheSize:
            print(name)
            Gets = getRes(name,S=C)
            Path = WRoot + name + "/"
            Kys = Gets.keys()
            for ky in Kys:
                PathW = Path + ky + "Cache_" + str(C) + ".txt"
                with open(PathW,"w") as fw:
                    fw.write(Gets[ky])



