

//Dijkstra.c

//OpenMP example program: Dijkstra shortest-path finder in a 
//bidirectional graph; finds the shortest path from vertex 0 to all
//others

//usage: dijkstra nv print

//where nv is the size of the graph, and print is 1 if graph and min
//distances are to be printed out, 0 otherwise

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
//global variables, shared by all threads by default; could placed them
//above the "parallel" pragma in dowork()

int nv, //number of vertices
   *notdone, //vertices not checked yet
   nth, //number of threads
   md, //current min over all threads
   mv, //vertex which achieves that min
   largeint = -1; //max possible unsigned int

int checked_vertex = 0; //display the path calculated shortest-path  

unsigned *ohd, //1-hop distances between vertices; "ohd[i][j]" is 
	// ohd[i*nv+j]
	*mind; //min distances found so far

void init(int ac, char **av)
{
  int i,j,tmp;
  nv = atoi(av[1]);

  //checked vertex given
  if (ac >= 4) {
    checked_vertex = atoi(av[3]);
    if (checked_vertex > nv)
      checked_vertex %= nv;
    printf("checked vertex at %d\n", checked_vertex);
  }


  ohd = malloc(nv*nv*sizeof(int));
  mind = malloc(nv*sizeof(int));
  notdone = malloc(nv*sizeof(int));
  //random graph


  for (i = 0; i < nv; i++) 
    for (j = i; j < nv; j++) {
      if (j == i) ohd[i*nv+i] = 0;
      else {
	ohd[nv*i+j] = rand() % 200;
	ohd[nv*j+i] = ohd[nv*i+j];
      }

    }
  for (i = 1; i < nv; i++) {
    notdone[i] = 1;
    mind[i] = ohd[i];
  }

}



void dowork()
{
  #pragma omp parallel
  {
    int step, //whole procedure goes nv steps
        mymv; //vertex which attains the min value in my chunk
    unsigned int mymd; //min value found by this thread
    int i;

    for (step = 0; step < nv; step++) {
      //find closest vertex to 0 among notnode; each thread finds
      //closest in its group, then we find overall closest

      #pragma omp single
      { md = largeint; mv = 0; }

      mymd = largeint;
      #pragma omp for private(i)
      for (i = 0; i < nv; i++) {
        if (notdone[i] == 1 && mind[i] < mymd) {
          mymd = mind[i];
          mymv = i;
        }
      }

      //update overall min if min is smaller
      #pragma omp critical
      {
	if (mymd < md)
	  { md = mymd; mv = mymv; }
      } 

      #pragma omp barrier
      //mark new vertex as done

      #pragma omp single
      { notdone[mv] = 0; }
      //now update my section of mind

      #pragma omp for private(i)
      for (i = 0; i < nv; i++)
        if (mind[mv] + ohd[mv*nv+i] < mind[i]) {
          if (i == checked_vertex)
            printf(" +v%d ", mv);
          mind[i] = mind[mv] + ohd[mv*nv+i];
        }

      #pragma omp barrier

    }
    
  }


}

int main(int argc, char **argv)
{
  int i,j,print;
  double startime,endtime;
  init(argc,argv);
  startime = omp_get_wtime();

  //parallel
  dowork();
  //back to single thread
  endtime = omp_get_wtime();

  printf("elapsed time: %f\n", endtime-startime);
  print = atoi(argv[2]);

  if (print) {
   /*
    printf("graph weights:\n");
    for (i = 0; i < nv; i++) {
      for (j = 0; j < nv; j++)
	printf("%u ", ohd[nv*i+j]);
      printf("\n");
    }
   */
    printf("minimum distances:\n");
    for (i = 1; i < nv; i++)
      printf("%u\n", mind[i]);

  }
}




