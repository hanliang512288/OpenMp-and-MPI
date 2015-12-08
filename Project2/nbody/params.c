#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "params.h"


/*@T
 * \section{Option processing}
 * 
 * The [[print_usage]] command documents the options to the [[nbody]]
 * driver program, and [[default_params]] sets the default parameter
 * values.  You may want to add your own options to control
 * other aspects of the program.  This is about as many options as
 * I would care to handle at the command line --- maybe more!  Usually,
 * I would start using a second language for configuration (e.g. Lua)
 * to handle anything more than this.
 *@c*/
static void print_usage()
{
    fprintf(stderr, 
            "nbody\n"
            "\t-h: print this message\n"
            "\t-o: output file name (run.out)\n"
            "\t-n: number of particles (500)\n"
            "\t-F: number of frames (200)\n"
            "\t-f: steps per frame (100)\n"
            "\t-t: time step (1e-4)\n"
            "\t-e: epsilon parameter in LJ potential (1)\n"
            "\t-s: distance parameter in LJ potential (1e-2)\n"
            "\t-g: gravitational field strength (1)\n"
            "\t-T: initial temperature (1)\n");
}

static void default_params(sim_param_t* params)
{
    params->fname   = "run.out";
    params->npart   = 500;
    params->nframes = 400;
    params->npframe = 50;
    params->dt      = 1e-4;
    params->eps_lj  = 1;
    params->sig_lj  = 1e-2;
    params->G       = 1;
    params->T0      = 1;
}

/*@T
 *
 * The [[get_params]] function uses the [[getopt]] package
 * to handle the actual argument processing.  Note that
 * [[getopt]] is {\em not} thread-safe!  You will need to
 * do some synchronization if you want to use this function
 * safely with threaded code.
 *@c*/
int get_params(int argc, char** argv, sim_param_t* params)
{
    extern char* optarg;
    const char* optstring = "ho:n:F:f:t:e:s:g:T:";
    int c;

    #define get_int_arg(c, field) \
        case c: params->field = atoi(optarg); break
    #define get_flt_arg(c, field) \
        case c: params->field = (float) atof(optarg); break

    default_params(params);
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
        case 'h': 
            print_usage(); 
            return -1;
        case 'o':
            strcpy(params->fname = malloc(strlen(optarg)+1), optarg);
            break;
        get_int_arg('n', npart);
        get_int_arg('F', nframes);
        get_int_arg('f', npframe);
        get_flt_arg('t', dt);
        get_flt_arg('e', eps_lj);
        get_flt_arg('s', sig_lj);
        get_flt_arg('g', G);
        get_flt_arg('T', T0);
        default:
            fprintf(stderr, "Unknown option\n");
            return -1;
        }
    }
    return 0;
}
