#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>


#define G             6.67384E-11
#define DT	          0.001   
        
int nbodynum,chunksize;
int nnodes;
int my_rank;
int number_threads;

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


data_t fRand(float,float,int);

int initBodies(Body* bodies, int Num)
{
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

void NbodyCalc(Body* bodyArr, int totalNum, Body* subArr,int chunksize)
{
  int i,j;
  data_t posTemp[totalNum*2];
omp_set_num_threads(number_threads);   
#pragma omp parallel private(j)
{
  #pragma omp for schedule(static)
  for(i=0;i<chunksize;i++)
  {
    data_t x_acc=0;
    data_t y_acc=0;
    for(j=0;j<totalNum;j++)
    {
      data_t dx = subArr[i].x_pos - bodyArr[j].x_pos;
      data_t dy = subArr[i].y_pos - bodyArr[j].y_pos;
      data_t inv = 1.0/sqrt(dx*dx + dy*dy);
      data_t force = G*bodyArr[j].mass*subArr[i].mass*inv*inv;
      x_acc += force*dx;
      y_acc += force*dy;
    }
    posTemp[i*2] = subArr[i].x_pos + DT*(subArr[i].x_vel) + 0.5*DT*DT*(x_acc);
    posTemp[i*2+1] = subArr[i].y_pos + DT*(subArr[i].y_vel) + 0.5*DT*DT*(y_acc);
    subArr[i].x_vel+= DT*(x_acc);
    subArr[i].y_vel+= DT*(y_acc);
  }

 for(i = 0; i < chunksize; i++)
  {
    subArr[i].x_pos = posTemp[i*2];
    subArr[i].y_pos = posTemp[i*2+1];
  }
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
  number_threads ＝ atoi(argv[2]);
  Body b[nbodynum];

  int provided, claimed;
  MPI_Init(&argc, &argv);
  MPI_Query_thread(&claimed);
  MPI_Comm_size(MPI_COMM_WORLD, &nnodes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  chunksize = nbodynum/nnodes;
 

  const int nitems=5;
  int blocklengths[5] = {1,1,1,1,1};
  MPI_Datatype types[5] = {MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_FLOAT};
  MPI_Datatype mpi_body_type;
  MPI_Aint     offsets[5];

  offsets[0] = offsetof(Body, mass);
  offsets[1] = offsetof(Body, x_pos);
  offsets[2] = offsetof(Body, y_pos);
  offsets[3] = offsetof(Body, x_vel);
  offsets[4] = offsetof(Body, y_vel);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_body_type);
  MPI_Type_commit(&mpi_body_type);

  initBodies(b,nbodynum);
  
  clock_gettime(CLOCK_REALTIME, &time1);
  Body update_body[nbodynum];
  int z;
  for(z=0;z<1000;z++){
      MPI_Bcast(b,nbodynum,mpi_body_type,0,MPI_COMM_WORLD);
      MPI_Barrier(MPI_COMM_WORLD);
      Body sub_b[chunksize];
      MPI_Scatter(b,chunksize,mpi_body_type,sub_b,chunksize,mpi_body_type,0,MPI_COMM_WORLD);
      NbodyCalc(b,nbodynum,sub_b,chunksize);
      MPI_Gather(&sub_b,chunksize,mpi_body_type,b,chunksize, mpi_body_type,0,MPI_COMM_WORLD);
  }
  clock_gettime(CLOCK_REALTIME, &time2);
  // time_stamp = diff(time1,time2);
  
  if (my_rank == 0){
    time_stamp = diff(time1,time2);
    printf("%lf\n",(double)((time_stamp.tv_sec + (time_stamp.tv_nsec/1.0e9))));
  }


  // printf("Execution time: %lf\n",(double)((time_stamp.tv_sec + (time_stamp.tv_nsec/1.0e9))));
  MPI_Finalize();
}
