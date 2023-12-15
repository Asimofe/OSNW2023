// trasfer pointer parameter of struct to thread function
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
   char field1[10];
   char field2[10];
   int  field3;
} PARAMS;

void *Producer(void *arg)
{
   PARAMS *pProducer = (PARAMS *) arg;
   sleep(1);
   printf("Producer => %s %d\n", pProducer->field1, pProducer->field3); 
}
void *Consumer(void *arg)
{
   PARAMS *pConsumer = (PARAMS *) arg;
   sleep(2);
   printf("Consumer => %s %d\n", pConsumer->field2, pConsumer->field3); 
}

pthread_t ThreadVector[2];	 // non-local variables

void main()
{
   PARAMS pSub;	

   strcpy(pSub.field1, "hello");
   strcpy(pSub.field2, "world");
   pSub.field3 = 2023;

   pthread_create(&ThreadVector[0], NULL, Producer, (void *) &pSub);
   pthread_create(&ThreadVector[1], NULL, Consumer, (void *) &pSub);

   pthread_join(ThreadVector[0], NULL);
   pthread_join(ThreadVector[1], NULL);
}
