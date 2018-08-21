import re, mmap

numTargets = range(15, 65, 5)

discretizationLength = [1, 2.5, 5]
discretizationLengthInt = [1, 2, 5]

threshold = range(15, 30, 5)

numLandmarks = 200

instances = {}

resultPath = '../runs/results/'

for t in numTargets:
    instances[t] = {}
    for d in discretizationLength:
        instances[t][d] = {}
        for r in threshold:
            instances[t][d][r] = []
            for instanceCount in range(1, 11):
                instanceName = '{}{}-{}-{}-{}-{}.txt'.format(resultPath, t, numLandmarks, instanceCount, int(d), r)
                instances[t][d][r].append(instanceName)

timeoutInstanceCount = 0
timeoutInstanceRuns = []
for t in numTargets:
    for d in discretizationLength:
        for r in threshold:
            for instance in instances[t][d][r]:
                with open(instance, 'r+') as f:
                    data = mmap.mmap(f.fileno(), 0)
                    match = re.search('Feasible', data)
                    if match:
                        timeoutInstanceCount += 1
                        instanceName = re.split('/', instance)[-1]
                        temp = re.split('[-.]', instanceName)
                        timeoutInstanceRuns.append('../bin/main -f {}-{}-{} -p ../instances/ -t {} -d {}'.format(t, numLandmarks, temp[2], r, d))
            
print 'timed-out instances = {}'.format(timeoutInstanceCount)
timeoutFile = open('timeout_runs', 'w')
timeoutFile.write('\n'.join(timeoutInstanceRuns))
timeoutFile.close()


