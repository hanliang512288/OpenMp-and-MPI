#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#include "common.h"
#include "nbody_io.h"
#include "params.h"


/*@T
 * \section{Parallel force computation with OpenMP}
 *
 * For the force computation before, we were a {\em little} bit
 * clever: we used the symmetry of the Lennard-Jones interaction
 * to simultaneously compute the effect of $i$ on $j$ and the effect
 * of $j$ on $i$.  This roughly doubles the speed of the computation
 * (in practice as well as in theory) and makes me happy.  On the
 * other hand, it means that the most obvious parallel [[for]]
 * construct won't work without some synchronization!  For our first
 * attempt at an OpenMP implementation, we will instead accumulate the
 * force contributions from each thread into a local array, and we'll
 * combine those force contributions later.
 *
 * We get some speed-up this way --- on my laptop, it takes 
 * about 35 seconds for the OpenMP code than the 45 seconds that
 * the serial code takes.  We can do better.
 *
 *@c*/
static float** Ftemp;

void ftemp_init(sim_param_t* params)
{
    int n = params->npart;
    Ftemp = malloc(omp_get_max_threads() * sizeof(float*));
    for (int i = 0; i < omp_get_max_threads(); ++i)
        Ftemp[i] = malloc(2*n*sizeof(float));
}

void ftemp_destroy()
{
    for (int i = 0; i < omp_get_max_threads(); ++i)
        free(Ftemp[i]);
    free(Ftemp);
}

void compute_forces(int n, const float* restrict x, float* restrict F, 
                    sim_param_t* params)
{
    float g    = params->G;
    float eps  = params->eps_lj;
    float sig  = params->sig_lj;
    float sig2 = sig*sig;

    /* Global force downward (e.g. gravity) */
    for (int i = 0; i < n; ++i) {
        F[2*i+0] = 0;
        F[2*i+1] = -g;
    }

    /* Particle-particle interactions (Lennard-Jones) */
    #pragma omp parallel shared(F,x,n)
    {
        float* Ft = Ftemp[omp_get_thread_num()];
        memset(Ft, 0, 2*n*sizeof(float));

        #pragma omp for schedule(static)
        for (int i = 0; i < n; ++i) {
            for (int j = i+1; j < n; ++j) {
                float dx = x[2*j+0]-x[2*i+0];
                float dy = x[2*j+1]-x[2*i+1];
                float C_LJ = compute_LJ_scalar(dx*dx+dy*dy, eps, sig2);
                Ft[2*i+0] += (C_LJ*dx);
                Ft[2*i+1] += (C_LJ*dy);
                Ft[2*j+0] -= (C_LJ*dx);
                Ft[2*j+1] -= (C_LJ*dy);
            }
        }

        #pragma omp critical
        for (int i = 0; i < 2*n; ++i)
            F[i] += Ft[i];
    }
}

/*@T
 *
 * The rest of the OpenMP code is nearly identical to the serial code.
 *@q*/

/*
 * \subsection{Initial conditions}
 *
 * We choose the particle velocity components to be normal with mean zero
 * and variance $T_0^2$, and we choose the particles uniformly at random
 * subject to the constrain that no too particles can be closer than
 * the basic interaction radius of Lennard-Jones lest the simulation
 * blow up on the first time steps (we choose $r_{\min} =
 * \sigma)$. It's possible to set the parameters such that we can't
 * place as many particles as desired subject to this constraint; if
 * that happens, we try to place as many as possible, then bail out.
 */
int init_particles_random(int n, float* x, float* v, sim_param_t* params)
{
    const int MAX_INIT_TRIALS = 1000;

    float T0     = params->T0;
    float sig    = params->sig_lj;
    float min_r2 = sig*sig;

    for (int i = 0; i < n; ++i) {
        float r2 = 0;

        /* Choose velocity components from N(0,T0) */
        double R = T0 * sqrt(-2*log(drand48()));
        double T = 2*M_PI*drand48();
        v[2*i+0] = (float) (R * cos(T));
        v[2*i+1] = (float) (R * sin(T));

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


/*
 * \subsection{Simulating a box}
 * 
 * The [[run_box]] routine simulates the motion of a collection of
 * unit-mass particles in a the box $[0,1]^2$ using a leapfrog
 * integration scheme.  Every [[npframes]] time steps, a frame is
 * written to the output file.  As described in the previous section,
 * we use a callback function to compute the force fields at each
 * step.
 */
void run_box(FILE* fp,              /* Output file */
             int n,                 /* Number of particles */
             int npframe,           /* Number of steps between frames */
             int nframes,           /* Number of frames generated */
             float dt,              /* Time step */
             float* restrict x,     /* Initial positions */
             float* restrict v,     /* Initial velocities */
             sim_param_t* params)   /* Simulation parameters */
{
    float* a = (float*) malloc(2*n*sizeof(float));

    memset(a, 0, 2*n*sizeof(float));
    ftemp_init(params);

    write_header(fp, n);
    write_frame_data(fp, n, x);
    compute_forces(n, x, a, params);
    for (int frame = 1; frame < nframes; ++frame) {
        for (int i = 0; i < npframe; ++i) {
            leapfrog1(n, dt, x, v, a);
            apply_reflect(n, x, v, a);
            compute_forces(n, x, a, params);
            leapfrog2(n, dt, v, a);
        }
        write_frame_data(fp, n, x);
    }

    ftemp_destroy();
    free(a);
}

/*
 * \section{The [[main]] event}
 */
int main(int argc, char** argv)
{
    sim_param_t params;
    float* x;
    float* v;
    FILE* fp;
    int npart;

    if (get_params(argc, argv, &params) != 0)
        exit(-1);

    fp = fopen(params.fname, "w");
    x = malloc(2*params.npart*sizeof(float));
    v = malloc(2*params.npart*sizeof(float));

    npart = init_particles_random(params.npart, x, v, &params);
    if (npart < params.npart) {
        fprintf(stderr, "Could not generate %d particles; trying %d\n",
                params.npart, npart);
        params.npart = npart;
    }

    run_box(fp, params.npart, params.npframe, params.nframes, 
            params.dt, x, v, &params);

    free(v);
    free(x);
    fclose(fp);
}
