#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue May 28 03:32:37 2024

@author: vamsi
"""

import numpy as np
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
import  random
from matplotlib.colors import LogNorm, Normalize
import matplotlib.cm as cm
import matplotlib.colors as mcolors
import pandas as pd
import numpy as np
import sys
import pylab
#%%
dump="dump_vpi/"
results="results_vpi/"
plots="/home/vamsi/src/phd/writings/ml-transport/plots/"

DCQCNCC=1
INTCC=3
TIMELYCC=7
PINTCC=10
CUBIC=2
DCTCP=4
DCTCPCC=8

FLOW_ECMP=0
RANDOM_ECMP=1
SOURCE_ROUTING=2
REPS=3

ALL_TO_ALL=666
ALL_REDUCE=667

RING=777
TREE=778


RDMACC=DCTCPCC


colors={}

colors[FLOW_ECMP]={}
colors[FLOW_ECMP][True]='r'
colors[FLOW_ECMP][False]='y'

colors[RANDOM_ECMP]={}
colors[RANDOM_ECMP][True]='g'
colors[RANDOM_ECMP][False]='b'

colors[SOURCE_ROUTING]={}
colors[SOURCE_ROUTING][True]='k'

labels={}

labels[FLOW_ECMP]={}
labels[FLOW_ECMP][True]='Ecmp-permute'
labels[FLOW_ECMP][False]='Ecmp'

labels[RANDOM_ECMP]={}
labels[RANDOM_ECMP][True]='Spray-permute'
labels[RANDOM_ECMP][False]='Spray'

labels[SOURCE_ROUTING]={}
labels[SOURCE_ROUTING][True]='SR-permute'

markers={}

markers[FLOW_ECMP]={}
markers[FLOW_ECMP][True]='o'
markers[FLOW_ECMP][False]='o'

markers[RANDOM_ECMP]={}
markers[RANDOM_ECMP][True]='s'
markers[RANDOM_ECMP][False]='s'

markers[SOURCE_ROUTING]={}
markers[SOURCE_ROUTING][True]='D'

TRANSFER_SIZES=[8000,16000,32000,64000,128000,256000]
QP_WINDOWS=[8,16,32,64,128,256]
#%%

df = pd.read_csv(results+"opeth.dat",delimiter=" ")

qpWindow=256

fig,ax = plt.subplots(1,1)
ax.xaxis.grid('True',ls='--')
ax.yaxis.grid('True',ls='--')
ax.set_xlabel("Transfer size (KB)")
ax.set_ylabel("Completion time (ms)")
for randomize in [True, False]:
    for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
        if (routing == SOURCE_ROUTING and randomize == False):
            continue
        dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
        if randomize == True:
            ax.plot(dfX["transferSize"]/1000,dfX["completionTime"]/10**6,c = colors[routing][randomize],label=labels[routing][randomize],marker=markers[routing][randomize], markersize = 10)
        else:
            ax.plot(dfX["transferSize"]/1000,dfX["completionTime"]/10**6,c = colors[routing][randomize],label=labels[routing][randomize],marker=markers[routing][randomize], markersize = 10, mfc='none')
        # print(dfX, randomize, routing)
ax.legend()
fig.tight_layout()
fig.savefig(plots+'cct-'+str(qpWindow)+'.pdf')
#%%
# df = pd.read_csv(results+"opeth.dat",delimiter=" ")

qpWindow=256

for transferSize in TRANSFER_SIZES:
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Completion time (ms)")
    ax.set_ylabel("CDF")
    for randomize in [True, False]:
        for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
            if (routing == SOURCE_ROUTING and randomize == False):
                continue
            if (routing == RANDOM_ECMP):
                multiPath = "true"
            else:
                multiPath = "false"
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            fcts = df["fctus"]
            x=np.sort(fcts/10**6)
            y=np.arange(len(fcts))/float(len(fcts))
            ax.plot(x,y,c = colors[routing][randomize])
            ax.set_title('Transfer size = '+str(transferSize))
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,label=labels[routing][randomize])
            else:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,mfc='none',label=labels[routing][randomize])
            # print(dfX)
    ax.legend()
    fig.tight_layout()
    fig.savefig(plots+'cct-cdf-'+str(qpWindow)+'-'+str(transferSize)+'.pdf')

#%%

qpWindow=256

for transferSize in TRANSFER_SIZES:
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Shared buffer occupancy (KB)")
    ax.set_ylabel("CDF")
    for randomize in [True, False]:
        for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
            if (routing == SOURCE_ROUTING and randomize == False):
                continue
            if (routing == RANDOM_ECMP):
                multiPath = "true"
            else:
                multiPath = "false"
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ", usecols=[0,1])
            df = df[df["switch"]==0]
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            buffer = df["totalused"]
            x=np.sort(buffer/10**3)
            y=np.arange(len(buffer))/float(len(buffer))
            ax.plot(x,y,c = colors[routing][randomize])
            ax.set_title('Transfer size = '+str(transferSize))
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,label=labels[routing][randomize])
            else:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,mfc='none',label=labels[routing][randomize])
            # print(dfX)
    ax.legend()
    fig.tight_layout()
    fig.savefig(plots+'buffer-cdf-'+str(qpWindow)+'-'+str(transferSize)+'.pdf')


    
#%%

colormap = cm.jet
normalize = mcolors.Normalize(vmin=np.min(QP_WINDOWS), vmax=np.max(QP_WINDOWS))
colorsmap = colormap(normalize(QP_WINDOWS))

df = pd.read_csv(results+"opeth.dat",delimiter=" ")
routing=FLOW_ECMP

fig,ax = plt.subplots(1,1)
ax.xaxis.grid('True',ls='--')
ax.yaxis.grid('True',ls='--')
ax.set_xlabel("Transfer size (KB)")
ax.set_ylabel("Completion time (ms)")
for randomize in [True, False]:
    color = 0
    for qpWindow in QP_WINDOWS:
        dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
        if randomize == True:
            ax.plot(dfX["transferSize"],dfX["completionTime"]/10**6,c=colorsmap[color],label="w="+str(qpWindow),marker=markers[routing][randomize], markersize = 10)
        else:
            ax.plot(dfX["transferSize"],dfX["completionTime"]/10**6,c=colorsmap[color],label="w="+str(qpWindow),marker=markers[routing][randomize], markersize = 10, mfc='none')
        # print(dfX, randomize, routing)
        color = color + 1
ax.legend(ncols=2,framealpha=0)
#%%
# df = pd.read_csv(results+"opeth.dat",delimiter=" ")

routing=FLOW_ECMP
multiPath = "false"

colormap = cm.jet
normalize = mcolors.Normalize(vmin=np.min(QP_WINDOWS), vmax=np.max(QP_WINDOWS))
colorsmap = colormap(normalize(QP_WINDOWS))


for transferSize in TRANSFER_SIZES:
    color = 0
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Completion time (ms)")
    ax.set_ylabel("CDF")
    for qpWindow in QP_WINDOWS:
        for randomize in [True, False]:
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            fcts = df["fctus"]
            x=np.sort(fcts/10**6)
            y=np.arange(len(fcts))/float(len(fcts))
            ax.plot(x,y,c = colorsmap[color])
            ax.set_title('Transfer size = '+str(transferSize)+ " (ECMP)")
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colorsmap[color],marker=markers[routing][randomize], markersize = 10,label="w="+str(qpWindow))
            else:
                ax.plot(x[-1],y[-1],c = colorsmap[color],marker=markers[routing][randomize], markersize = 10,mfc='none',label="w="+str(qpWindow))
            # print(dfX)
        color = color + 1
    ax.legend()
    
    
    
#%%

plt.rcParams.update({'font.size': 13})
qpWindow="256"
transferSize = 64000

for rand in ["false","true"]:
    fig,ax = plt.subplots(1,1,figsize=(8,4))
    fig1,ax1 = plt.subplots(1,1,figsize=(8,4))
    fig2,ax2 = plt.subplots(1,1,figsize=(8,4))

    multiPath="false"
    routing=FLOW_ECMP
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ")
    df = df[df["switch"]==1]
    ax.plot((df["time"]-1e9)/1e3, df["3"]/1000, label="ECMP",c = 'k')
    ax1.plot((df["time"]-1e9)/1e3, df["17"]/1000, label="ECMP",c = 'k')
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
    fcts = df["fctus"]
    x=np.sort(fcts/10**6)
    y=np.arange(len(fcts))/float(len(fcts))
    ax2.plot(x,y,c = 'k')
    ax2.plot(x[-1],y[-1],c = 'k',marker='s', markersize = 10,label="ECMP")
    
    
    multiPath="true"
    routing=RANDOM_ECMP
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ")
    df = df[df["switch"]==1]
    ax.plot((df["time"]-1e9)/1e3, df["3"]/1000,label="Spray",c = 'r')
    ax1.plot((df["time"]-1e9)/1e3, df["17"]/1000,label="Spray",c = 'r')
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
    fcts = df["fctus"]
    x=np.sort(fcts/10**6)
    y=np.arange(len(fcts))/float(len(fcts))
    ax2.plot(x,y,c = 'r')
    ax2.plot(x[-1],y[-1],c = 'r',marker='X', markersize = 10,label="Spray")
    
    multiPath="true"
    routing=REPS
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ")
    df = df[df["switch"]==1]
    ax.plot((df["time"]-1e9)/1e3, df["3"]/1000,label="Reps",c = 'g')
    ax1.plot((df["time"]-1e9)/1e3, df["17"]/1000,label="Reps",c = 'g')
    df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
    fcts = df["fctus"]
    x=np.sort(fcts/10**6)
    y=np.arange(len(fcts))/float(len(fcts))
    ax2.plot(x,y,c = 'g')
    ax2.plot(x[-1],y[-1],c = 'g',marker='o', markersize = 10,label="Reps")
    
    if rand=="true":
        multiPath="false"
        routing=SOURCE_ROUTING
        df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ")
        df = df[df["switch"]==1]
        ax.plot((df["time"]-1e9)/1e3, df["3"]/1000, label="Ethereal",c = 'b')
        ax1.plot((df["time"]-1e9)/1e3, df["17"]/1000, label="Ethereal",c = 'b')
        df = pd.read_csv(dump+"motiv-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
        fcts = df["fctus"]
        x=np.sort(fcts/10**6)
        y=np.arange(len(fcts))/float(len(fcts))
        ax2.plot(x,y,c = 'b')
        ax2.plot(x[-1],y[-1],c = 'b',marker='d', markersize = 10,label="Ethereal")
    
    ax.set_xlabel("Time (us)")
    ax.set_ylabel("Queue length (KB)")
    ax.xaxis.grid(True,ls="--")
    ax.yaxis.grid(True,ls="--")
    # ax.set_xlim(0,400)
    ax.set_ylim(0,200)
    ax.legend(loc=2,framealpha=0.5)
    # ax.set_title("Downlink")
    # fig.tight_layout()
    # fig.savefig(plots+'motiv-queue-downlink.pdf')
    
    ax1.set_xlabel("Time (us)")
    ax1.set_ylabel("Queue length (KB)")
    ax1.xaxis.grid(True,ls="--")
    ax1.yaxis.grid(True,ls="--")
    # ax1.set_xlim(0,400)
    ax1.set_ylim(0,200)
    ax1.legend(loc=1,framealpha=0.5)
    # fig.legend(ncol=2,loc=9)
    # ax1.set_title("Uplink")
    
    
    
    ax2.set_xlabel("Completion time (ms)")
    ax2.set_ylabel("CDF")
    ax2.xaxis.grid(True,ls="--")
    ax2.yaxis.grid(True,ls="--")
    x_position = 1000*255*(transferSize*8)/(100*1e9)
    ax2.axvline(x_position, c='green', ls='--')
    # ax2.annotate('Optimal', xy=(x_position, 0), xytext=(x_position + 0.05, -0.04),
             # arrowprops=dict(facecolor='green', arrowstyle='fancy', linewidth=1, linestyle='-',edgecolor='green'),
             # fontsize=12, ha='right', va='bottom')
    # ax2.set_xlim(0,400)
    ax2.legend(loc=2,framealpha=0.5)
    # ax2.set_title("Global")
    
    
    
    
    fig.tight_layout()
    # fig.savefig(plots+'motiv-queue-length-downlink-'+rand+'.pdf')
    fig1.tight_layout()
    # fig1.savefig(plots+'motiv-queue-length-uplink-'+rand+'.pdf')
    fig2.tight_layout()
    # fig2.savefig(plots+'motiv-cct-'+rand+'.pdf')
    
    
    
    figlegend = pylab.figure(figsize=(3,0.5))
    # ax3 = figlegend.add_subplot(111)
    figlegend.legend(*ax.get_legend_handles_labels(),ncol=2,loc='center',framealpha=0.5)
    # figlegend.savefig(plots+'motiv-legend-'+rand+'.pdf')
    
    
    
    
#%%




#%%

import random
import math
import matplotlib.pyplot as plt
import numpy as np

def generate_exponential_excluding_i(lambd, i, size=1000):
    results = []
    while len(results) < size:
        rand_num = math.floor(random.expovariate(lambd))
        if rand_num != i:
            results.append(rand_num)
    return results

# Parameters
lambd = 0.01  # rate parameter
i = 2      # value to exclude
size = 10000  # number of samples

# Generate the data
data = generate_exponential_excluding_i(lambd, i, size)

# Plot the data
plt.figure(figsize=(10, 6))
plt.hist(data, bins=size, edgecolor='k', alpha=0.7)
plt.title(f'Exponential Distribution (λ={lambd}) excluding {i}')
plt.xlabel('Value')
plt.ylabel('Frequency')
plt.grid(True)
plt.show()
