#!/bin/bash
#PBS -l nodes=8:ppn=2
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 8n_2p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS = 2

for i in `seq 1 10`;

do
    mpiexec -n 8 ./hybrid 1024
done

wait

