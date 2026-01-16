import csv
import pandas as pd

path =""
currentTime2=0. # be 0.0
res_S=[]## #stem cells
res_percent=[]

time_=[]
time_.append(0.) # 0.0
tot_S=0

Tot=[]
Tot.append(15)

with open('trajectory.txt', 'r') as f: 
    reader = csv.reader(f, delimiter = ' ')
    for row in reader:  # loop through all the lines of the file, store one line at a time in row
        if row[0] not in '#$!': # skip header lines that start with #, $ or !

            # below is your code with data replaced by row
            if(float(float(row[0])/24.)==currentTime2):
           #     print (float(row[11]))
                if (float(row[8])==1.):
                    tot_S=tot_S+1
            else:
                time_.append(float(row[0])/24.)
                currentTime2=float(row[0])/24.
                res_S.append(tot_S)
                Tot.append(float(row[1]))
                res_percent.append(tot_S/float(row[1]))
                tot_S=0
                if (float(row[8])==1.):
                    tot_S=tot_S+1

res_S.append(tot_S)
res_percent.append(tot_S*100./float(row[1]))
print(tot_S)

df3= pd.DataFrame({'time':time_ , 'Num_S':res_S, 'Total':Tot , 'percent': res_percent})

df3.to_csv (path+"Percent_Stem.txt",sep="\t", index=False)
