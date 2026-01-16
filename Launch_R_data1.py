# /usr/bin/python
import os,sys
t=1
nom_dossiers="dossier_Test_K"+str(t)+"_"
fichier_mega_batch='mega_batch_data'+str(t)+'.sh'

mbatch=open(fichier_mega_batch,'w') # One file to launch them all... My precious...

npc=5 # number of parallel calculation

for numero_simul in range(1,npc+1):

 # Copy the needed files into each dossier folder
   os.system("cp -r "+"index.R"+" ./"+nom_dossiers+str(numero_simul)+"/SimusNeuroblas/run/cancer/"+"/.")

    #c copy and modify the retype.sh file
   redata=open(nom_dossiers+str(numero_simul)+'/redata.sh','w')
   redata.write("#!/bin/bash\n")
   redata.write("#SBATCH --job-name=Thao\n")
   redata.write("#SBATCH -p Cascade,Lake\n")
   redata.write("#SBATCH --nodes=1\n")
   redata.write("#SBATCH --ntasks-per-node=2         # number of MPI processes per node\n")
   redata.write("#SBATCH --cpus-per-task=8           # number of OpenMP threads per MPI process\n")
   redata.write("#SBATCH --time=0-5:30:00\n")
   redata.write("#\n")
   redata.write("#SBATCH --mail-type=FAIL\n")
   redata.write("#SBATCH --mail-user=thi.nguyen1@ens-lyon.fr\n")

   redata.write("cd simuscale/run/cancer/\n")
   redata.write("rm -rf trajectory_copie.txt\n")
   redata.write("awk '{if( $1 != "+'"$"'+" && $1 != "+'"#"'+" && $1 != "+'"!"'+") {print} }' trajectory.txt > trajectory_copie.txt\n")
   redata.write("Rscript index.R\n")

   redata.close()
   os.system("chmod a+x "+nom_dossiers+str(numero_simul)+'/redata.sh') # sets file execution rights

 # write the corresponding line in the megabatch file
   mbatch.write("cd ./"+nom_dossiers+str(numero_simul)+"\n")
   mbatch.write("sbatch redata.sh\n")
   mbatch.write("cd ..\n")

mbatch.close()
os.system("chmod a+x "+fichier_mega_batch) # sets file execution rights
