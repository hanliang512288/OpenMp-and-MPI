#!/bin/bash
#PBS -l nodes=8:ppn=1
#PBS -l walltime=02:00:00
#PBS -l pmem=2000mb
#PBS -N 8n_1p_parallel2
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 128
done

for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 256
done

for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 512
done

for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 1024
done

for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 2048
done
for i in `seq 1 10`;

do
    mpiexec -n 8 ./mpi3 4096
done

wait

