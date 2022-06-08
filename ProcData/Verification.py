import numpy as np
import copy as cp
import time
import random as rd
from sklearn.linear_model import LinearRegression

def load_data(path):
    Data = []
    with open(path) as fr:
        for f in fr:
            data = f[:-1].split(";")
            gdata = [float(data[0]), int(data[1]), int(data[2])]
            Data.append(gdata)
    Len = len(Data)
    if Len >= 2000000:
        Len = 2000000
    return Data[:Len]

class ZipfEst:
    def __init__(self, trace):
        self.trace = trace

    def estimate(self):
        Data = self.trace
        data = {}
        num_data = 0
        for req in Data:
            Id = req[1]
            if Id not in data.keys():
                data[Id] = 1
            else:
                data[Id] += 1
            num_data += 1

        Data = {}
        Sort_Data = sorted(data.items(), key=lambda item: item[1])
        for i in range(len(Sort_Data)):
            id_now = i + 1
            freq_now = Sort_Data[-1 - i][1]
            Data[id_now] = freq_now

        X = []
        Y = []
        for ky in Data.keys():
            X.append([np.log(ky)])
            Y.append([np.log(Data[ky] / num_data)])

        X = np.array(X)
        Y = np.array(Y)

        LrModel = LinearRegression()

        LrModel.fit(X, Y)

        alpha = -LrModel.coef_[0][0]
        return alpha

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

class VerifyEquas:
    def __init__(self, trace, Lms1, Lms2, NLms1, NLms2, netdelay=100, gen=0):
        self.Trace = trace
        self.LMs1 = Lms1
        self.LMs2 = Lms2
        self.NLMs1 = NLms1
        self.NLMs2 = NLms2
        self.NetDelay = netdelay

        self.Perts = {}
        for i in range(len(trace)):
            Id = trace[i][1]
            if Id not in self.Perts.keys():
                self.Perts[Id] = 1
            else:
                self.Perts[Id] += 1

        self.GenDatas = {}
        if gen == 1:
            for ky in self.NLMs2.keys():
                GLambda = self.LMs2[ky]

                Reqs = []
                for i in range(2000):
                    Rd = rd.expovariate(GLambda)
                    Reqs.append(Rd)

                Trace = []
                Timer = 0
                for i in range(2000):
                    Timer += Reqs[i]
                    Trace.append(Timer)

                self.GenDatas[ky] = Trace

            print("Generation Finished...")

    def equa_res(self,PathW):
        UIDs = list(sorted(list(self.NLMs1.keys())))
        Sizes = {}
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            Size = Req[2]
            Sizes[Id] = Size

        Ts = {}
        for ky in Sizes.keys():
            T = self.NetDelay
            Ts[ky] = T

        R1_1 = {}
        R1_2 = {}
        for ky in UIDs:
            GLambda = self.LMs1[ky]
            GT = Ts[ky]
            R1_1[ky] = int(10000 * GLambda * GT ** 2 / 2) / 10000
        for ky in UIDs:
            GLambda = self.LMs2[ky]
            GT = Ts[ky]
            R1_2[ky] = int(GLambda * GT ** 2 / 2 * 10000) / 10000

        R3_1 = {}
        R3_2 = {}
        for ky in UIDs:
            GLambda = self.LMs1[ky]
            GT = Ts[ky]
            R3_1[ky] = int(10000 * (1 + 1 / (1 + GLambda * GT)) * GT / 2) / 10000
        for ky in UIDs:
            GLambda = self.LMs2[ky]
            GT = Ts[ky]
            R3_2[ky] = int(10000 * (1 + 1 / (1 + GLambda * GT)) * GT / 2) / 10000

        R2_1 = 0
        R2_2 = 0
        TotalNums = 0
        for ky in UIDs:
            pert = self.Perts[ky]
            TotalNums += pert

            pert = self.NLMs1[ky]
            Val = R1_1[ky]
            R2_1 += Val * pert

            pert = self.NLMs2[ky]
            Val = R1_2[ky]
            R2_2 += Val * pert

        R4_1 = 0
        R4_2 = 0
        for ky in UIDs:
            pert = self.NLMs1[ky]
            Val = R3_1[ky]
            R4_1 += Val * pert

            pert = self.NLMs2[ky]
            Val = R3_2[ky]
            R4_2 += Val * pert

        TotalNums = 1

        print("Zipf:", R2_1/TotalNums, R4_1/TotalNums)
        print("Overall:", R2_2/TotalNums, R4_2/TotalNums)

        F1 = PathW + "zipf.txt"
        F2 = PathW + "all.txt"

        DW = ""
        for ky in UIDs:
            DW += "E1 " + str(ky) + " " + str(R1_1[ky]) + "\n"
        for ky in UIDs:
            DW += "E3 " + str(ky) + " " + str(R3_1[ky]) + "\n"
        DW += "E2 " + str(R2_1/TotalNums) + "\n"
        DW += "E4 " + str(R4_1/TotalNums) + "\n"

        with open(F1,"w") as fw:
            fw.write(DW)

        DW = ""
        for ky in UIDs:
            DW += "E1 " + str(ky) + " " + str(R1_2[ky]) + "\n"
        for ky in UIDs:
            DW += "E3 " + str(ky) + " " + str(R3_2[ky]) + "\n"
        DW += "E2 " + str(R2_2/TotalNums) + "\n"
        DW += "E4 " + str(R4_2/TotalNums) + "\n"

        with open(F2, "w") as fw:
            fw.write(DW)

    def empi_res(self,PathW):
        UIDs = list(sorted(list(self.NLMs1.keys())))
        Sizes = {}
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            Size = Req[2]
            Sizes[Id] = Size

        Ts = {}
        for ky in Sizes.keys():
            T = self.NetDelay
            Ts[ky] = T

        NTraces = {}
        for i in range(len(self.Trace)):
            Req = self.Trace[i]
            Id = Req[1]
            if Id not in NTraces.keys():
                NTraces[Id] = [Req]
            else:
                NTraces[Id].append(Req)

        Eq1 = {}
        Eq3 = {}
        Eq1_Test = {}

        for ky in UIDs:
            Id = ky
            T = Ts[Id]
            GTrace = NTraces[Id]

            for i in range(len(GTrace)):
                Req = GTrace[i]
                Time = Req[0]
                DLat = []
                for j in range(min(i + 1, len(GTrace)), len(GTrace)):
                    NReq = GTrace[j]
                    NTime = NReq[0]
                    if Time + T >= NTime:
                        DLat.append(Time + T - NTime)
                    if Time + T < NTime:
                        break
                E1 = np.sum(DLat)

                if Id not in Eq1.keys():
                    Eq1[Id] = [E1]
                    Eq3[Id] = [DLat]
                    Eq1_Test[Id] = [DLat]
                else:
                    Eq1[Id].append(E1)
                    Eq3[Id].append(DLat)
                    Eq1_Test[Id].append(DLat)

            GEq3 = Eq3[Id]
            FirstLen = len(GEq3)
            SecondLen = 0
            SecondSum = 0
            GNums = []
            for i in range(len(GEq3)):
                GGEq3 = GEq3[i]
                SecondLen += len(GGEq3)
                SecondSum += np.sum(GGEq3)
                GNums.append(len(GGEq3))
            E3 = T * FirstLen / (FirstLen + SecondLen) + SecondSum / (SecondLen + 0.0001) * SecondLen / (
                    FirstLen + SecondLen)
            Eq3[Id] = E3
            E1 = np.mean(Eq1[Id][:-1])
            Eq1[Id] = E1

        Eq2 = 0
        Eq4 = 0
        TotalNums = 0
        for ky in UIDs:
            pert = self.Perts[ky]
            TotalNums += pert
            l = self.NLMs2[ky]
            val1 = Eq1[ky]
            val2 = Eq3[ky]
            Eq2 += l * val1
            Eq4 += l * val2
        TotalNums = 1
        print("Empirical:", Eq2 / TotalNums, Eq4 / TotalNums)
        F3 = PathW + "empi.txt"

        DW = ""
        for ky in UIDs:
            DW += "E1 " + str(ky) + " " + str(Eq1[ky]) + "\n"
        for ky in UIDs:
            DW += "E3 " + str(ky) + " " + str(Eq3[ky]) + "\n"
        DW += "E2 " + str(Eq2 / TotalNums) + "\n"
        DW += "E4 " + str(Eq4 / TotalNums) + "\n"
        with open(F3, "w") as fw:
            fw.write(DW)

    def req_analy(self, Reqs):
        Eq1 = []
        Eq3 = []
        Z = self.NetDelay
        for i in range(len(Reqs)):
            Time = Reqs[i]
            DLat = []

            for j in range(min(i + 1, len(Reqs)), len(Reqs)):
                NTime = Reqs[j]
                if Time + Z >= NTime:
                    DLat.append(Time + Z - NTime)
                if Time + Z < NTime:
                    break
            E1 = np.sum(DLat)
            Eq1.append(E1)
            Eq3.append(DLat)

        GEq3 = Eq3
        FirstLen = len(GEq3)
        SecondLen = 0
        SecondSum = 0
        GNums = []
        for i in range(len(GEq3)):
            GGEq3 = GEq3[i]
            SecondLen += len(GGEq3)
            SecondSum += np.sum(GGEq3)
            GNums.append(len(GGEq3))
        E3 = Z * FirstLen / (FirstLen + SecondLen) + SecondSum / (SecondLen + 0.0001) * SecondLen / (
                FirstLen + SecondLen)
        Eq3 = E3
        E1 = np.mean(Eq1[:-1])
        Eq1 = E1

        return Eq1, Eq3

    def genempi_res(self,PathW):
        UIDs = list(sorted(list(self.NLMs1.keys())))

        Eq1 = {}
        Eq3 = {}
        count = 0
        for ky in UIDs:
            GTrace = self.GenDatas[ky]
            E1, E3 = self.req_analy(GTrace)
            Eq1[ky] = E1
            Eq3[ky] = E3
            count += 1
            if count % int(len(UIDs)*0.1) == 0:
                print(count/len(UIDs)*100,"% Finished...")

        Eq2 = 0
        Eq4 = 0
        TotalNums = 0
        for ky in UIDs:
            pert = self.Perts[ky]
            TotalNums += pert
            l = self.NLMs2[ky]
            val1 = Eq1[ky]
            val2 = Eq3[ky]
            Eq2 += l * val1
            Eq4 += l * val2
        TotalNums = 1
        print("Gen-Empirical:", Eq2 / TotalNums, Eq4 / TotalNums)

        F4 = PathW + "genempi.txt"
        DW = ""
        for ky in UIDs:
            DW += "E1 " + str(ky) + " " + str(Eq1[ky]) + "\n"
        for ky in UIDs:
            DW += "E3 " + str(ky) + " " + str(Eq3[ky]) + "\n"
        DW += "E2 " + str(Eq2 / TotalNums) + "\n"
        DW += "E4 " + str(Eq4 / TotalNums) + "\n"

        with open(F4, "w") as fw:
            fw.write(DW)


if __name__ == '__main__':
    Names = ["Bilibili","Wikipedia"]
    Netdelays = [1,10,100,200,500,1000,2000,5000,10000]

    TrRoot = "./Example/Traces/"
    VRoot = "./Example//Verify/"

    for Name in Names:
        print(Name)
        PathT = TrRoot + str(Name) + ".csv"
        Trace = load_data(PathT)

        AEst = ZipfEst(Trace)
        Alpha = AEst.estimate()

        GParas = ParaEst(Trace, Alpha, Name)
        GParas.main()

        VEquas = VerifyEquas(Trace, GParas.Lambdas1, GParas.Lambdas2, GParas.NLambdas1, GParas.NLambdas2, 0, 1)

        for d in Netdelays:
            PathW = VRoot + Name + "_" + str(d) + "l_"
            VEquas.NetDelay = d
            VEquas.equa_res(PathW)
            VEquas.empi_res(PathW)
            VEquas.genempi_res(PathW)
            print("-"*10)










