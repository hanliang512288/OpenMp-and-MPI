#!/bin/bash
#PBS -l nodes=16:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 16n_1p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS = 1

for i in `seq 1 10`;

do
    mpiexec -n 16 ./hybrid 1024
done

wait

