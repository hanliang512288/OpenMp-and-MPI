#!/bin/bash
#PBS -l nodes=2:ppn=8
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_8p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS = 8

for i in `seq 1 10`;

do
    mpiexec -n 2 ./mpi 1024
done

wait

