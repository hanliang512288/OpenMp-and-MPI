#!/bin/bash
#PBS -l nodes=8:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 8n_4n_2n_parallel
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
    mpiexec -n 2 ./mpi 1024
done

wait


for i in `seq 1 10`;

do
    mpiexec -n 2 ./mpi 1024
done

wait

for i in `seq 1 10`;

do
    mpiexec -n 2 ./mpi 1024
done

wait