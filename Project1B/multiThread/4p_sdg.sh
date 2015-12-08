#!/bin/bash
#PBS -l nodes=1:ppn=4
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -N 4p_sdg
#PBS -q fast
#PBS -j oe

cd $PBS_O_WORKDIR

for i in `seq 1 10`;
do
	./static 2560
done
echo "\n"

for i in `seq 1 10`;
do
	./dynamic 2560
	
done

echo "\n"

for i in `seq 1 10`;
do
	./guided 2560
	
done

echo "\n"

for i in `seq 1 10`;
do
	./rc 2560
done

echo "\n"
wait
