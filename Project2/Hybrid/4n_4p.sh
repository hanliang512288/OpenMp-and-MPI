#!/bin/bash
#PBS -l nodes=4:ppn=4
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 4n_4p_parallel
#PBS -q parallel
#PBS -j oe

cd $PBS_O_WORKDIR


for i in `seq 1 10`;

do
    mpiexec -n 4 ./hybrid 1024 4
done

wait

