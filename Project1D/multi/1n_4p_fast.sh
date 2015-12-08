#!/bin/bash
#PBS -l nodes=1:ppn=4
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_4p_fast
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
	mpiexec -n 1 ./mpi_not_rc_static 10240
done

wait