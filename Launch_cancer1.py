import os,sys
t=1
os.system("rm -rf dossier_Test_K"+str(t)+"_*")

nom_dossiers="dossier_Test_K"+str(t)+"_"
fichier_mega_batch='mega_batch_type_K'+str(t)+'.sh'
mbatch=open(fichier_mega_batch,'w') # One file to launch them all...

npc=5 # number of parallel calculation

for numero_simul in range(1,npc+1):
    os.makedirs( nom_dossiers+str(numero_simul) )
    os.system("cp -r "+"../SimusNeuroblas"+" ./"+nom_dossiers+str(numero_simul)+"/.")

    resol=open(nom_dossiers+str(numero_simul)+'/resol.sh','w')
    resol.write("#!/bin/bash\n")
    resol.write("#SBATCH --job-name=Thao\n")
    resol.write("#SBATCH -p Lake,Cascade\n")
    resol.write("#SBATCH --nodes=1\n")
    resol.write("#SBATCH --ntasks-per-node=2         # number of MPI processes per node\n")
    resol.write("#SBATCH --cpus-per-task=8           # number of OpenMP threads per MPI process\n")
    resol.write("#SBATCH --time=0-15:30:00\n")
    resol.write("#\n")
    resol.write("#SBATCH --mail-type=FAIL\n")

    resol.write("#SBATCH --mail-user=thi.nguyen1@ens-lyon.fr\n")
    resol.write("module purge\n")
    resol.write("module use /applis/PSMN/debian11/E5/modules/all\n")
    resol.write("module load GCC/10.3.0\n")
    resol.write(" module load GSL\n")
    resol.write("cd SimusNeuroblas\n")
    resol.write("rm -rf build\n")
    resol.write("mkdir build && cd build\n")
    resol.write("cmake -DCMAKE_INSTALL_PREFIX=/home/tnguye22/bin ..\n")
    resol.write("make simuscale\n")
    resol.write("cd ../run/cancer \n")
    resol.write("rm -rf phylogeny_*\n") 
   #change param.in  
    resol.write("\cp ../../../../param_diffusive_"+str(t)+"_"+str(numero_simul)+".in param.in \n")
    resol.write("./../../build/bin/simuscale \n")
    resol.close()
    os.system("chmod a+x "+nom_dossiers+str(numero_simul)+'/resol.sh') # sets file execution rights
    # write the corresponding line in the megabatch file
    mbatch.write("cd ./"+nom_dossiers+str(numero_simul)+"\n")
    mbatch.write("sbatch resol.sh\n")
    mbatch.write("cd ..\n")

mbatch.close()
os.system("chmod a+x "+fichier_mega_batch) 
