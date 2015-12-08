#ifndef PARAMS_H
#define PARAMS_H

/*@T
 * \section{System parameters}
 * 
 * The [[sim_param_t]] structure holds the parameters that
 * describe the simulation.  These parameters are filled in
 * by the [[get_params]] function (described later).
 *@c*/
typedef struct sim_param_t {
    char* fname;   /* File name (run.out)        */
    int   npart;   /* Number of particles (500)  */
    int   nframes; /* Number of frames (200)     */
    int   npframe; /* Steps per frame (100)      */
    float dt;      /* Time step (1e-4)           */
    float eps_lj;  /* Strength for L-J (1)       */
    float sig_lj;  /* Radius for L-J   (1e-2)    */
    float G;       /* Gravitational strength (1) */
    float T0;      /* Initial temperature (1)    */
} sim_param_t;

int get_params(int argc, char** argv, sim_param_t* params);

/*@q*/
#endif /* PARAMS_H */
