#!/bin/bash
#
### variables SGE
#
### shell du job
#$ -S /bin/bash
### nom du job (a changer)
#$ -N ToggleSwitch2Genes
### file d'attente (a changer)
#$ -q E5-*
### charger l'environnement utilisateur pour SGE
#$ -cwd
### exporter les variables d'environnement sur tous les noeuds d'execution
#$ -V
### mails en debut et fin d'execution
#$ -m be
# aller dans le repertoire de travail/soumission
# important, sinon, le programme est lancé depuis ~/
cd ${SGE_O_WORKDIR}
### configurer l'environnement (a changer)
alias python=python3 
module load Python/3.8.3 
### export PATH=$PATH:/home/mmarti30/.local/bin

### definition SCRATCHDIR en fonction de la partition existent
 
if [[ -d "/scratch/Lake" ]]
then
    SCRATCHDIR="/scratch/Lake/${USER}/${JOB_ID}/"
elif [[ -d "/scratch/E5N" ]]
then
    SCRATCHDIR="/scratch/E5N/${USER}/${JOB_ID}/" 
else
    echo "Cannot create ${SCRATCHDIR} on /scratch, creating it in the current directory"
    SCRATCHDIR="${SGE_O_WORKDIR}/scratch/"
fi
 
###SCRATCHDIR=/scratch/E5N/mmarti30/Formations/Sequentiel
 
### verif SCRATCHDIR
echo "SCRATCHDIR=${SCRATCHDIR}"
 
### creation du repertoire de travail dans le /scratch
if [[ ! -d "${SCRATCHDIR}" ]] 
then
   /bin/mkdir -p ${SCRATCHDIR}
fi 
 
### copie des fichiers sources dans le /scratch
/bin/cp ${SGE_O_WORKDIR}/*.sh ${SCRATCHDIR}/
 
### se placer dans le repertoire d'execution AVANT le lancement du programme
cd ${SCRATCHDIR}
 
### execution du programme
EXECDIR=${SCRATCHDIR}

PREFIXPYTHON="/applis/PSMN/debian9/software/Core/Python/3.8.3"
PYTHONRUN="${PREFIXPYTHON}"/bin/python3
 
PREFIX="/home/mmarti30"
SIMUSCALERUN="${PREFIX}"/simuscale/build/bin/simuscale
/bin/cp "${PREFIX}"/cyclingparam.txt ${SCRATCHDIR}/.
/bin/cp "${PREFIX}"/param.in ${SCRATCHDIR}/.
/bin/cp "${PREFIX}"/GeneToggleSwitch.txt ${SCRATCHDIR}/GeneInteractionsMatrix.txt
/bin/cp "${PREFIX}"/kineticsparam.txt ${SCRATCHDIR}/.
/bin/cp "${PREFIX}"/WriteResults.py ${SCRATCHDIR}/.
/bin/cp "${PREFIX}"/visu3D.py ${SCRATCHDIR}/.
"${SIMUSCALERUN}"> output_messages
###

# copy results from scratch to home
/bin/mkdir $SGE_O_WORKDIR/simuscale/results/${JOB_ID}
/bin/cp ${SCRATCHDIR}/backup* $SGE_O_WORKDIR/simuscale/results/${JOB_ID}/.
/bin/cp ${SCRATCHDIR}/*.txt $SGE_O_WORKDIR/simuscale/results/${JOB_ID}/.
/bin/cp ${SCRATCHDIR}/param.in $SGE_O_WORKDIR/simuscale/results/${JOB_ID}/.
 

# rm -fr ${SCRATCHDIR}/*
 
# fin
