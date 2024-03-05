import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

import sys
n = len(sys.argv)

if (n<2):
	fileno = "4242337"
else:
	fileno = sys.argv[1]


	

df1 = pd.read_csv(fileno + "/Cell_1.txt", sep=" ", skiprows=0, names = ['t','P1','mRNA1','P2','mRNA2','P3','mRNA3','P4','mRNA4'], index_col=False)
df2 = pd.read_csv(fileno + "/Cell_2.txt", sep=" ", skiprows=0, names = ['t','P1','mRNA1','P2','mRNA2','P3','mRNA3','P4','mRNA4'], index_col=False)
dfMatrix = pd.read_csv(fileno + "/GeneInteractionsMatrix.txt", sep='\t')

dfKin = pd.read_csv(fileno + "/kineticsparam.txt", sep='\t')
print(fileno)
print(dfKin)
print(dfMatrix)

Time_array1 = df1['t'].to_numpy()
Time_array2 = df2['t'].to_numpy()
Time_array=Time_array1

P1_arr1 = df1['P1'].to_numpy()
P1_arr2 = df2['P1'].to_numpy()
P2_arr1 = df1['P2'].to_numpy()
P2_arr2 = df2['P2'].to_numpy()
P3_arr1 = df1['P3'].to_numpy()
P3_arr2 = df2['P3'].to_numpy()

mRNA1_arr1 = df1['mRNA1'].to_numpy()
mRNA1_arr2 = df2['mRNA1'].to_numpy()
mRNA2_arr1 = df1['mRNA2'].to_numpy()
mRNA2_arr2 = df2['mRNA2'].to_numpy()
mRNA3_arr1 = df1['mRNA3'].to_numpy()
mRNA3_arr2 = df2['mRNA3'].to_numpy()

LastLine = np.size(Time_array)

if (n>2):
	Tend = int(sys.argv[2])
else: 
	Tend = np.max(Time_array)

# Creates four polar axes, and accesses them through the returned array
fig, axes = plt.subplots(2, 4, sharex='all')
axes[0, 0].plot(Time_array1, mRNA1_arr1, color='r')
axes[0, 0].plot(Time_array2, mRNA1_arr2, color='b')
axes[0, 0].set_title('Expr. of mRNA 1')


axes[0, 1].plot(Time_array1, mRNA2_arr1, color='r')
axes[0, 1].plot(Time_array2, mRNA2_arr2, color='b')
axes[0, 1].set_title('Expr. of mRNA 2')

axes[0, 2].plot(Time_array1, mRNA1_arr1, color='r')
axes[0, 2].plot(Time_array1, mRNA2_arr1, color='b')
axes[0, 2].plot(Time_array1, mRNA3_arr1, color='g')
axes[0, 2].set_title('CellA Expr. of mRNA 1 2 3')


axes[0, 3].plot(Time_array2, mRNA1_arr2, color='r')
axes[0, 3].plot(Time_array2, mRNA2_arr2, color='b')
axes[0, 3].plot(Time_array2, mRNA3_arr2, color='g')
axes[0, 3].set_title('CellB Expr. of mRNA 1 2 3')


axes[1, 0].plot(Time_array1, P1_arr1, color='r')
axes[1, 0].plot(Time_array2, P1_arr2, color='b')
axes[1, 0].set_title('Expr. of Protein 1')

axes[1, 1].plot(Time_array1, P2_arr1, color='r')
axes[1, 1].plot(Time_array2, P2_arr2, color='b')
axes[1, 1].set_title('Expr. of Protein 2')


axes[1, 2].plot(Time_array, P1_arr1, color='r')
axes[1, 2].plot(Time_array, P2_arr1, color='b')
axes[1, 2].plot(Time_array, P3_arr1, color='g')
axes[1, 2].set_title('Expr. of P 1 2 3')

axes[1, 3].plot(Time_array2, P1_arr2, color='r')
axes[1, 3].plot(Time_array2, P2_arr2, color='b')
axes[1, 3].plot(Time_array2, P3_arr2, color='g')
axes[1, 3].set_title('Expr. of P 1 2 3')


plt.xlim([0*Tend/5, Tend])

plt.savefig('GenesPlot_'+fileno+'.png',facecolor='w')







# Figure
fig2 = plt.figure(2,figsize=(12,4))
gs = gridspec.GridSpec(2, 1)
ax1 = plt.subplot(gs[0,0])
ax2 = plt.subplot(gs[1,0])

# Plot proteins
ax1.plot(Time_array, P1_arr1, color='r', label='P1')
ax1.plot(Time_array, P2_arr1, color='b', label='P2')
ax1.plot(Time_array, P3_arr1, color='g', label='P3')

ax1.set_ylim(0, np.max([1.2*np.max(P3_arr1)]))
ax1.tick_params(axis='x', labelbottom=False)
#ax1.tick_params(axis='y', left=False, labelleft=False)
ax1.legend(loc='upper left', ncol=4, borderaxespad=0, frameon=False)
ax1.set_xlim(0*Tend/5, Tend)

# Plot mRNA
ax2.plot(Time_array, mRNA1_arr1, color='r', label='mNA1')
ax2.plot(Time_array, mRNA2_arr1, color='b', label='mRNA2')
ax2.plot(Time_array, mRNA3_arr1, color='g', label='mRNA3')

ax2.set_ylim(0, 1.2*np.max(mRNA1_arr1))
#ax2.tick_params(axis='y', left=False, labelleft=False)
ax2.legend(loc='upper left', ncol=4, borderaxespad=0, frameon=False)

ax2.set_xlim(0*Tend/5, Tend)
plt.savefig('GenesPlotB_'+fileno+'.png',facecolor='w')

plt.show()
