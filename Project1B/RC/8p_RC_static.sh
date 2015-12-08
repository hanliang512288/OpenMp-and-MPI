#!/bin/bash
#PBS -l nodes=2:ppn=4
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 8p_rc_static
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;
do
	./rc 2560
done

wait