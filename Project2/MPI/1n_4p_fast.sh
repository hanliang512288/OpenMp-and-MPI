#!/bin/bash
#PBS -l nodes=1:ppn=4
#PBS -l walltime=04:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_4p_fast
#PBS -q fast
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

