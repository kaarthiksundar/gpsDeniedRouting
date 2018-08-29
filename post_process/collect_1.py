import mmap
import common
import re

numTargets = [20]

discretizationLength = [0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]

threshold = [15]

numLandmarks = 200

instances = {}


resultPath = '../runs/results/'

for t in numTargets:
    instances[t] = {}
    for r in threshold:
        instances[t][r] = {}
        for d in discretizationLength:
            instances[t][r][d] = []
            for instanceCount in range(1, 11):
                instanceName = '{}{}-{}-{}-{}-{}.txt'.format(resultPath, t, numLandmarks, instanceCount, int(d*10), r)
                instances[t][r][d].append(instanceName)


tab = common.Table(1)
tab.header[1] += ['$\Delta$', '$1$', '$2$', '$3$', '$4$', '$5$', '$6$', '$7$', '$8$', '$9$', '$10$']
rowCount = 1
for t in numTargets:
    for r in threshold:
        for d in discretizationLength:
            row = []
            row.append(d)
            for instanceCount in range(1, 11):
                resultFile = instances[t][r][d][instanceCount-1]
                results = common.Results()
                results.populate(resultFile)
                row.append(results.numLMs)
            tab.rows[rowCount] = row
            rowCount += 1

csvOutputFile = 'table-lm-{}.csv'.format(int(d*10))
print 'writing table {}'.format(csvOutputFile)
with open(csvOutputFile, 'w') as file:
    common.writecsv(tab, file)