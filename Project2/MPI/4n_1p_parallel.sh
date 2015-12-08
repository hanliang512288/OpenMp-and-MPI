#!/bin/bash
#PBS -l nodes=4:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 4n_1p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 128
done
for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 256
done
for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 512
done

for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 1024
done

for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 2048
done

for i in `seq 1 10`;

do
    mpiexec -n 4 ./mpi3 4096
done

wait

