#!/bin/bash
#PBS -l nodes=4:ppn=2
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 4n_2p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
	mpiexec -n 4 ./mpi_rc_dynamic 10240
done

wait