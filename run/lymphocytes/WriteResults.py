import numpy as np
import os
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.backends.backend_pdf import PdfPages
import matplotlib.lines as lines
import webbrowser
from visu3D import VarToPlot
import sys
n = len(sys.argv)

if (n<2):
	fileno = "."
else:
	fileno = sys.argv[1]
	

def DifferentiationRates(ThePlotVar,filename,thr=None):
	TotalTime_div_PopM1=0
	TotalTime_div_PopNotM1=0

	Num_dividing_PopM1=0
	Num_dividing_PopNotM1=0
	filePhylo = open(filename)
	contentPhylo = filePhylo.readlines()
	if thr:
		ThePlotVar.threshold = ThePlotVar.max / thr
	Num_CellPos = 0
	Num_CellNeg = 0
	n1=ThePlotVar.LastLine-ThePlotVar.NcellsEnd
	n2=ThePlotVar.LastLine

	for i in range(n1,n2):
		if (ThePlotVar.var_arr[i]>= ThePlotVar.threshold):
			theline = contentPhylo[i-n1]
			ListPhylo = [float(string_t_div) for string_t_div in theline.split()]
			number_of_divisions = len(ListPhylo)-1
			Num_dividing_PopM1+=number_of_divisions
			TotalTime_div_PopM1+=ListPhylo[number_of_divisions]
			Num_CellPos +=1
		else:
			theline = contentPhylo[i-n1]
			ListPhylo = [float(string_t_div) for string_t_div in theline.split()]
			number_of_divisions = len(ListPhylo)-1
			Num_dividing_PopNotM1+=number_of_divisions
			TotalTime_div_PopNotM1+=ListPhylo[number_of_divisions]
			Num_CellNeg+=1
			
	if Num_dividing_PopM1*Num_dividing_PopNotM1==0:
		print("Error: The threshold seems inadequate")
		return 1
	MeanTime_VarPos = TotalTime_div_PopM1/Num_dividing_PopM1
	MeanTime_VarNeg = TotalTime_div_PopNotM1/Num_dividing_PopNotM1
			
	return MeanTime_VarPos, MeanTime_VarNeg, Num_CellPos, Num_CellNeg


def ViewStartAndEnd(filename1,filename2,filenorm1,filenorm2,titlestr,thr=None):
	currentfig = plt.figure(figsize=(12,8))
	currentfig.tight_layout()
	Plot_P1 = VarToPlot(filename1,filenorm1,currentfig)
	if thr:
		Plot_P1.threshold = Plot_P1.max / thr
	Plot_P1.PlotStartAndEnd(1)
	Plot_P1.GiveTitle(titlestr)
	Plot_P2 = VarToPlot(filename2,filenorm2,currentfig)
	if thr:
		Plot_P2.threshold = Plot_P2.max / thr
	Plot_P2.PlotStartAndEnd(2)
	Plot_P2.GiveTitle(titlestr)
	return currentfig, Plot_P1, Plot_P2

def ViewPositiveCells(Plot1,Plot2,name,thr=20):
	N_PosCells_t, N_Cells_t, t= Plot1.GetPositiveCells(thr)
	N_PosCells_t2, N_Cells_t2, t2= Plot2.GetPositiveCells(thr)
	currentfig = plt.figure(figsize=(12,8))
	currentfig.tight_layout()
	currentfig.suptitle("Evolution of the number of positive cells, criterion = Max_"+name+"/"+str(thr), fontsize=18, fontweight = "bold")
	Ax1 = currentfig.add_subplot(2,1,1)
	Ax2 = currentfig.add_subplot(2,1,2)
	##
	Ax1.plot(t,N_PosCells_t, label='Gene 1 +')
	Ax1.plot(t,N_PosCells_t2, label='Gene 2 +')
	Ax1.plot(t,N_Cells_t, label='All')
	Ax1.set_xlabel("Time", fontsize = 14)
	Ax1.set_ylabel("Cell numbers", fontsize = 14)
	Ax1.legend()
	##
	Propt =[100*m/n for m, n in zip(N_PosCells_t,N_Cells_t)]
	Ax2.plot(t,Propt, label='Percentage of G1 + cells, criterion = Max_'+name+"/"+str(thr))
	Propt =[100*m/n for m, n in zip(N_PosCells_t2,N_Cells_t2)]
	Ax2.plot(t,Propt, label='Percentage of G2 + cells, criterion = Max_'+name+"/"+str(thr))
	Ax2.set_xlabel("Time", fontsize = 14)
	Ax2.set_ylabel("Percentage of 'i+' cells", fontsize = 14)
	Ax2.set_ylim(0,100)
	Ax2.legend()
	return currentfig
	
def ViewPhenotypes(name):
	currentfig = plt.figure(figsize=(12,8))
	currentfig.tight_layout()
	currentfig.suptitle("Evolution of the number of cells \n for each phenotype", fontsize=18, fontweight = "bold")
	Ax1 = currentfig.add_subplot(2,1,1)
	Ax2 = currentfig.add_subplot(2,1,2)
	
	PhenoTxt = os.path.join(fileno, "trajectory.txt")
	Phenotypes = VarToPlot(PhenoTxt,None,currentfig)
	
	Resu = Phenotypes.GetPhenotypes()
	Labels = ["Phenotype 0 (Naive)","Phenotype 1 : Effector","Phenotype 2","Phenotype 3","Phenotype 4","Phenotype 5","Phenotype 6","Support Cells (Niche)","Antigen presenting cells (APC)","Total number of cells"]
	LabelsAlt = ["Antigen presenting cells (APC)","Total without niche"]
	##
	for i in range(10):
		Ax1.plot(Resu[10],Resu[i],label=Labels[i])
	Ax1.set_xlabel("Time", fontsize = 14)
	Ax1.set_ylabel("Cell numbers", fontsize = 14)
	Ax1.legend()
	##
	Prop =[(m-n) for m, n in zip(Resu[9],Resu[7])]
	for i in range(7):
		Ax2.plot(Resu[10],Resu[i],label=Labels[i])
	Ax2.plot(Resu[10],Resu[8],label=LabelsAlt[0])
	Ax2.plot(Resu[10],Prop,label=LabelsAlt[1])
	Ax2.set_xlabel("Time", fontsize = 14)
	Ax2.set_ylabel("Cell numbers", fontsize = 14)
	Ax2.legend()
	return currentfig

	
print("This python scripts writes a PDF file for the simulation results.")
#######################################################################################
# View Parameters
print("Step 1/6: Getting parameters from input data...")
firstPage = plt.figure(figsize=(12,8))
firstPage.clf()
txt = 'Simulation parameters'
firstPage.text(0.5,0.9,txt, transform=firstPage.transFigure, size=24, weight = "bold", ha="center")


nameK = os.path.join(fileno, "kineticsparam.txt")
dfK = pd.read_csv(nameK, sep="\t", skiprows=1, header=None, index_col=False)
txt = "Kinetics parameters: \n  \n"
txt += "d0: " + str(dfK.iloc[0,0])+ "\n"
txt += "d1: " + str(dfK.iloc[0,1])+ "\n"
txt += "a0: " + str(dfK.iloc[0,2])+ "\n"
txt += "a1: " + str(dfK.iloc[0,3])+ "\n"
txt += "a2: " + str(dfK.iloc[0,4])+ "\n"
firstPage.text(0.75,0.6,txt, transform=firstPage.transFigure, size=14, ha="left")


nameK = os.path.join(fileno, "cyclingparam.txt")
dfK = pd.read_csv(nameK, delim_whitespace=True, skiprows=1, header=None, index_col=False)
txt = "Cycling parameters: \n \n"
txt += "mitotic_threshold_: " + str(dfK.iloc[0,0])+ "\n"
txt += "cycling_threshold_: " + str(dfK.iloc[0,1])+ "\n"
txt += "dividing_threshold_: " + str(dfK.iloc[0,2])+ "\n"
txt += "death_threshold_: " + str(dfK.iloc[0,3])+ "\n"
txt += "mitotic_threshold_P0_*: " + str(dfK.iloc[0,4])+ "\n"
txt += "k_survival: " + str(dfK.iloc[0,5])+ "\n"
firstPage.text(0.75,0.3,txt, transform=firstPage.transFigure, size=14, ha="left")

txt = "(*) mitotic_threshold_P0_ (MTP0) is the mean of the exponential law determining the the division threshold (D.T.) \n"
txt+="CD8 Protein 1 levels must be above D.T. to divide. If MTP0 is close enough to zero, the D.T. is always met: there is no regulation by P1."
firstPage.text(0.1,0.04,txt, transform=firstPage.transFigure, size=12, ha="left")

nameK = os.path.join(fileno, "GeneInteractionsMatrix.txt")
dfK = pd.read_csv(nameK, sep="\t", skiprows=0, header=None, index_col=False)
with open(nameK, 'r') as file:
    data = file.read().expandtabs()#replace('\t', '         ')
txt = "Genes Interaction Matrix: \n \n "
txt += data
firstPage.text(0.1,0.6,txt, transform=firstPage.transFigure, size=14, ha="left")

firstPage.add_artist(lines.Line2D([0, 1], [0.1, 0.1],linewidth=2,color="black"))
firstPage.add_artist(lines.Line2D([0.7, 0.7], [0.1, 0.85],linewidth=2,color="black"))

#######################################################################################
# View mRNA 1,2 and Protein 1,2 at the end of the simulation
print("Step 2/6: Generating 3D Plots...")
print("          ...for Protein levels...")
fileP1 = os.path.join(fileno, "gene1.txt")
fileP2 = os.path.join(fileno, "gene2.txt")
fileNP1 = os.path.join(fileno, "NormGene1.txt")
fileNP2 = os.path.join(fileno, "NormGene2.txt")
resu_P = ViewStartAndEnd(fileP1,fileP2,fileNP1,fileNP2, "3D Plot: Protein levels ",thr=20)
print("          ...for mRNA levels...")
fileM1 = os.path.join(fileno, "mRNA1.txt")
fileM2 = os.path.join(fileno, "mRNA2.txt")
fileNM1 = os.path.join(fileno, "NormM1.txt")
fileNM2 = os.path.join(fileno, "NormM2.txt")
resu_M = ViewStartAndEnd(fileM1,fileM2,fileNM1,fileNM2, "3D Plot: mRNA levels",thr=20)

#######################################################################################
# Number of cells in Population mRNA 1 +, Population P1+, and total nb of cells: evolution, and at the end of simulation
# -> ratio of positive (+) cells for Gene 1
# criterion for positivity var(t) >= maxvar / 20
#######################################################################################
print("Step 3/6: Generating data corresponding to specific gene activation...")
print("          ...for Protein levels...")
Figure3 = ViewPositiveCells(resu_P[1],resu_P[2],"Protein",thr=20)
print("          ...for mRNA levels...")
Figure4 = ViewPositiveCells(resu_M[1],resu_M[2],"mRNA",thr=20)

#######################################################################################
# Differentiation rates of the populations mRNA1+/-, P1+/-
#######################################################################################
print("Step 4/6: Generating data corresponding to cell proliferation...")
PhyloTxt = os.path.join(fileno, "phylogeny_t.txt")
ResP1 = DifferentiationRates(resu_P[1],PhyloTxt,thr=20)
ResP2 = DifferentiationRates(resu_P[2],PhyloTxt,thr=20)
ResM1 = DifferentiationRates(resu_M[1],PhyloTxt,thr=20)
ResM2 = DifferentiationRates(resu_M[2],PhyloTxt,thr=20)
lines =[0.65,0.5,0.4,0.3,0.2]
cols =[0.1,0.4,0.55,0.7,0.85]

Page5 = plt.figure(figsize=(12,8))
Page5.clf()
txt = "Cell proliferation for positive or negative populations: \n Calculation of the mean time before the next division"
Page5.text(0.5,0.9,txt, transform=Page5.transFigure, size=24, ha="center")

txt = "Cell population \n +: Gene i is expressed \n -: Gene i is not expressed"
Page5.text(cols[0],lines[0],txt, transform=Page5.transFigure, size=14, ha="left")
txt = "Criterion: \n Protein 1"
Page5.text(cols[0],lines[1],txt, transform=Page5.transFigure, size=14, ha="left")
txt = "Criterion: \n Protein 2"
Page5.text(cols[0],lines[2],txt, transform=Page5.transFigure, color="darkgray", size=14, ha="left")
txt = "Criterion: \n mRNA 1"
Page5.text(cols[0],lines[3],txt, transform=Page5.transFigure, size=14, ha="left")
txt = "Criterion: \n mRNA 2"
Page5.text(cols[0],lines[4],txt, transform=Page5.transFigure, color="darkgray", size=14, ha="left")

txt_arr = ["Division time \n G+ cells"]
txt_arr.append("Division time \n G- cells")
txt_arr.append("Number of \nG+ cells")
txt_arr.append("Number of \nG- cells")

for i in range(0,4):
	Page5.text(cols[i+1],lines[0],txt_arr[i], transform=Page5.transFigure, size=14, ha="center")
	Page5.text(cols[i+1],lines[1],str(round(ResP1[i],2)), transform=Page5.transFigure, size=14, ha="center")
	Page5.text(cols[i+1],lines[2],str(round(ResP2[i],2)), transform=Page5.transFigure, size=14, color="darkgray", ha="center")
	Page5.text(cols[i+1],lines[3],str(round(ResM1[i],2)), transform=Page5.transFigure, size=14, ha="center")
	Page5.text(cols[i+1],lines[4],str(round(ResM2[i],2)), transform=Page5.transFigure, size=14, color="darkgray", ha="center")

#######################################################################################
# Nb of naive, effector cells, APC as a function of time
#######################################################################################
print("Step 5/6: Generating data corresponding to cell phenotype...")
Figure6 = ViewPhenotypes("trajectory.txt")



#Write PDF
print("Step 6/6: Writing PDF...")
OutputName = os.path.join(fileno, "Results.pdf")
Figure1 = resu_P[0]
Figure2 = resu_M[0]

with PdfPages(OutputName) as pdf:
	pdf.savefig(firstPage)
	pdf.savefig(Figure1)
	pdf.savefig(Figure2)
	pdf.savefig(Figure3)
	pdf.savefig(Figure4)
	pdf.savefig(Page5)
	pdf.savefig(Figure6)
	plt.close()
	print("Done.")

webbrowser.open(OutputName)
