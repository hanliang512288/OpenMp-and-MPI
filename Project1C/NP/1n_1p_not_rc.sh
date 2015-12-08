#!/bin/bash
#PBS -l nodes=1:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_1p_not_rc
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR


for i in `seq 1 10`;

do
	mpiexec -n 1 ./mpi_not_rc 2560
done


wait