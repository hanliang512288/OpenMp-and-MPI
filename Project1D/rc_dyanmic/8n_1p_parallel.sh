#!/bin/bash
#PBS -l nodes=8:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 8n_1p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
	mpiexec -n 8 ./mpi_rc_dynamic 10240
done

wait