from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import sys
import argparse

parser = argparse.ArgumentParser(description='Arguments')

parser.add_argument('-x', action="store", dest="x_max", type=int, default = 0)
parser.add_argument('-y', action="store", dest="y_max", type=int, default = 0)
parser.add_argument('-l', '--list', help='delimited list input', type=str, default="PMC0")
args = parser.parse_args()

x_max = args.x_max;
y_max = args.y_max;
counter_names = [item for item in args.list.split(',')]

PMC = []
num_cols = 0
with open('stripped_for_plotting.txt') as f:
    lines = f.readlines()
    num_cols = len(lines[0].split())
    cycles = [int(line.split()[0]) for line in lines]

    for i in range(1,num_cols):
        PMC.append([int(line.split()[i]) for line in lines])


fig = plt.figure()
ax = plt.gca()
ax.grid(True)

num_names = len(counter_names)
print(num_names)

for i in range(0,num_cols-1):
    if i < num_names:
        plt.scatter(cycles, PMC[i],s=10, label=counter_names[i])
    else:
        plt.scatter(cycles, PMC[i],s=10, label='PMC'+str(i))

plt.xlabel('Number of Cycles', fontsize='large')
plt.ylabel('Perf Counts', fontsize='large')

plt.legend(loc='upper left')

if x_max != 0:
    plt.xlim([-0.1,x_max])
    if y_max != 0:
        plt.ylim([-0.1,y_max])
    else:
        y_max = 0
        for i in range(0, num_cols-1):
            if y_max < max(PMC[i][1:x_max]):
                y_max = max(PMC[i][1:x_max])
        plt.ylim([-0.1,y_max+5])
if x_max == 0 and y_max != 0:
    plt.ylim([-0.1,y_max])
        
plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.1)
mng = plt.get_current_fig_manager()
mng.resize(1080,1080)
plt.show()
#plt.show(block=False)
#plt.pause(0.01)
#plt.close()
#fig.savefig('nn_k-'+str(z[0])+'.jpg',dpi=300)
