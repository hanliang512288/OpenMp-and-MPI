#!/bin/bash
#PBS -l nodes=16:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 16n_1p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 128
done

for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 256
done

for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 512
done

for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 1024
done

for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 2048
done
for i in `seq 1 10`;

do
    mpiexec -n 16 ./mpi3 4096
done


wait

