// 포인터가 가르키는 데이터의 유효성 문제 2 – free heap memory
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

void *Producer(void* arg)
{
   PARAMS *pProducer = (PARAMS *) arg;
   sleep(1);
   printf("Producer => %s %d\n", pProducer->field1, pProducer->field3); 
}

void *Consumer(void* arg)
{
   PARAMS *pConsumer = (PARAMS *) arg;
   sleep(2);
   printf("Consumer => %s %d\n", pConsumer->field2, pConsumer->field3); 
}

void main()
{
   PARAMS *pMain;
   pthread_t ThreadVector[2];

   pMain = (PARAMS *) malloc( sizeof(PARAMS) );
   strcpy(pMain->field1, "hello");
   strcpy(pMain->field2, "world");
   pMain->field3 = 2023;

   pthread_create(&ThreadVector[0], NULL, Producer, (void *) pMain);
   pthread_create(&ThreadVector[1], NULL, Consumer, (void *) pMain);

   //free( pMain );           // no problem ????
   pthread_join(ThreadVector[0], NULL);
   pthread_join(ThreadVector[1], NULL);
   free(pMain);
}   
