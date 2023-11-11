import matplotlib.pyplot as plt
import numpy as np

with open("pbstest.txt", "r") as fin:
    rawData = fin.read()

rawData = rawData.strip().split('\n')

# yuples of (tick, pid, statPri, DynPri)
rawData = [(int(x.split()[0]), int(x.split()[1]), int(x.split()[2]), int(x.split()[3])) for x in rawData]

# get list of unique pids
temp_pids = [x[1] for x in rawData]
pids = list()

for x in temp_pids:
    if len(pids) == 10:
        break
    if x not in pids:
        pids.append(x)
pids.sort()

processes = list()
for i in range(len(pids)):
    processes.append(list())

for i in range(len(rawData)):
    tick = rawData[i][0]
    pid = rawData[i][1]
    dp = rawData[i][3]

    processes[pids.index(pid)].append((tick, dp))

# now we have a list of cpu ticks against dynamic priorities for each process
# think Y : dynamic priority, X : tick number (need not start from 0)

def makePlot(processes: list[list[tuple[int, int]]]):
    for proc in processes:
        ticks = [x[0] for x in proc]
        dps = [x[1] for x in proc]
        # plt.scatter(ticks, dps, s = [5 for x in dps])
        plt.plot(ticks, dps)
    
    plt.legend(labels = ['P1', 'P2', 'P3', 'P4', 'P5', 'P6', 'P7', 'P8', 'P9', 'P10'])
    plt.xlabel("CPU Ticks")
    plt.ylabel("Dynamic Priority")
    plt.title("PBS Analysis")
    plt.savefig("pbsplots/pbs1line.png") # change file name if pbstest is run with other options (0 or pbs1), or if it is a plt.plot or plt.scatter plot
    plt.show()

makePlot(processes)
