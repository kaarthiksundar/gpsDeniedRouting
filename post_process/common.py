import csv
import re

class Table():
    def __init__(self):
        self.numHeaderRows = 1
        self.header = {}
        self.rows = {}
        self.tableCaption = ''
    
    def __init__(self, numHeaderRows):
        self.numHeaderRows = numHeaderRows
        self.header = {i: [] for i in range(1, numHeaderRows+1)}
        self.rows = {}
        self.tableCaption = ''
    
def writecsv(tab, outfile):
    writer = csv.writer(outfile, delimiter = ',', quoting=csv.QUOTE_NONE)
    for (key, val) in tab.header.iteritems():
        writer.writerow(val)
    for (key, val) in tab.rows.iteritems():
        writer.writerow(val)
    return

class Results():
    def __init__(self):
        time = 0.0
        numLMs = 0
        travelCost = 0.0
        placementCost = 0.0
        objective = 0.0
        gap = 0.0
        cuts = 0

    def populate(self, fileName):
        f = open(fileName, 'r')
        for line in f:
            if re.search('computation time:', line):
                self.time = float(re.split('\s+', line)[-3])
            if re.search('number of landmarks placed:', line):
                self.numLMs = int(re.split('\s+', line)[-2]) 
            if re.search('travel cost:', line):
                self.travelCost = float(re.split('\s+', line)[-2])
            if re.search('placement cost:', line):
                self.placementCost = float(re.split('\s+', line)[-2])
            if re.search('total cost:', line):
                self.objective = float(re.split('\s+', line)[-2])
            if re.search('number of user cuts added:', line):
                self.cuts = int(re.split('\s+', line)[-2])
            if re.search('optimality gap', line):
                self.gap = float(re.split('\s+', line)[-2])
            else:
                self.gap = 0.0000
        return