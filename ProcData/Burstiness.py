import numpy as np
import copy as cp


def load_data(path):
    Data = []
    with open(path) as fr:
        for f in fr:
            data = f[:-1].split(";")
            gdata = [float(data[0]), int(data[1]), int(data[2])]
            Data.append(gdata)
    Len = len(Data)
    if Len > 2000000:
        Len = 2000000
    return Data[:Len]

class ParaEst:
    def __init__(self, trace, alpha=0.8, name = ""):
        self.Trace = trace
        self.Alpha = alpha
        self.Name = name
        self.Lambdas1 = {}
        self.NLambdas1 = {}
        self.Lambdas2 = {}
        self.NLambdas2 = {}

    def zipfest(self):
        IDs = []
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            IDs.append(Id)

        UIDs = list(np.unique(IDs))
        UIDs = list(sorted(UIDs))
        Lambdas = {}
        Sum = 0
        for I in UIDs:
            Sum += 1 / pow(I, self.Alpha)

        C = 1 / Sum
        for I in UIDs:
            L = C / pow(I, self.Alpha)
            Lambdas[I] = L

        self.Lambdas1 = cp.deepcopy(Lambdas)

    def allest(self):
        Times = {}
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            Time = Req[0]
            if Id not in Times:
                Times[Id] = [Time]
            else:
                Times[Id].append(Time)

        Lambdas = {}
        for ky in Times.keys():
            GTimes = Times[ky]
            Inter = []
            if len(GTimes) > 1:
                for i in range(len(GTimes) - 1):
                    Inter.append(GTimes[i + 1] - GTimes[i])

                Lambdas[ky] = 1 / np.mean(Inter)

        self.Lambdas2 = cp.deepcopy(Lambdas)

    def main(self):
        Nums = {'Bilibili': 639, 'Wikipedia': 18044}
        Num = Nums[self.Name]
        self.zipfest()
        self.allest()

        LMs1 = self.Lambdas1
        LMs2 = self.Lambdas2
        Len = Num
        GIDs = list(sorted(LMs1.keys()))[:Len]

        NLMs1 = {}
        NLMs2 = {}

        Sum1 = 0
        Sum2 = 0
        for ky in GIDs:
            Sum1 += LMs1[ky]
            Sum2 += LMs2[ky]

        for ky in GIDs:
            val1 = LMs1[ky] / Sum1
            val2 = LMs2[ky] / Sum2
            NLMs1[ky] = val1
            NLMs2[ky] = val2

        Sum1 = 0
        Sum2 = 0
        for ky in GIDs:
            Sum1 += NLMs1[ky]
            Sum2 += NLMs2[ky]

        self.NLambdas1 = cp.deepcopy(NLMs1)
        self.NLambdas2 = cp.deepcopy(NLMs2)

        print("Finished...")


class BAnalysis:
    def __init__(self,trace):
        self.Trace = cp.deepcopy(trace)
        self.Times = {}
        self.Num = 0
        self.UIDs = None
        self.process()

    def process(self):
        Times = {}
        Init = self.Trace[0][0]
        End = self.Trace[-1][0]
        IDs = []
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            Time = Req[0]
            if Id not in Times.keys():
                Times[Id] = [Time]
            else:
                Times[Id].append(Time)
            IDs.append(Id)

        self.UIDs = list(sorted(np.unique(IDs)))

        Inters = {}
        for ky in Times.keys():
            GTimes = Times[ky]
            inter = []
            if len(GTimes) > 1:
                for i in range(len(GTimes)-1):
                    inter.append(GTimes[i+1]-GTimes[i])
            Inters[ky] = inter
            self.Num += 1
        self.Times = cp.deepcopy(Inters)

    def get_burst(self,Times):
        Sigma = np.sqrt(np.var(Times))
        Mean = np.mean(Times)
        R = Sigma / Mean
        B1 = (R-1)/(R+1)
        n = len(Times)
        B2 = (np.sqrt(n + 1) * R - np.sqrt(n - 1)) / ((np.sqrt(n + 1) - 2) * R + np.sqrt(n - 1))
        return B1,B2

    def estimate(self,Probs):
        BSum1 = 0
        BSum2 = 0
        BS = {}
        SumReqs = 0
        for ky in Probs.keys():
            GTimes = self.Times[ky]
            L = len(GTimes)
            SumReqs += L

        Means = []
        Deviations = []
        for ky in Probs.keys():
            GTimes = self.Times[ky]
            L = len(GTimes)

            B1, B2 = self.get_burst(GTimes)
            devi = np.std(GTimes)
            mea = np.mean(GTimes)
            Deviations.append(devi*L)
            Means.append(mea*L)

            BS[ky] = [B1,B2]
            L = len(GTimes)
            BSum1 += L * B1
            BSum2 += L * B2

        R = np.sum(Deviations) / np.sum(Means)
        BL = (R-1)/(R+1)

        return BSum1/SumReqs,BSum2/SumReqs,BS


if __name__ == '__main__':
    Names = ["Bilibili","Wikipedia"]
    TrRoot = "./Traces/"
    BRoot = "./Example/Burst/"

    for Name in Names:
        print(Name)
        PathT = TrRoot + Name + ".csv"
        PathW = BRoot + Name + "_b.txt"

        Trace = load_data(PathT)

        GParas = ParaEst(Trace,name=Name)
        GParas.main()

        BEst = BAnalysis(Trace)
        S1, S2, BS = BEst.estimate(GParas.NLambdas2)

        DW = str(S1) + " " + str(S2) + "\n"
        for ky in BS.keys():
            dw = str(ky) + " " + str(BS[ky][0]) + " " + str(BS[ky][1]) + "\n"
            DW += dw

        with open(PathW, "w") as fw:
            fw.write(DW)














