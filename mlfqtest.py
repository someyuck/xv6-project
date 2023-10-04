import matplotlib.pyplot as plt
import numpy as np

PQSlices = [1, 3, 9, 15]

with open("mlfqAnalysis.txt", "r") as fin:
    rawData = fin.read()

rawData = rawData.strip().split('\n')

# pairs of (tick, pid, process queue)
rawData = [(int(x.split()[0]), int(x.split()[1]), int(x.split()[2])) for x in rawData]

# get list of unique pids
temp_pids = [x[1] for x in rawData]
pids = list()

for x in temp_pids:
    if len(pids) == 5:
        break
    if x not in pids:
        pids.append(x)
pids.sort()

# other pids may have entered (for instance the pid of the mlfqtest process)
# for i in range(len(rawData)): 
#     if rawData[i][1] not in pids:
#         print("hey")
#         del rawData[i]

processes = list()
for i in range(len(pids)):
    processes.append(list())

for i in range(len(rawData)):
    tick = rawData[i][0]
    pid = rawData[i][1]
    pqueue = rawData[i][2]
    if processes[pids.index(pid)] == []:
        processes[pids.index(pid)].append((rawData[0][0]-1, pqueue)) # reflect creation of process

    processes[pids.index(pid)].append((tick, pqueue))

# now we have a list of cpu ticks against queue numbers for each process, including demotions and promotions
# think Y : queue number, X : tick number (need not start from 0)

def makePlot(processes: list[list[tuple[int, int]]]):
    # plt.figure(figsize = (150, 75))

    for proc in processes:
        ticks = [x[0] for x in proc]
        pQs = [x[1] for x in proc]
        plt.plot(ticks, pQs)
    
    runTicks = [x[0] for x in rawData if x[-1] == "run"]
    runTicksQueues = [x[2] for x in rawData if x[-1] == "run"]
    plt.scatter(runTicks, runTicksQueues)
    
    plt.legend(labels = ['P1', 'P2', 'P3', 'P4', 'P5'])
    plt.xlabel("CPU Ticks")
    plt.ylabel("Priority Queue Number")
    plt.title("MLFQ Analysis with aging time 30s")
    plt.savefig("aging50.png")
    plt.show()

makePlot(processes)
