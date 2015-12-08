# -- Compiler settings for my laptop
#CFLAGS=-std=gnu99 -O3 -ftree-vectorize -march=core2 -Wall
#CC=gcc-4.2

# -- Compiler settings for the cluster
CC = /share/apps/local/bin/gcc
CFLAGS=-std=gnu99 -O3 -march=native -Wall
LIBS=-lm
MPICC=OMPI_CC=$(CC) mpicc
NPROC=8

.PHONY: all clean realclean

all: nbserial.x nbomp.x nbmpi.x

run.out: nbserial.x
	./nbserial.x

# =======
nbserial.x: nbserial.o common.o nbody_bin_io.o params.o
	$(CC) -o $@ $^ $(LIBS)

nbomp.x: nbomp.o common.o nbody_bin_io.o params.o
	$(CC) -o $@ -fopenmp $^ $(LIBS)

nbmpi.x: nbmpi.o common.o nbody_bin_io.o params.o
	$(MPICC) -o $@ $^ $(LIBS)

nbomp.o: nbomp.c
	$(CC) -c $(CFLAGS) -fopenmp $<

nbmpi.o: nbmpi.c
	$(MPICC) -c $(CFLAGS) $<

%.o: %.c
	$(CC) -c $(CFLAGS) $<

# =======
run:
	qsub run_serial.qsub

run-omp:
	qsub run_omp.qsub

run-mpi:
	qsub -pe orte $(NPROC) run_mpi.qsub

# =======
main.pdf: main.tex codes.tex
	pdflatex $<
	pdflatex $<

codes.tex: params.h common.c nbserial.c nbomp.c nbmpi.c \
	params.c nbody_bin_io.c
	dsbweb -o $@ -p macros.tex -c $^

view: run.out
	java -jar ../jbouncy/Bouncy.jar run.out

# =======
tgz: realclean
	(cd ..; tar -czf nbody.tgz nbody/*)

clean:
	rm -f *~ *.o
	rm -f codes.tex main.log main.aux main.out
	rm -f *.qsub.o* *.qsub.po*

realclean: clean
	rm -f main.pdf main.toc *.x run.out

