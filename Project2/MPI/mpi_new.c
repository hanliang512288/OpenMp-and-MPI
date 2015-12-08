#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define G             6.67384E-11
#define PI            3.14159265
#define DT	           0.001
#define T0			   1       

int rank;
int nproc;
MPI_Datatype pairtype;


typedef float data_t;
data_t fRand(float,float,int);

typedef struct body {
  data_t mass;    
  data_t x_pos;     
  data_t y_pos;     
  data_t x_vel;     
  data_t y_vel;     
}Body;


typedef struct forceVec {    
  data_t x_vel;             
  data_t y_vel;
}Force;

inline data_t invDistance(Body* body1, Body* body2)
{
  return (data_t) sqrt((body1->x_pos-body2->x_pos)*(body1->x_pos-body2->x_pos) +
      (body1->y_pos-body2->y_pos)*(body1->y_pos-body2->y_pos));
}

data_t fRand(float max, float min, int seed)     
{
  srand(seed);
  return (data_t) (min + ((float)(rand()/(float)RAND_MAX)*(float)(max-min)));
}


struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

void init_particles_random(int n, float* x){
  int i,mult=12827467;
  for (i =0; i<n;i++){
    x[2*i+0] = fRand(10,-10,i+1*mult);
    x[2*i+1] = fRand(10,-10,i+2*mult);
  }
}

void init_velocity_mass_random(int n, float* v, float* m){
  int i,mult=12827467;
  for (i =0; i<n;i++){
    v[2*i+0] = fRand(10,-10,i+3*mult);
    v[2*i+1] = fRand(10,-10,i+4*mult);
    m[2*i]=fRand(100000.0,-100000.0,i);
    m[2*i+1]=fRand(100000.0,-100000.0,i);
  }
}

void partition_problem(int* iparts, int* counts, int npart)
{
    int num_each = npart/nproc;
    int num_left = npart-num_each*nproc;
    iparts[0] = 0;
    int i =0;
    for (i = 0; i < nproc; ++i) {
        counts[i] = num_each + (i < num_left ? 1 : 0);
        iparts[i+1] = iparts[i] + counts[i];
    }
}

// void nbody_cal(int n,int nlocal,int* iparts, int* counts,float* xlocal, float* vlocal)
// { 
 
//   int i,j;
//   data_t posTemp[totalNum*2];
  
//   for(i=0;i<nlocal;i++)
//   {
//     data_t x_acc=0;
//     data_t y_acc=0;
//     //    data_t z_acc=0;
//     for(j=0;j<n;j++)
//     {
//       if(i!=j){
//         data_t dx = subArr[i].x_pos - bodyArr[j].x_pos;
//         data_t dy = subArr[i].y_pos - bodyArr[j].y_pos;
//         data_t inv = 1.0/sqrt(dx*dx + dy*dy);
//         data_t force = G*mass*mass*inv*inv;
//         x_acc += force*dx;
//         y_acc += force*dy;
//       }
//     }
//     posTemp[i*2] = subArr[i].x_pos + DT*(subArr[i].x_vel) + 0.5*DT*DT*(x_acc);
//     posTemp[i*2+1] = subArr[i].y_pos + DT*(subArr[i].y_vel) + 0.5*DT*DT*(y_acc);
//     subArr[i].x_vel+= DT*(x_acc);
//     subArr[i].y_vel+= DT*(y_acc);

//   }
//   for(i = 0; i < totalNum; i++)
//   {
//     subArr[i].x_pos = posTemp[i*2];
//     subArr[i].y_pos = posTemp[i*2+1];
//   }
// }

void main(int argc, char **argv)
{
  float* x;
  float* xlocal;
  float* vlocal;
  float* mlocal;
  int npart;
  int* iparts;
  int* counts;
  int nlocal;
  int provided;

  npart = atoi(argv[1]);

  MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE, &provided);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Type_vector(1, 2, 1, MPI_FLOAT, &pairtype);
  MPI_Type_commit(&pairtype);

  x = malloc(2*npart*sizeof(float));

  if(rank==0){
    	init_particles_random(npart, x);
  }

  // MPI_Bcast(&npart, 1, MPI_INT, 0, MPI_COMM_WORLD);
  // MPI_Bcast(x, npart, pairtype, 0, MPI_COMM_WORLD);

  // counts = malloc( nproc   *sizeof(int));
  // iparts = malloc((nproc+1)*sizeof(int));
  // partition_problem(iparts, counts, npart);
  // nlocal = counts[rank];

  // xlocal = malloc(2*nlocal*sizeof(float));
  // vlocal = malloc(2*nlocal*sizeof(float));
  // mlocal = malloc(2*nlocal*sizeof(float));
  // memcpy(xlocal, x+2*iparts[rank], 2*nlocal*sizeof(float));
  // init_velocity_mass_random(nlocal,vlocal,mlocal);
  // int i;
  // for (i = 0;i<1000;i++){
  //   // nbody_cal(npart,nlocal,x,xlocal,vlocal,mlocal);
  //   MPI_Allgatherv(xlocal,nlocal,pairtype,x,counts,iparts,pairtype,MPI_COMM_WORLD);
  // }

  printf("%f",nproc);
  MPI_Finalize();
}
