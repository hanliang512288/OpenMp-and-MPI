#!/bin/bash
#PBS -N test_256_2
#PBS -q fast
#PBS -l nodes=1:ppn=2
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -j oe

./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
./compiled 256 n 0
