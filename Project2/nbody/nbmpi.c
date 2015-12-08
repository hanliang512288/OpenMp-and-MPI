#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#include "common.h"
#include "nbody_io.h"
#include "params.h"

/*@T
 * \section{MPI driver}
 * 
 * Somewhat remarkably, this MPI code gets better performance than
 * my serial code on my two-core laptop.  I think this is a testament
 * to the cleverness of the OpenMPI programmers more than any sort of
 * statement about my (deliberately naive) code.  My strategy is to have
 * each processor claim ownership of a small number of particles, and
 * to compute their force interactions and position updates.  Then the
 * information gets exchanged via an [[MPI_Allgatherv]].  This was the most
 * straightforward parallelization I could think of --- and I figured
 * it would take much more work than I have with a few hundred particles
 * to mask the latency of an all-to-all communication that I used at each
 * step.
 *
 * \subsection{Global state}
 * 
 * Usually, global variables are a Bad Idea.  In this case, however:
 * \begin{enumerate}
 * \item The rank, size, and data types are written only once.
 * \item They are read from nearly everywhere.
 * \item They are at least declared [[static]] (local to the current file).
 * \end{enumerate}
 *@c*/
static int rank;
static int nproc;
static MPI_Datatype pairtype;

/*@T
 * \subsection{Problem partitioning}
 *
 * Divide the problem more-or-less evenly among processors.  Every
 * processor at least gets [[num_each]] = $\lfloor n/p \rfloor$
 * particles, and the first few processors each get one more.
 *@c*/
void partition_problem(int* iparts, int* counts, int npart)
{
    int num_each = npart/nproc;
    int num_left = npart-num_each*nproc;
    iparts[0] = 0;
    for (int i = 0; i < nproc; ++i) {
        counts[i] = num_each + (i < num_left ? 1 : 0);
        iparts[i+1] = iparts[i] + counts[i];
    }
}

/*@T
 *
 * \subsection{The force computation}
 * 
 * Right now, we take local and global arrays of positions and a
 * local array of forces, and compute the force into a local array.
 * The local position array corresponds to indices [[istart <= i < iend]]
 * in the global array.  Apart from not trying to be clever about
 * re-using computations, this looks much the same as (though not identical
 * to) the serial code.
 *@c*/
void compute_forces(int n, const float* restrict x, 
                    int istart, int iend, 
                    const float* restrict xlocal, float* restrict Flocal,
                    sim_param_t* params)
{
    int nlocal = iend-istart;
    float g    = params->G;
    float eps  = params->eps_lj;
    float sig  = params->sig_lj;
    float sig2 = sig*sig;

    /* Global force downward (e.g. gravity) */
    for (int i = 0; i < nlocal; ++i) {
        Flocal[2*i+0] = 0;
        Flocal[2*i+1] = -g;
    }

    /* Particle-particle interactions (Lennard-Jones) */
    for (int i = istart; i < iend; ++i) {
        int ii = i-istart;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                float dx = x[2*j+0]-xlocal[2*ii+0];
                float dy = x[2*j+1]-xlocal[2*ii+1];
                float C_LJ = compute_LJ_scalar(dx*dx+dy*dy, eps, sig2);
                Flocal[2*ii+0] += (C_LJ*dx);
                Flocal[2*ii+1] += (C_LJ*dy);
            }
        }
    }
}

/*@T
 * \subsection{Initial conditions}
 *
 * Previously, we chose the particles and their velocities simultaneously.
 * Now, it doesn't make sense to do so.  I know how to generate a random
 * initial particle distribution by rejection on a single node given
 * all the point coordinates, but I don't need to do this for the velocities.
 *@c*/
int init_particles_random(int n, float* x, sim_param_t* params)
{
    const int MAX_INIT_TRIALS = 1000;

    float sig    = params->sig_lj;
    float min_r2 = sig*sig;

    for (int i = 0; i < n; ++i) {
        float r2 = 0;

        /* Choose new point via rejection sampling */
        for (int trial = 0; r2 < min_r2 && trial < MAX_INIT_TRIALS; ++trial) {
            x[2*i+0] = (float) drand48();
            x[2*i+1] = (float) drand48();
            for (int j = 0; j < i; ++j) {
                float dx = x[2*i+0]-x[2*j+0];
                float dy = x[2*i+1]-x[2*j+1];
                r2 = dx*dx + dy*dy;
                if (r2 < min_r2)
                    break;
            }
        }

        /* If it takes too many trials, bail and declare victory! */
        if (i > 0 && r2 < min_r2) 
            return i;
    }
    return n;
}


void init_particles_random_v(int n, float* v, sim_param_t* params)
{
    float T0 = params->T0;
    for (int i = 0; i < n; ++i) {
        double R = T0 * sqrt(-2*log(drand48()));
        double T = 2*M_PI*drand48();
        v[2*i+0] = (float) (R * cos(T));
        v[2*i+1] = (float) (R * sin(T));
    }
}

/*@T
 *
 * \subsection{Simulating a box}
 * 
 * The [[run_box]] code looks similar to the serial code,
 * except for a communication phase ([[MPI_Allgatherv]])
 * immediately after updating the positions.  If MPI-2
 * had nonblocking collective operations, this would have been
 * a perfect place to use them.
 *
 * Note that we output whenever the output file [[fp]] is
 * non-[[NULL]].
 *@c*/
void run_box(FILE* fp,                  /* Output file (at 0) */
             int n, int nlocal,         /* Counts (all and local) */
             int* iparts, int* counts,  /* Offsets and counts per proc */
             int npframe,               /* Steps per frame */
             int nframes,               /* Frames */
             float dt,                  /* Time step */
             float* restrict x,         /* Global position vec */
             float* restrict xlocal,    /* Local part of position */
             float* restrict vlocal,    /* Local part of velocity */
             sim_param_t* params)       /* Simulation params */
{
    float* alocal = (float*) malloc(2*nlocal*sizeof(float));
    memset(alocal, 0, 2*nlocal*sizeof(float));

    if (fp) {
        write_header(fp, n);
        write_frame_data(fp, n, x);
    }

    compute_forces(n, x, iparts[rank], iparts[rank+1],
                   xlocal, alocal, params);

    for (int frame = 1; frame < nframes; ++frame) {
        for (int i = 0; i < npframe; ++i) {
            leapfrog1(nlocal, dt, xlocal, vlocal, alocal);
            apply_reflect(nlocal, xlocal, vlocal, alocal);
            MPI_Allgatherv(xlocal, nlocal, pairtype,
                           x, counts, iparts, pairtype,
                           MPI_COMM_WORLD);
            compute_forces(n, x, iparts[rank], iparts[rank+1],
                           xlocal, alocal, params);
            leapfrog2(nlocal, dt, vlocal, alocal);
        }
        if (fp)
            write_frame_data(fp, n, x);
    }

    free(alocal);
}

/*@T
 *
 * \subsection{The [[main]] event}
 *
 * As with many MPI codes, this code has a lot of bookkeeping, most
 * of which I currently have in [[main]].  Other than the usual
 * initialization, finalization, and inquiries, we have the following
 * more interesting features:
 * \begin{enumerate}
 * \item
 *   We set up an MPI type ([[pairtype]]) to denote a pair of
 *   floating point numbers.  Note that we have to do an [[MPI_Type_commit]]
 *   before we can make such a constructed data type useable to the rest
 *   of the system.
 * \item
 *   We initialize on the first processor, then send information out to
 *   the others using an [[MPI_Bcast]].
 * \end{enumerate}
 *@c*/
int main(int argc, char** argv)
{
    sim_param_t params;
    float* x;
    float* xlocal;
    float* vlocal;
    FILE* fp = NULL;
    int npart;
    int* iparts;
    int* counts;
    int nlocal;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Type_vector(1, 2, 1, MPI_FLOAT, &pairtype);
    MPI_Type_commit(&pairtype);

    if (get_params(argc, argv, &params) != 0) {
        MPI_Finalize();
        exit(-1);
    }

    /* Get file handle and initialize everything on P0 */
    x = malloc(2*params.npart*sizeof(float));
    if (rank == 0) {
        fp = fopen(params.fname, "w");
        npart = init_particles_random(params.npart, x, &params);
        if (npart < params.npart) {
            fprintf(stderr, "Could not generate %d particles; trying %d\n",
                    params.npart, npart);
        }
    }

    /* Broadcast initial information from root */
    MPI_Bcast(&npart, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(x, npart, pairtype, 0, MPI_COMM_WORLD);
    params.npart = npart;

    /* Decide who is responsible for which item */
    counts = malloc( nproc   *sizeof(int));
    iparts = malloc((nproc+1)*sizeof(int));
    partition_problem(iparts, counts, npart);
    nlocal = counts[rank];

    /* Allocate space for local storage and copy in data */
    xlocal = malloc(2*nlocal*sizeof(float));
    vlocal = malloc(2*nlocal*sizeof(float));
    memcpy(xlocal, x+2*iparts[rank], 2*nlocal*sizeof(float));
    init_particles_random_v(nlocal, vlocal, &params);

    run_box(fp, npart, nlocal, iparts, counts,
            params.npframe, params.nframes, 
            params.dt, x, xlocal, vlocal, &params);

    free(vlocal);
    free(xlocal);
    free(iparts);
    free(x);
    if (fp)
        fclose(fp);

    MPI_Finalize();
    return 0;
}
