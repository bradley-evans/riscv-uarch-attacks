
FILENAME = "20200911-hitvmisstests.csv"


class Address():

    def __init__(self, va, pa, memacctime, group):
        self.va = va
        self.pa = pa
        self.memacctimes = []
        self.memacctimes.append(int(memacctime))
        self.group = group
        self.longest_time = int(memacctime)
        self.shortest_time = int(memacctime)
        self.avg_time = int(memacctime)

    def addTime(self, newtime):
        self.memacctimes.append(int(newtime))
        if int(newtime) > self.longest_time:
            self.longest_time = int(newtime)
        if int(newtime) < self.shortest_time:
            self.shortest_time = int(newtime)
        self.avg_time = self.computeAvg()

    def computeAvg(self):
        total_time = 0
        for time in self.memacctimes:
            total_time = time + total_time
        return total_time / len(self.memacctimes)


def parse(filename):
    import csv

    group = [{},{},{},{},{}]

    with open(filename) as file:
        reader = csv.reader(file)
        for row in reader:
            curr_group = int(row[3])
            curr_addr = int(row[0], 16)
            if curr_addr in group[curr_group]:
                group[curr_group][curr_addr].addTime(row[2])     
            else:
                group[curr_group][curr_addr] = Address( va = curr_addr,
                                                        pa = row[1],
                                                        memacctime = row[2],
                                                        group = row[3] )
    return group


def graph(addresses):
    curr_group = 0
    for address_dict in addresses:
        print("**********************")
        print("** Group {}".format(curr_group))
        print("**********************")
        for address in address_dict:
            print("addr:{:#x} | max: {} | min: {} | avg: {:.2f}".format(
                address_dict[address].va,
                address_dict[address].longest_time,
                address_dict[address].shortest_time,
                address_dict[address].avg_time
                ))
        curr_group = curr_group + 1



def analyse(filename):
    addresses = parse(filename)
    graph(addresses)


if __name__== "__main__":
    analyse(FILENAME)

