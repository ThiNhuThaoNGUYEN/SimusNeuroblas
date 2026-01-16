import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

import os,sys

n=5 ## 5 values for the diffusion coefficient

nom_dossiers_="dossier_Test_K"

npc=5 # number of parallel calculation

S1=[]
S2=[]
S3=[]
S4=[]
S5=[]
S6=[]
S7=[]
X=[]
S=[]
Stem_Thr=[]
Stem_Signal=[]
Stem_pos=[]
Stem_neg=[]
Stem_Mean_P=[]
Stem_Mean_N=[]
Stem_MI=[]
Stem_ENT=[]
Stem_Z=[]
central=[]
Percent_pos=[]
std_MI=[]
std_ENT=[]
std_center=[]
std_pos=[]
M_central=[]
M_Percent_pos=[]

i=1
while (i < n+1):
#for numero_test in range(1,n+1):
  
  for numero_simul in range(1,npc+1):
    path =""
    path1 = path+nom_dossiers_+str(i)+"_"+str(numero_simul)+"/"
    path1_1 =path1+"SimusNeuroblas/run/cancer/"
    df_S1=pd.read_csv(path1_1 + "data.txt", sep="\t", header=0)
     
    Stem_Thr.append(float(df_S1['D_stem']))
    Stem_Signal.append(float(df_S1['S_stem']))
    Stem_pos.append(df_S1['Pos'])
    Stem_neg.append(df_S1['Ne'])
    Stem_Mean_P.append(float(df_S1['Mean_P']))
    Stem_Mean_N.append(float(df_S1['Mean_N']))
    Stem_MI.append(float(df_S1['MI']))
    Stem_ENT.append(float(df_S1['ENT']))
    Stem_Z.append(float(df_S1['Z']))

    if (float(df_S1['Mean_N']) != 0.):
      central.append(float(df_S1['Mean_P'])/float(df_S1['Mean_N']))
    else:
      central.append(2.)
    Percent_pos.append(df_S1['Pos']*100/(df_S1['Pos']+df_S1['Ne']))    

  p0=np.mean(Stem_Thr)
  p0_0=np.mean(Stem_Signal)
  p1=np.mean(Stem_pos)
  p2=np.mean(Stem_neg)
  p3=np.mean(Stem_Mean_P)
  p4=np.mean(Stem_Mean_N)
  p5=np.mean(Stem_MI)
  p6=np.mean(Stem_ENT)
  p7=np.mean(Stem_Z)
  p8=np.mean(central)
  p9=np.mean(Percent_pos)
  p10=np.std(Stem_MI)
  p11=np.std(Stem_ENT)
  p12=np.std(central)
  p13=np.std(Percent_pos)

  X.append(p0)
  S.append(p0_0)
  S1.append(p1)
  S2.append(p2)
  S3.append(p3)
  S4.append(p4)
  S5.append(p5)
  S6.append(p6)
  S7.append(p7)
  M_central.append(p8)
  M_Percent_pos.append(p9)
  std_MI.append(p10)
  std_ENT.append(p11)
  std_center.append(p12)
  std_pos.append(p13)
  
  i = i + 1

  Stem_Thr=[]
  Stem_Signal=[]
  Stem_pos=[]
  Stem_neg=[]
  Stem_Mean_P=[]
  Stem_Mean_N=[]
  Stem_MI=[]
  Stem_ENT=[]
  Stem_Z=[]
  central=[]
  Percent_pos=[]
##
df1= pd.DataFrame({"D_stem":X, "Signal_stem":S,	"Pos":S1,	"Ne":S2,	"Mean_P":S3,	"Mean_N":S4,	"MI":S5,	"ENT":S6,	"Z":S7, "Central":M_central, "Percent_pos":M_Percent_pos, "std_MI":std_MI, "std_ENT": std_ENT, "std_center": std_center, "std_percent_pos": std_pos})

df1.to_csv (path+"out_data_mean.txt",sep="\t",index=False)
