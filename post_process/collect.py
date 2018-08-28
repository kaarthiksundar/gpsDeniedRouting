import mmap
import common
import re

numTargets = range(15, 65, 5)

discretizationLength = [1, 2.5, 5]
discretizationLengthInt = [1, 2, 5]

threshold = range(15, 30, 5)

numLandmarks = 200

instances = {}

success = {}
timeoutInstanceCount = 0
timeoutInstanceRuns = []

resultPath = '../runs/results/'

for t in numTargets:
    instances[t] = {}
    success[t] = {}
    for d in discretizationLength:
        instances[t][d] = {}
        success[t][d] = {}
        for r in threshold:
            instances[t][d][r] = []
            success[t][d][r] = []
            for instanceCount in range(1, 11):
                instanceName = '{}{}-{}-{}-{}-{}.txt'.format(resultPath, t, numLandmarks, instanceCount, int(d), r)
                instances[t][d][r].append(instanceName)
                with open(instanceName, 'r+') as f:
                    data = mmap.mmap(f.fileno(), 0)
                    match_f = re.search('Feasible', data)
                    match_o = re.search('Optimal', data)
                    if match_f: 
                        timeoutInstanceCount += 1
                        fileName = re.split('/', instanceName)[-1]
                        temp = re.split('[-.]', fileName)
                        timeoutInstanceRuns.append('../bin/main -f {}-{}-{} -p ../instances/ -t {} -d {}'.format(t, numLandmarks, temp[2], r, d))
                        success[t][d][r].append(0)
                    if match_o:
                        success[t][d][r].append(1)


print 'timed-out instances = {}'.format(timeoutInstanceCount)
# timeoutFile = open('timeout_runs', 'w')
# timeoutFile.write('\n'.join(timeoutInstanceRuns))
# timeoutFile.close()

# paper table parameters

numTargets = range(20, 65, 10)

discretizationLength = [1, 2.5]
discretizationLengthInt = [1, 2]

threshold = [15, 20]

numLandmarks = 200

# table 1: success count
tab1 = common.Table(2)
tab1.header[1] += ['$|T|$', 'Succ', 'Succ', 'Succ', 'Succ']
tab1.header[2] += ['']
for d in discretizationLength:
    for r in threshold:
        tab1.header[2].append('$|R| = {} \! |D| = {}$'.format(r, d))

for i in range(0, len(numTargets)):
    t = numTargets[i]
    row = [t]
    for d in discretizationLength:
        for r in threshold:
            row.append(sum(success[t][d][r]))
    tab1.rows[i+1] = row
tab1.tableCaption = 'Number of runs for which the branch-and-cut algorithm \
                    was able to compute an optimal solution within the computation time limit'

csvOutputFile = 'table1.csv'
print 'writing success count table to: {}'.format(csvOutputFile)

with open(csvOutputFile, 'w') as file:
    common.writecsv(tab1, file)
            
# tables 2-6:  all comprehensive results
import itertools
combinations = [element for element in itertools.product(discretizationLength, threshold)]

runTimes = {t: [[], [], [], []] for t in numTargets}
numLMs = {t: [[], [], [], []] for t in numTargets}
indexMap = { combinations[i]: i for i in range(0, len(combinations))}
indexMapReverse = { i: combinations[i] for i in range(0, len(combinations))}

for (d, r) in combinations:
    tab = common.Table(1)
    tab.header[1] += ['$|T|$', '$n$', 'time', 'numLMs', 't-cost', 'p-cost', 'obj', 'gap', 'sec']
    rowCount = 1
    for t in numTargets:
        for instanceCount in range(1, 11):
            row = []
            resultFile = instances[t][d][r][instanceCount-1]
            results = common.Results()
            results.populate(resultFile)
            row.append(t)
            row.append(instanceCount)
            row.append('{:5.2f}'.format(results.time))
            if results.time <= 7199:
                runTimes[t][indexMap[(d, r)]].append(results.time)
            row.append(results.numLMs)
            numLMs[t][indexMap[(d, r)]].append(results.numLMs)
            row.append('{:5.2f}'.format(results.travelCost))
            row.append('{:5.2f}'.format(results.placementCost))
            row.append('{:5.2f}'.format(results.objective))
            row.append('{:0.4f}'.format(results.gap))
            row.append(results.cuts)
            tab.rows[rowCount] = row
            rowCount += 1
    csvOutputFile = 'table-{}-{}.csv'.format(int(d), r)
    print 'writing table {}'.format(csvOutputFile)
    with open(csvOutputFile, 'w') as file:
        common.writecsv(tab, file)


# plot run times
import matplotlib.pyplot as plt
import numpy as np

def set_box_color(bp, color, ls):
    plt.setp(bp['boxes'], color='black', ls=ls, lw=1)
    plt.setp(bp['whiskers'], color='black', lw=1)
    plt.setp(bp['caps'], color='black', lw=1)
    plt.setp(bp['medians'], lw=1)
    for patch, c in zip(bp['boxes'], color):
        patch.set_facecolor(color)

plt.rcParams["font.family"] = "Times New Roman"
plt.rc('text', usetex=True)
labels = [r'$|T|=20$', r'$|T|=30$', r'$|T|=40$', r'$|T|=50$', r'$|T|=60$']

# class a instances plot
fig, ax = plt.subplots()

values = [runTimes[t][0] for t in numTargets]
position = [1, 6, 11, 16, 21]
bplot_1 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_1, 'lightblue', '-')


values = [runTimes[t][1] for t in numTargets]
position = [2, 7, 12, 17, 22]
bplot_2 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_2, 'lightgreen', '-')

values = [runTimes[t][2] for t in numTargets]
position = [3, 8, 13, 18, 23]
bplot_3 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_3, 'lightgray', '-')

values = [runTimes[t][3] for t in numTargets]
position = [4, 9, 14, 19, 24]
bplot_4 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_4, 'wheat', '-')

ax.grid(linestyle='--')
ax.set_yscale('log')
ax.set_xlim(0, 25)
ax.set_xticklabels(labels)
ax.set_ylabel(r'Computation time in seconds')
ax.set_xticks([2.5, 7.5, 12.5, 17.5, 22.5])
d, r = indexMapReverse[0]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.74, 0.35, figtext, backgroundcolor='lightblue', color='black')
d, r = indexMapReverse[1]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.74, 0.28, figtext, backgroundcolor='lightgreen', color='black')
d, r = indexMapReverse[2]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.73, 0.21, figtext, backgroundcolor='lightgray', color='black')
d, r = indexMapReverse[3]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.73, 0.14, figtext, backgroundcolor='wheat', color='black')
plt.savefig('runTimes.pdf', format='pdf')

# plot numLMs
plt.rcParams["font.family"] = "Times New Roman"
plt.rc('text', usetex=True)
labels = [r'$|T|=20$', r'$|T|=30$', r'$|T|=40$', r'$|T|=50$', r'$|T|=60$']

# class a instances plot
fig, ax = plt.subplots()

values = [numLMs[t][0] for t in numTargets]
position = [1, 6, 11, 16, 21]
bplot_1 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_1, 'lightblue', '-')


values = [numLMs[t][1] for t in numTargets]
position = [2, 7, 12, 17, 22]
bplot_2 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_2, 'lightgreen', '-')

values = [numLMs[t][2] for t in numTargets]
position = [3, 8, 13, 18, 23]
bplot_3 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_3, 'lightgray', '-')

values = [numLMs[t][3] for t in numTargets]
position = [4, 9, 14, 19, 24]
bplot_4 = ax.boxplot(values, positions=position, sym='+', widths=0.5,  vert=True, patch_artist=True)
set_box_color(bplot_4, 'wheat', '-')


ax.grid(linestyle='--')
ax.set_xlim(0, 25)
ax.set_xticklabels(labels)
ax.set_ylabel(r'Number of landmarks placed')
ax.set_xticks([2.5, 7.5, 12.5, 17.5, 22.5])
d, r = indexMapReverse[0]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.74, 0.55, figtext, backgroundcolor='lightblue', color='black')
d, r = indexMapReverse[1]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.74, 0.48, figtext, backgroundcolor='lightgreen', color='black')
d, r = indexMapReverse[2]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.73, 0.21, figtext, backgroundcolor='lightgray', color='black')
d, r = indexMapReverse[3]
figtext = r'$\Delta={},\,r={}$'.format(d, r)
plt.figtext(0.73, 0.14, figtext, backgroundcolor='wheat', color='black')
plt.savefig('numLMs.pdf', format='pdf')

