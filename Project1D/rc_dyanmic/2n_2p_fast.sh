#!/bin/bash
#PBS -l nodes=2:ppn=2
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 2n_2p_fast
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
	mpiexec -n 2 ./mpi_rc_dynamic 10240
done

wait