#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "common.h"
#include "nbody_io.h"
#include "params.h"

/*@T
 * \section{Serial driver}
 *
 * \subsection{The force field abstraction}
 * 
 * Because I wanted to play with a variety of combinations of potentials,
 * I decided to set up a system in which the force field is not hard-wired
 * into the integrator.  Instead, the integrator calls the force field
 * evaluation via a function pointer of type [[compute_force_t]].
 *
 * The assumed data layout of the [[x]] array is 
 * $[x_1, y_1, x_2, y_2, \ldots, x_n, y_n]$.
 * The [[F]] array is treated similarly.
 *@c*/
typedef
void (*compute_force_t)(int n,                    /* Particle count */
                        const float* restrict x,  /* Positions */
                        float* restrict F,        /* Forces */
                        void* fdata);             /* Parameters */

/*@T
 *
 * Despite the function abstraction, right now there's only one force
 * computation: Lennard-Jones potential plus a gravitational field.
 *@c*/
void compute_forces(int n, const float* restrict x, float* restrict F, 
                    void* fdata)
{
    sim_param_t* params = (sim_param_t*) fdata;
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
    for (int i = 0; i < n; ++i) {
        for (int j = i+1; j < n; ++j) {
            float dx = x[2*j+0]-x[2*i+0];
            float dy = x[2*j+1]-x[2*i+1];
            float C_LJ = compute_LJ_scalar(dx*dx+dy*dy, eps, sig2);
            F[2*i+0] += (C_LJ*dx);
            F[2*i+1] += (C_LJ*dy);
            F[2*j+0] -= (C_LJ*dx);
            F[2*j+1] -= (C_LJ*dy);
        }
    }
}

/*@T
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
 *@c*/
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


/*@T
 * \subsection{Simulating a box}
 * 
 * The [[run_box]] routine simulates the motion of a collection of
 * unit-mass particles in a the box $[0,1]^2$ using a leapfrog
 * integration scheme.  Every [[npframes]] time steps, a frame is
 * written to the output file.  As described in the previous section,
 * we use a callback function to compute the force fields at each
 * step.
 *@c*/
void run_box(FILE* fp,              /* Output file */
             int n,                 /* Number of particles */
             int npframe,           /* Number of steps between frames */
             int nframes,           /* Number of frames generated */
             float dt,              /* Time step */
             float* restrict x,     /* Initial positions */
             float* restrict v,     /* Initial velocities */
             compute_force_t force, /* Function to compute force */
             void* force_data)      /* Data used by force() */
{
    float* a = (float*) malloc(2*n*sizeof(float));
    memset(a, 0, 2*n*sizeof(float));

    write_header(fp, n);
    write_frame_data(fp, n, x);
    force(n, x, a, force_data);
    for (int frame = 1; frame < nframes; ++frame) {
        for (int i = 0; i < npframe; ++i) {
            leapfrog1(n, dt, x, v, a);
            apply_reflect(n, x, v, a);
            force(n, x, a, force_data);
            leapfrog2(n, dt, v, a);
        }
        write_frame_data(fp, n, x);
    }

    free(a);
}

/*@T
 * \subsection{The [[main]] event}
 *@c*/
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
            params.dt, x, v, compute_forces, &params);

    free(v);
    free(x);
    fclose(fp);
}
