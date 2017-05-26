#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//Global variables
int* numbers;
int clusterSize;

//Thread functions
void* bubblesort(void* initPos);
void* combine(void* paramsPointer);
int printClusters(int totalnums, int groupc);

//Variables to send to union
struct parameters{
  int initPos;
  int clusterMultiplier;
};

int main(int argc, char const *argv[])
{
  //Declare Variables
  int i, j;
  clusterSize = (argc-1)/8;
  pthread_t* tid = (pthread_t*)malloc(8*sizeof(pthread_t));
  pthread_attr_t attr;

  //Allocate space for and fill numbers array
  numbers = (int*)malloc((argc-1)*sizeof(int));
  for(i=0; i<argc-1; i++){
    numbers[i] = atoi(argv[i+1]);
  }

  //Call Bubble sort per cluster in parallel
  pthread_attr_init(&attr);
  for(i=0; i<8; i++){
    int *position = (int*)malloc(sizeof(int));
    *position = i*clusterSize;
    pthread_create(&(tid[i]), &attr, bubblesort, position);
  }

  //Confirm Bubble sort completion
  for(i=0; i<8; i++){
    pthread_join(tid[i], NULL);
  }

  //Print clusters
  printClusters(argc-1, 8);

  //Call mathematical union in parallel
  for (i = 4; i > 0; i/=2){
      for (j = 0; j < i; j++){
        struct parameters* params = (struct parameters*)malloc(sizeof(struct parameters));
        (*params).initPos = j*clusterSize*(8/i);
        (*params).clusterMultiplier = (8/(i*2));
        pthread_create(&(tid[j]), &attr, combine, params);
      }
      for (j = 0; j < i; j++)
        pthread_join(tid[j], NULL);
      //Print clusters
      printClusters(argc-1, i);
    }

  return 0;
}

//Bubble sort
void* bubblesort(void* initPos){
  int c, d, swap, position;
  position = *((int*)initPos);
  for (c = 0 ; c < ( clusterSize - 1 ); c++){
    for (d = 0 ; d < clusterSize - c - 1; d++){
      if (numbers[position+d] > numbers[position+d+1]){
        swap = numbers[position+d];
        numbers[position+d] = numbers[position+d+1];
        numbers[position+d+1] = swap;
      }
    }
  }
  free(initPos);
  pthread_exit(0);
}

//Combine
void* combine(void* paramsPointer){
  struct parameters params = *((struct parameters*)paramsPointer);
  int *arrA = (int*)malloc(clusterSize*params.clusterMultiplier*sizeof(int));
  int *arrB = (int*)malloc(clusterSize*params.clusterMultiplier*sizeof(int));
  int a, b, i, newClusterSize;
  newClusterSize = params.clusterMultiplier*clusterSize;

  //Fill up arrA and arrB
  for(a=0, b=newClusterSize*2; a<b; a++)
    ((a/newClusterSize<1)?arrA:arrB)[a%newClusterSize] = numbers[params.initPos+a];

  //Union
  for(i=0, a=0, b=0; a<(newClusterSize) || b<(newClusterSize); i++)
    numbers[params.initPos+i] = (a==newClusterSize)?arrB[b++]:((b==newClusterSize)?arrA[a++]:(arrA[a]>arrB[b]?arrB[b++]:arrA[a++]));

  free(arrA);
  free(arrB);
  free(paramsPointer);
  pthread_exit(0);
}

//Print clusters
int printClusters(int totalnums, int groupc){
  int i;
  printf("\n(");
  for(i=0, groupc=totalnums/groupc; i<(totalnums-1); i++)
    printf("%d%s", numbers[i], (!((i+1)%groupc))?"), (":", ");
  printf("%d)\n\n", numbers[i]);
}