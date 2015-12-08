/*
 * Fist cut at the Nbody simple naive algorithm with zero
 * optimizations at all. This will serve as our absolute 
 * baseline. We will need to look at units to make sure that all
 * units align well. I suggest using metric.
 *
 * */
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
// #include <errno.h>


#define G             6.67384E-11
#define PI            3.14159265
#define DT	      0.001           //  0.001 second time increments

int nbodynum,chunksize;
int nnodes;
int my_rank;

typedef float data_t;

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

data_t fRand(float,float,int);

int initBodies(Body* bodies, int Num)
{
  // Initialize all of the bodies
  int i,mult=12827467;
  for(i=0; i< Num; i++)
  {
    bodies[i].mass = fRand(100000.0,-100000.0,i);
    bodies[i].x_pos = fRand(10,-10,i+mult);
    bodies[i].y_pos = fRand(10,-10,i+2*mult);
    bodies[i].x_vel = fRand(10,-10,i+3*mult);
    bodies[i].y_vel = fRand(10,-10,i+4*mult);
    mult+=81722171%192323820820392;
  }
}

data_t fRand(float max, float min, int seed)     
{
  srand(seed);
  return (data_t) (min + ((float)(rand()/(float)RAND_MAX)*(float)(max-min)));
}


int findmyrange(int n, int nth, int me)
{
  int chunksize = n / nth;
  int range;
  range = me * chunksize;
  return range;
}

void NbodyCalc(Body* bodyArr, int totalNum, Body* subArr, int chunksize)
{
  int i,j;
  data_t posTemp[totalNum*2];
  
  for(i=0;i<chunksize;i++)
  {
    data_t x_acc=0;
    data_t y_acc=0;
    //    data_t z_acc=0;
    for(j=0;j<totalNum;j++)
    {
      if(i!=j){
        data_t dx = subArr[i].x_pos - bodyArr[j].x_pos;
        data_t dy = subArr[i].y_pos - bodyArr[j].y_pos;
        //        data_t dz = bodyArr[i].z_pos - bodyArr[j].z_pos;
        data_t inv = 1.0/sqrt(dx*dx + dy*dy);
        data_t force = G*bodyArr[j].mass*subArr[i].mass*inv*inv;
        x_acc += force*dx;
        y_acc += force*dy;
      }
    }
    posTemp[i*2] = subArr[i].x_pos + DT*(subArr[i].x_vel) + 0.5*DT*DT*(x_acc);
    posTemp[i*2+1] = subArr[i].y_pos + DT*(subArr[i].y_vel) + 0.5*DT*DT*(y_acc);
    subArr[i].x_vel+= DT*(x_acc);
    subArr[i].y_vel+= DT*(y_acc);

  }
  for(i = 0; i < totalNum; i++)
  {
    subArr[i].x_pos = posTemp[i*2];
    subArr[i].y_pos = posTemp[i*2+1];
  }
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

void main(int argc, char **argv)
{

  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp;

  nbodynum = atoi(argv[1]);
  Body b[nbodynum];

  initBodies(b,nbodynum);

  int provided, claimed;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  MPI_Query_thread(&claimed);
  MPI_Comm_size(MPI_COMM_WORLD, &nnodes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  chunksize = nbodynum/nnodes;

  int myrange;
  myrange = findmyrange(nbodynum,nnodes,my_rank);

  Body sub_b[chunksize];

  int i = myrange;
  int j;
  for(j = 0;j < chunksize;j++){
    int b_index = myrange+j;
    sub_b[j] = b[b_index];

  }
  clock_gettime(CLOCK_REALTIME, &time1);
  NbodyCalc(b,nbodynum,sub_b,chunksize);
  printf("%i",myrange);
  clock_gettime(CLOCK_REALTIME, &time2);
  time_stamp = diff(time1,time2);

  // printf("%f\n",time_stamp);
  printf("Execution time: %lf\n",(double)((time_stamp.tv_sec + (time_stamp.tv_nsec/1.0e9))));
  MPI_Finalize();
}
