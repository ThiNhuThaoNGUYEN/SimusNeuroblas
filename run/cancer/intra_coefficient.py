import pandas as pd
import numpy as np
import csv
import math

## value R
Test_R = pd.read_csv('Test.txt', sep ="\t",header = None)
R=float(Test_R[0])

##read file
path= ''
Data_CD133 = pd.read_csv(path+'CD133.txt', sep ="\t", header = 0)

alpha= 0.05

Red = Data_CD133[Data_CD133['Classification'] == "Positive"] ### stem cells

# Extract 'Positive' classifications = stem cells
if 'Classification' in Data_CD133.columns:
    Data_CD133["Classification"] = Data_CD133["Classification"].str.strip()
    positive_cells = Data_CD133[Data_CD133['Classification'] == 'Positive'].copy()
    positive_cells['Index'] = positive_cells.index  
    
    # Convert to numeric
    positive_cells['Centroid X µm'] = pd.to_numeric(positive_cells['Centroid X µm'], errors='coerce')
    positive_cells['Centroid Y µm'] = pd.to_numeric(positive_cells['Centroid Y µm'], errors='coerce')
    
else:
    print("Error: 'Classification' column not found in the file.")


dist1 = R  # distance
dist2 = 4.* dist1 *dist1
   

# Compute ntot
def ntot():
  if 'Centroid X µm' in Data_CD133.columns and 'Centroid Y µm' in Data_CD133.columns:
        xtout = Data_CD133['Centroid X µm'].values
        ytout = Data_CD133['Centroid Y µm'].values  
  ntot = np.full(len(xtout), -1.0)  # Initialize ntot to -1
  nn = 0
  ntoutcorr = 0
  for i in range(len(xtout)):
        for j in range(len(xtout)):
            dd = (xtout[i] - xtout[j]) ** 2 + (ytout[i] - ytout[j]) ** 2  # Euclidean distance
            if dd <= dist1*dist1:
                ntot[i] += 1  # Increment ntot if condition is met

        if ( ntot[i] > 0): nn +=1

  ntoutcorr = nn
  return  ntot, ntoutcorr    


def ntot_A():
  if 'Centroid X µm' in positive_cells.columns and 'Centroid Y µm' in positive_cells.columns:
        xtout_A = positive_cells['Centroid X µm'].values
        ytout_A = positive_cells['Centroid Y µm'].values  
  ntot_A = np.full(len(xtout_A), -1.0)  # Initialize ntot_A to -1
  for i in range(len(xtout_A)):
        for j in range(len(xtout_A)):
            dd = (xtout_A[i] - xtout_A[j]) ** 2 + (ytout_A[i] - ytout_A[j]) ** 2  # Euclidean distance
            if dd <= dist1*dist1:
                ntot_A[i] += 1  # Increment ntot_A if condition is met
  return ntot_A   


def compute_nvois_nloin_ntot0():
      global ntot
      if 'Centroid X µm' in Data_CD133.columns and 'Centroid Y µm' in Data_CD133.columns:
        xtout = Data_CD133['Centroid X µm'].values
        ytout = Data_CD133['Centroid Y µm'].values  

      if len(ntot) == 0:
        return 0.0
    
      xt=0.
      nvois=0
      ave2totv=0.
      non0=0

      for i in range(len(xtout)-1):
         for j in range(i+1,len(xtout)):
             if (ntot[i]*ntot[j] > 0.):
                 non0 += 1
                 nvt = 0
                 dd = (xtout[i] - xtout[j]) ** 2 + (ytout[i] - ytout[j]) ** 2
                 if (dd <= dist1*dist1):
                     ave2totv += 1./(ntot[i])/(ntot[j]) 

                     niv = -2
                     nvois += 1
                     for k in range(0,len(xtout)):
                         dd1 = (xtout[i] - xtout[k]) ** 2 + (ytout[i] - ytout[k]) ** 2
                         dd2 = (xtout[j] - xtout[k]) ** 2 + (ytout[j] - ytout[k]) ** 2
                         if (dd1 <= dd and dd2 <= dd):
                             niv += 1

                     xt += niv/(ntot[i])/(ntot[j])
                 elif (dd <= dist2):
                     niv=0
                     for k in range(0,len(xtout)):
                         dd1 = (xtout[i] - xtout[k]) ** 2 + (ytout[i] - ytout[k]) ** 2
                         dd2 = (xtout[j] - xtout[k]) ** 2 + (ytout[j] - ytout[k]) ** 2
                         if (dd1 <= dd and dd2 <= dd):
                             niv += 1       
                     xt += niv/ntot[i]/ntot[j]
                 else:
                     niv = 0

                 xt += niv/ntot[i]/ntot[j]


      return  ave2totv, xt, non0

# compute ave1tout
def compute_ave1tout():
    """ Computes ave1tout """
    global ntot, ntoutcorr
    
    if len(ntot) == 0:
        return 0.0
    
    ave1tout = 0.0
    nn=0
    xntotmoy=0.

    for i in range(len(ntot)):
        if ntot[i] > 0:  # Avoid division by zero
            ave1tout += 1./ntot[i]
            xntotmoy = xntotmoy + ntot[i]

    ave1tout /= ntoutcorr
    
    return ave1tout



ntot, ntoutcorr = ntot()

NA = len(Red)
NT = len(Data_CD133)

ave1totv=compute_ave1tout()

ave2totv, xt, non0 = compute_nvois_nloin_ntot0()


###################

def comput_NA():

  NT=[]
  for j in range(0,len(positive_cells)):
    for i in range(len(ntot)):
       if (i == positive_cells['Index'].values[j]):
           NT.append(ntot[i])
  return NT         
  
positive_cells['ntot']= comput_NA()


def compute_ave_NA_NT():
    global ntot, ntoutcorr

    NA_A_i = ntot_A()
    NT_i =comput_NA()

    sum_NA_NT = 0
    n_sum = 0
    for j in range(0,len(positive_cells)):
       if (NT_i[j] != 0.):    
           sum_NA_NT +=  NA_A_i[j]/NT_i[j]
           n_sum += 1
       elif (NT_i[j] == 0. and NA_A_i[j] == 0.):
           sum_NA_NT += 1.
           n_sum += 1

    return sum_NA_NT, n_sum


sum_NA_NT, n_sum =compute_ave_NA_NT()
S_NA_NT =  sum_NA_NT/n_sum

xna = len(positive_cells)
xnt = ntoutcorr

a_AA = (xnt-1)*S_NA_NT/(xna-1)

################ var 1

Var1 = -(xnt - xna)/(xnt - 2)/(xna - 1)

################ var 2

V2 =  (xnt - 1)*(xnt - xna)/xna/(xna - 1)/(xnt -2)

Var2 = V2*ave1totv

################ var 3

V3 = (xnt - 1)*(xnt-1)*(xnt - xna)*(xnt-xna-1)/(xnt-2)/(xnt-3)/xna/(xna-1)

Var3= V3*ave2totv/float(non0)

################ var 4

V4 = (xnt-1)*(xnt-1)*(xnt-xna)*(xna-2)/(xnt-2)/(xnt-3)/xna/(xna-1)
Var4 = V4*xt/float(non0)


#print(Var1,Var2,Var3,Var4)

phi_2= Var1+Var2+Var3+Var4

q_AA_al = math.sqrt(phi_2/alpha)

#print(q_AA_al)

df3= pd.DataFrame({"a_AA": [a_AA],"Var1":Var1, "Var2":Var2, "Var3":Var3, "Var4 ":Var4, "q_AA_al":q_AA_al})

df3.to_csv (path+"output_variance.txt",sep="\t",index=False)


