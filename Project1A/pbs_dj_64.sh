 -q fast
#PBS -l nodes=1:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -j oe

#PBS -N test_64
#PBS -q fast
#PBS -l nodes=1:ppn=1
#PBS -l walltime=01:00:00
#PBS -l pmem=2000mb
#PBS -j oe

./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
./compiled 64 n 0
