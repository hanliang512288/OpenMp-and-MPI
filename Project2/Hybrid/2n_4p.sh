#!/bin/bash
#PBS -l nodes=2:ppn=4
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_8p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS = 4
for i in `seq 1 10`;

do
    mpiexec -n 2 ./hybrid 1024
done

wait

