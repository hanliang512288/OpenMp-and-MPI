#!/bin/bash
#PBS -l nodes=1:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_1p_fast
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
	mpiexec -n 1 ./mpi_rc_dynamic 10240
done

wait