01 int ncount;
02 
03 void Producer(void* arg)
04 {
05    int i, mydata;
06
07    for(i=0; i<10; i++)
08    {
09       mydata = ncount;	mydata++;	Sleep(5);	ncount = mydata;
10	 printf("Producer => %d\n", ncount);
11    }
12 }
13 void Consumer(void* arg)
14 {
15    int i, mydata;
16
17    for(i=0; i<10; i++)
18    {
19	  mydata = ncount;	mydata++;	Sleep(5);	ncount = mydata;
20       printf("Consumer => %d\n", ncount);
21    }
22 }
23
24 void main()
25 {	
26     HANDLE hThreadVector[2];
27     DWORD ThreadID;
28     int i, mydata;
29 
30     ncount = 0;	
31
32     hThreadVector[0] = CreateThread (NULL, (unsigned)0, 
                                   (LPTHREAD_START_ROUTINE) Producer,
                                   NULL, 0, (LPDWORD)&ThreadID);
33     hThreadVector[1] = CreateThread (NULL, 0, 
                                   (LPTHREAD_START_ROUTINE) Consumer,
                                   NULL, 0, (LPDWORD)&ThreadID);
34     for(i=0; i<10; i++)
35     {
36	   mydata = ncount;	mydata++;	Sleep(5);	ncount = mydata;
37	   printf("Main => %d\n", ncount);
38     }
39     WaitForMultipleObjects(2,hThreadVector,TRUE,INFINITE);
40
41     printf("Final Value => %d\n", ncount);
42 }
01 // race condition 을 Critical Section 을 사용하여 해결 
02 CRITICAL_SECTION	cs;
03 int ncount;
04 
05 void Producer(void* arg)
06 {
07    int i, mydata;
08
09    for(i=0; i<10; i++)
10    {
11       EnterCriticalSection( &cs );
12	    mydata = ncount;   mydata++;   Sleep(5); 	ncount = mydata;
13	    printf("Producer => %d\n", ncount);
14	 LeaveCriticalSection( &cs );
15    }
16 }
17
18 void Consumer(void* arg)
19 {
20    int i, mydata;
21
22    for(i=0; i<10; i++)
23   {
24	 EnterCriticalSection( &cs );
25	    mydata = ncount;   mydata++;   Sleep(5); 	ncount = mydata;
26	    printf("Consumer => %d\n", ncount);
27	LeaveCriticalSection( &cs );
28   }
29 }
30
31 void main()
32 {	
33     HANDLE hThreadVector[2];
34     DWORD ThreadID;
35     int i, mydata;
36
37     InitializeCriticalSection( &cs );
38     ncount = 0;	
39     hThreadVector[0] = CreateThread (NULL, (unsigned)0, 
                                   (LPTHREAD_START_ROUTINE) Producer,
                                   NULL, 0, (LPDWORD)&ThreadID);
40     hThreadVector[1] = CreateThread (NULL, 0, 
                                   (LPTHREAD_START_ROUTINE) Consumer,
                                   NULL, 0, (LPDWORD)&ThreadID);
41     for(i=0; i<10; i++)
42     {
43	   EnterCriticalSection( &cs );
44	      mydata = ncount;   mydata++;   Sleep(5); 	ncount = mydata;
45	      printf("Main => %d\n", ncount);
46	   LeaveCriticalSection( &cs );
47    }
48    WaitForMultipleObjects(2, hThreadVector, TRUE, INFINITE);
49    DeleteCriticalSection( &cs );
50
51    printf("Final Value => %d\n", ncount);
52 }
01 // race condition 을 뮤텍스를 사용하여 해결 
02 HANDLE	hMutex;
03 int ncount;
04
05 void Producer(void* arg)
06 {
07    int i, mydata;
08 
09    for(i=0; i<10; i++)
10    {
11	 WaitForSingleObject( hMutex, INFINITE );
12	    mydata = ncount;   mydata++;   Sleep(5);   ncount = mydata;
13	    printf("Producer => %d\n", ncount);
14	 ReleaseMutex( hMutex );
15    }
16 }
17
18 void Consumer(void* arg)
19 {
20    int i, mydata;
21
22    for(i=0; i<10; i++)
23    {
24       WaitForSingleObject( hMutex, INFINITE );
25	    mydata = ncount;   mydata++;   Sleep(5);   ncount = mydata;
26	    printf("Consumer => %d\n", ncount);
27	 ReleaseMutex( hMutex );
28    }
29 }
30
31 void main()
32 {	
33    HANDLE hThreadVector[2];
34    DWORD ThreadID;
35    int i, mydata;
36
37    hMutex = CreateMutex( NULL, FALSE, "sample_mutex" );
38    ncount = 0;	
39    hThreadVector[0] = CreateThread (NULL, (unsigned)0, 
                                   (LPTHREAD_START_ROUTINE) Producer,
                                   NULL, 0, (LPDWORD)&ThreadID);
40    hThreadVector[1] = CreateThread (NULL, 0, 
                                   (LPTHREAD_START_ROUTINE) Consumer,
                                   NULL, 0, (LPDWORD)&ThreadID);
41    for(i=0; i<10; i++)
42    {
43        WaitForSingleObject( hMutex, INFINITE );
44	     mydata = ncount;   mydata++;   Sleep(5);   ncount = mydata;
45	     printf("Main => %d\n", ncount);
46	  ReleaseMutex( hMutex );
47     }
48    WaitForMultipleObjects(2, hThreadVector, TRUE, INFINITE);
49    CloseHandle( hMutex );
50
51    printf("Final Value => %d\n", ncount);
52 }
01 // race condition 을 세마포를 사용하여 해결 
02 HANDLE	hSemaphore;
03 int ncount;
04 
05 void Producer(void* arg)
06 {
07    int i, mydata;
08
09    for(i=0; i<10; i++)
10    {
11	  WaitForSingleObject( hSemaphore, INFINITE );
12	      mydata = ncount;  mydata++; Sleep(5); 	ncount = mydata;
13	      printf("Producer => %d\n", ncount);
14	  ReleaseSemaphore( hSemaphore, 1, NULL );
15    }
16 }
17 
18 void Consumer(void* arg)
19 {
20    int i, mydata;
21
22    for(i=0; i<10; i++)
23    {
24        WaitForSingleObject( hSemaphore, INFINITE );
25	      mydata = ncount;  mydata++; Sleep(5); 	ncount = mydata;
26	      printf("Consumer => %d\n", ncount);
27	   ReleaseSemaphore( hSemaphore, 1, NULL );
28    }
29 }
30 
31 void main()
32 {	
33     HANDLE hThreadVector[2];
34     DWORD ThreadID;
35     int i, mydata;
36
37     hSemaphore = CreateSemaphore( NULL, 1, 1, "sample_semaphore" );
38     ncount = 0;
39     hThreadVector[0] = CreateThread (NULL, (unsigned)0, 
                                   (LPTHREAD_START_ROUTINE) Producer,
                                   NULL, 0, (LPDWORD)&ThreadID);
40     hThreadVector[1] = CreateThread (NULL, 0, 
                                   (LPTHREAD_START_ROUTINE) Consumer,
                                   NULL, 0, (LPDWORD)&ThreadID);
41     for(i=0; i<10; i++)
42     {
43	   WaitForSingleObject( hSemaphore, INFINITE );
44	      mydata = ncount;  mydata++;  	Sleep(5);   ncount = mydata;
45	      printf("Main => %d\n", ncount);
46	   ReleaseSemaphore( hSemaphore, 1, NULL );
47     }
48    WaitForMultipleObjects(2, hThreadVector, TRUE, INFINITE);
49    CloseHandle( hSemaphore );
50
51    printf("Final Value => %d\n", ncount);
52 }
01 // race condition 을 이벤트를 사용하여 해결 
02 HANDLE	hEvent;
03 int ncount;
04
05 void Producer(void* arg)
06 {
07    int i, mydata;
08
09    for(i=0; i<10; i++)
10    {
11       WaitForSingleObject( hEvent, INFINITE );
12	     mydata = ncount;  mydata++;   Sleep(5);   ncount = mydata;
13	     printf("Producer => %d\n", ncount);
14       SetEvent( hEvent );
15    }
16 }
17
18 void Consumer(void* arg)
19 {
20    int i, mydata;
21
22    for(i=0; i<10; i++)
23    {
24	  WaitForSingleObject( hEvent, INFINITE );
25	     mydata = ncount; 	mydata++;  Sleep(5); 	ncount = mydata;
26	     printf("Consumer => %d\n", ncount);
27 	  SetEvent( hEvent );
28    }
29 }
30
31 void main()
32 {	
33    HANDLE hThreadVector[2];
34    DWORD ThreadID;
35    int i, mydata;
36
37    hEvent = CreateEvent( NULL, FALSE, TRUE, NULL ); // 자동리셋, 초기 TRUE
38    ncount = 0;	
39    hThreadVector[0] = CreateThread (NULL, (unsigned)0, 
                                   (LPTHREAD_START_ROUTINE) Producer,
                                   NULL, 0, (LPDWORD)&ThreadID);
40    hThreadVector[1] = CreateThread (NULL, 0, 
                                   (LPTHREAD_START_ROUTINE) Consumer,
                                   NULL, 0, (LPDWORD)&ThreadID);
41    for(i=0; i<10; i++)
42    {
43	     WaitForSingleObject( hEvent, INFINITE );
44		mydata = ncount;  mydata++; 	Sleep(5);   ncount = mydata;
45	        printf("Main => %d\n", ncount);
46	     SetEvent( hEvent );
47     }
48    WaitForMultipleObjects(2,hThreadVector,TRUE,INFINITE);
49    CloseHandle( hEvent );
50
51    printf("Final Value => %d\n", ncount);
52 }
