import csv
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib import collections as matcoll
import statistics


attacker_data_file = "20200823-attacker-l1pp-output.csv"
victim_data_file = "20200823-victim-l1pp-output.csv"

"""
                cache1: is a 4 way L1 Data cache 

                    with size 16384 and and 64 sets. The block

                    size is 4096. The line size is 64.

                        Derived parameters: there are 6 offset bits,

                    6 set bits, and 52 tag bits. Address length

                    is 64.
"""

numoffsetbits = 6
numsetbits = 6
numtagbits = 52

setmask = (pow(2,numsetbits)-1)<<numoffsetbits


class ProbedAddress():

    def __init__(self, va, pa, time, type):
        self.va = va
        self.pa_list = []
        self.pa_list.append(pa)
        self.time_list = []
        self.time_list.append(time)
        self.type = type

    def avg_mem_access_time(self):
        total_time = 0
        num_samples = 0
        for time in self.time_list:
            num_samples = num_samples+1
            total_time = total_time+time
        return total_time/num_samples

    def show_va_set(self):
        return (self.va & setmask) >> numoffsetbits

    def show_pa_set(self):
        pa_set = []
        for pa in self.pa_list:
            pa_set.append((pa,(pa&setmask)>>numoffsetbits))
        return pa_set


def cache_set_study(probes, outliers):
    victims = {}
    with open(victim_data_file) as file:
        reader = csv.reader(file)
        for row in reader:
            va = int(row[0],16)
            pa = int(row[1],16)
            time = -1
            victims[va] = ProbedAddress(va,pa,time,'victim')

    print("\n============================")
    print("Our outliers are as follows:")
    print("============================")
    for va in outliers:
        print("virtual addr {:#x} ({:b}) is in set {}\t({:b})".format(probes[va[0]].va, probes[va[0]].va, probes[va[0]].show_va_set(), probes[va[0]].show_va_set()))
        # print("\twith detected physical addrs:")
        # for pa_set in probes[va[0]].show_pa_set():
        #     print("\t\tphys: {:#x} set {}".format(pa_set[0], pa_set[1]))
    print("\n============================")
    print("Our victims are as follows:")
    print("============================")
    for va in victims:
        print("virtual addr {:#x} ({:b}) is in set {}\t({:b})".format(victims[va].va, victims[va].va, victims[va].show_va_set(), victims[va].show_va_set()))



def attacker_va_memaccess_plot(atkdata):
    times = []
    for probe in atkdata:
        times.append(probe[1])
    median = statistics.median(times)
    q1 = np.quantile(times, .25)
    q3 = np.quantile(times, .75)
    upper_bound = ((q3-q1) * 12) + q3

    outliers = []
    for probe in atkdata:
        if probe[1] > upper_bound:
            outliers.append(probe)
            atkdata.remove(probe)
    outliers = sorted(outliers, key=lambda x:x[0])

    x,y = map(list,zip(*atkdata))
    x_outlier, y_outlier = map(list,zip(*outliers))

    plt.stem(x,y, use_line_collection=True, markerfmt=',')
    axes = plt.gca()
    xlabels = map(lambda t: '0x%08X' % int(t), axes.get_xticks())    
    axes.set_xticklabels(xlabels)

    plt.stem(x_outlier,y_outlier, linefmt='r', use_line_collection=True, markerfmt=',')

    for probe in outliers:
        plt.text(probe[0],probe[1], "{:#x} | {}".format(probe[0],probe[1]), fontsize='xx-small')

    plt.title("RISCV L1 Prime and Probe, Probe Memory Access Times")
    plt.xlabel("Probed memory address")
    plt.ylabel("Memory access time (cycles)")
    plt.show()

    return outliers



def analyse_attacker_data():
    probes = {}

    with open(attacker_data_file) as file:
        reader = csv.reader(file)
        for row in reader:
            va = int(row[0],16)
            pa = int(row[1],16)
            time = int(row[2])
            if va in probes:
                probes[va].pa_list.append(pa)
                probes[va].time_list.append(time)
            else:
                probe = ProbedAddress(va,pa,time,'attacker')
                probes[va] = probe
    probe_data = []
    for va in probes:
        print("addr {:#x} ({:b}) is in set {}\t({:b})".format(probes[va].va, probes[va].va, probes[va].show_va_set(), probes[va].show_va_set()))
        probe_data.append((probes[va].va, probes[va].avg_mem_access_time()))
    # We'll get back outliers from the plotting function.
    outliers = attacker_va_memaccess_plot(probe_data)
    cache_set_study(probes, outliers)


def main():
    analyse_attacker_data()


main()