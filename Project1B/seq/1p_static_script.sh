#!/bin/bash
#PBS -l nodes=1:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 1p_static
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;
do
	./seqstatic 320
done

for i in `seq 1 10`;
do
	./seqstatic 640
done

for i in `seq 1 10`;
do
	./seqstatic 1280
done

for i in `seq 1 10`;
do
	./seqstatic 2560
done

wait
