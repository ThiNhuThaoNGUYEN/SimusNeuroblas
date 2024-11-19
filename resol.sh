#!/bin/bash
#SBATCH --job-name=Thao
#SBATCH -p E5,Lake,Cascade
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2         # number of MPI processes per node
#SBATCH --cpus-per-task=8           # number of OpenMP threads per MPI process
#SBATCH --time=0-02:30:00
#
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=thi.nguyen1@ens-lyon.fr
module purge
module use /applis/PSMN/debian11/E5/modules/all
module load GCC/10.3.0
module load GSL
#cd cancer_simuscale
rm -rf build
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/tnguye22/bin ..
make simuscale
cd ../run/cancer 
./../../build/bin/simuscale 
