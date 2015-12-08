#!/bin/bash
#PBS -l nodes=1:ppn=16
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1n_16p_fast
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS = 16

for i in `seq 1 10`;

do
    mpiexec -n 1 ./hybrid 1024
done

wait
