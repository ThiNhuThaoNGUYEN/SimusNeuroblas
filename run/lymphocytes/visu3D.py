import numpy as np
import os
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from mpl_toolkits.mplot3d import Axes3D

class VarToPlot:
	def __init__(self,filename = None,filenorm = None, fig = None):
		self.fig = fig
		self.ax = None
		self.ScalingFactor = None
		
		self.StartEnd = ['Start', 'End']
		
		df1 = pd.read_csv(filename, sep=" ", skiprows=10, names = ['t','NCells','id','X','Y','Z','radius','var'], index_col=False)
		if filenorm:
			dfM1 = pd.read_csv(filenorm, sep=" ", skiprows=1, header=None, index_col=False)
			self.max = dfM1.iloc[0,0]
		else:
			self.max = 0

		Ncells_array = df1['NCells'].to_numpy()
		self.CellArray = Ncells_array
		
		self.CellsX = df1['X'].to_numpy()
		self.CellsY = df1['Y'].to_numpy()
		self.CellsZ = df1['Z'].to_numpy()
		self.R = df1['radius'].to_numpy()
		self.var_arr = df1['var'].to_numpy()
		self.ids = df1['id'].to_numpy()
		
		self.threshold = self.max
		
		LastLine = np.size(Ncells_array)
		self.Last_id = df1.iloc[LastLine-1,1]
		self.LastLine = LastLine
		NcellsInit = int(Ncells_array[0])
		NcellsEnd = int(Ncells_array[LastLine-1])
		self.NcellsEnd=NcellsEnd
		self.n1=[0,LastLine-NcellsEnd]
		self.n2=[NcellsInit,LastLine]
		
		self.Time_array = df1['t'].to_numpy()
		FirstTime = self.Time_array[0]
		DT = self.Time_array[NcellsInit]-FirstTime
		self.LastTime = self.Time_array[LastLine-1]
		self.NFrames = int((self.LastTime-FirstTime)/DT)+1

	def GiveTitle(self, name = " "):
		self.fig.suptitle(name, fontsize=18, fontweight = "bold")

	def SetAxis(self,i,ProteinNo):
		self.ax.set_xlim(np.min(self.CellsX), np.max(self.CellsX))
		self.ax.set_ylim(np.min(self.CellsY), np.max(self.CellsY))
		self.ax.set_zlim(np.min(self.CellsZ), np.max(self.CellsZ))
		self.ax.set_box_aspect((np.ptp(self.CellsX), np.ptp(self.CellsY), np.ptp(self.CellsZ)))
		for line in self.ax.zaxis.get_ticklines():
			line.set_visible(False)
		self.ax.zaxis.set_ticklabels([])
		self.ScalingFactor = (self.ax.get_window_extent().width  / np.ptp(self.CellsX) * 72./self.fig.dpi) ** 2
		
		if ProteinNo<2:
			self.ax.set_title(self.StartEnd[i], fontdict = {'fontsize': '14'})
		
		self.ax.text(50,50, 0.5, ("Gene #"+ str(ProteinNo)), fontsize = 12)
			
	def PrintNicheAndCells(self,n1,n2,bl):
		for i in range(n1,n2):
			x = self.CellsX[i]
			y = self.CellsY[i]
			z = self.CellsZ[i]
			var = self.var_arr[i]
			r = self.ScalingFactor*self.R[i]
			if (z>0):
				p=self.ax.scatter3D(x,y,z, s=r, edgecolor="w", c=var, cmap='plasma', vmin=0, vmax=self.threshold,alpha=0.7)
			else:
				self.ax.scatter3D(x,y,z, s=r, edgecolor="gray", facecolor="lightgray")
			
		if bl:
			cbaxes = self.fig.add_axes([0.05, 0.2, 0.013, 0.6], label="Colorbar")
			self.fig.colorbar(p,cax = cbaxes)		

	def GetPositiveCells(self, thr):
		if thr:
			self.threshold = self.max / thr

		N_PosCells_t = []
		t= []
		N_Cells_t = []
		n2=0
		
		while n2<self.LastLine:
			N_tothetime = n2
			Ncells = int(self.CellArray[N_tothetime])
			n1=N_tothetime
			n2=N_tothetime+Ncells
			N_PosCells_t.append(np.sum(self.var_arr[n1:n2]>=self.threshold))
			N_Cells_t.append(Ncells)
			t.append(self.Time_array[n1])
		
		return N_PosCells_t, N_Cells_t, t
		
	def NCellPhenotype(self, p,n1,n2):
		res = 0
		for i in range(n1,n2):
			if ((self.var_arr[i]<=p+1e-7)and(self.var_arr[i]>=p-1e-7)):
				res+=1
		return res
		
	def GetPhenotypes(self):
		Pop1_t = []
		Pop2_t = []
		Pop3_t = []
		Pop4_t = []
		Pop5_t = []
		Pop6_t = []
		PopNiche_t = []
		PopAPC_t = []
		Pop0_t = [] #naive
		t= []
		N_Cells_t = []
		n2=0
		while n2<self.LastLine:
			N_tothetime = n2
			Ncells = int(self.CellArray[N_tothetime])
			n1=N_tothetime
			n2=N_tothetime+Ncells
			N_Cells_t.append(Ncells)
			t.append(self.Time_array[n1])
			Pop1_t.append(self.NCellPhenotype(1,n1,n2))
			Pop2_t.append(self.NCellPhenotype(2,n1,n2))
			Pop3_t.append(self.NCellPhenotype(3,n1,n2))
			Pop4_t.append(self.NCellPhenotype(4,n1,n2))
			Pop5_t.append(self.NCellPhenotype(5,n1,n2))
			Pop6_t.append(self.NCellPhenotype(6,n1,n2))
			PopNiche_t.append(self.NCellPhenotype(-20,n1,n2))
			PopAPC_t.append(self.NCellPhenotype(-10,n1,n2))
			Pop0_t.append(self.NCellPhenotype(0,n1,n2)) #naive
		return Pop0_t, Pop1_t, Pop2_t, Pop3_t, Pop4_t, Pop5_t, Pop6_t, PopNiche_t, PopAPC_t, N_Cells_t, t
			

        
	def PlotStartAndEnd(self,ProteinNo):
		#Plot 2 subfigures
		for i in range(2):
			self.ax = self.fig.add_subplot(2,2,(ProteinNo*2+i-1), projection='3d')
			self.SetAxis(i, ProteinNo)
			self.PrintNicheAndCells(self.n1[i],self.n2[i],(i*(ProteinNo-1)>0))
		return self.fig
