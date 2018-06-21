/* calculating the scalar product of two victors with multiple threads*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>
#include <signal.h>
#include <pthread.h>

int counter=0;
int result=0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_index = PTHREAD_MUTEX_INITIALIZER;
#define SIG_AIO SIGRTMIN+1
// declaring the struct list
struct aiocb * cb[4];              
//the reading buffer 
char * rbuf[4];
int frstready=0,scndready=0; // those are two counters to launch the thread creation once the reading is done 


//the printer thread function
void *print()
{
printf("printer thread\n"); 
    
      pthread_mutex_lock( &lock);
      while(counter<2)
      pthread_cond_wait( &condition_var, &lock);                  
      printf("le resultat est %d\n",result);           
      pthread_mutex_unlock( &lock);
    printf("\n" );             
              
pthread_exit((void *) 0);   
}

// the calculating thread function
void *calcul(void *msg)
{
   int a[200],b[200];
   int tmp=0,i=0,j=0,t=0;
   int *index;  //this variable indicates whether this is the first thread (calculate the first halfs) or the second thread(calculate the second halfs).
  
   index= (int *)msg; printf("index %d\n",*index);
 
 //if index 0 get the first half of a[] from rbuf[0] and the first half of b[] from rbuf[1].
   if (*index==0){

			const char s[2] = ",";
			char *token;
			char *token2;

			token = strtok(rbuf[0], s);
  
			while( token != NULL ) {
   			   		        a[i]=atoi(token);
   				     		i++;
  			    			token = strtok(NULL, s);
   						}

			token2 = strtok(rbuf[1], s);
  
			while( token2 != NULL ) {
      						b[j]=atoi(token2);
     			           		j++;
     						token2 = strtok(NULL, s);
   						}

                 }
 
//if index 1 get the second half of a[] from rbuf[2] and the second half of b[] from rbuf[3].  
else if (*index==1){

			const char s[2] = ",";
			char *token;
			char *token2;

			token = strtok(rbuf[2], s);
  
			while( token != NULL ) {
   			   		        a[i]=atoi(token);
   				     		i++;
  			    			token = strtok(NULL, s);
   						}

			token2 = strtok(rbuf[3], s);
  
			while( token2 != NULL ) {
      						b[j]=atoi(token2);
     			           		j++;
     						token2 = strtok(NULL, s);
   						}

                   }



   for (t=0; t<200; t++)
                       {
                       tmp +=a[t]*b[t];
                       }
  pthread_mutex_lock(&lock);
  result+=tmp;
  counter++;
  if(counter>=2)  pthread_cond_signal(&condition_var); 
  pthread_mutex_unlock(&lock);

pthread_exit((void *) 0);
}

void create_thread(int indx) {
// creating the msg 
int * msg;
msg=malloc(sizeof(int));// cannot pass by ref	
*msg=indx;

pthread_t thread[2],printer;
pthread_attr_t attr;
int rc,tc;
void *status;
pthread_attr_init(&attr); //valeur par défaut
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
 
printf("Creation du thread %d \n",indx);


    rc = pthread_create(&thread[indx], &attr, calcul,(void *)msg); 
  		    if (rc)
     			  {
         		  printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        		  
     			  } 
       if (pthread_create(&printer, &attr, print,NULL))
     			  {
         		  printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        		  exit(-1);
     			  } 


}



//Signal handler called when an AIO operation finishes
void aio_handler(int signal, siginfo_t *info, void*uap)
{


	int cbNumber = info->si_value.sival_int;
	printf("AIO operation %d completed returning %zu\n",cbNumber, aio_return(cb[cbNumber]));
        if (cbNumber==0 || cbNumber ==1)  frstready++;  // each execution of op 0 (read first half a) or op 1 (read first half b) increments frstready by one  
        else if (cbNumber==2 || cbNumber ==3) scndready++;   
       
        if(frstready==2) {create_thread(0); frstready=0;} // when first ready equalls 2 that means that the reading of the first two halfs is done,than creat a thread
        if(scndready==2) {create_thread(1); scndready=0;}
       
}

int main(void)
{
struct sigaction action;

//Create a buffer to store the read data
         rbuf[0] = calloc(1,400);
         rbuf[1] = calloc(1,400);
         rbuf[2] = calloc(1,400);
         rbuf[3] = calloc(1,400);
//Set up the signal handler
	action.sa_sigaction = aio_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIG_AIO, &action, NULL);


// create a file 

 FILE *fp1 = fopen("vec1.txt", "r+");
 FILE *fp2 = fopen("vec2.txt", "r+");


//Allocate space for the aio control blocks
	cb[0] = calloc(1,sizeof(struct aiocb));
	cb[1] = calloc(1,sizeof(struct aiocb));
        cb[2] = calloc(1,sizeof(struct aiocb));
        cb[3] = calloc(1,sizeof(struct aiocb));
//Somewhere to store the result
	cb[0]->aio_buf = rbuf[0];
	cb[1]->aio_buf = rbuf[1];
        cb[2]->aio_buf = rbuf[2];
        cb[3]->aio_buf = rbuf[3];
//The file to read/write from/to
	cb[0]->aio_fildes = fileno(fp1); //read he first half of a 
	cb[1]->aio_fildes = fileno(fp2); // read the first half of b
        cb[2]->aio_fildes = fileno(fp1); //read the second half of a 
        cb[3]->aio_fildes = fileno(fp2); // read the second half of b        

//The number of bytes to read/write, and the offset
	cb[0]->aio_nbytes = 400; // 200 number + 200 ','
	cb[1]->aio_nbytes = 400;
	cb[2]->aio_nbytes = 400;
	cb[3]->aio_nbytes = 400;

	cb[0]->aio_offset = 0;
	cb[1]->aio_offset = 0;
	cb[2]->aio_offset = 400; // start from half a
	cb[3]->aio_offset = 400; // start from half b
//The signal to send, and the value of the signal
	cb[0]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cb[0]->aio_sigevent.sigev_signo = SIG_AIO;
	cb[0]->aio_sigevent.sigev_value.sival_int = 0;

	cb[1]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cb[1]->aio_sigevent.sigev_signo = SIG_AIO;
	cb[1]->aio_sigevent.sigev_value.sival_int = 1;
  
        cb[2]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cb[2]->aio_sigevent.sigev_signo = SIG_AIO;
	cb[2]->aio_sigevent.sigev_value.sival_int = 2;

        cb[3]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cb[3]->aio_sigevent.sigev_signo = SIG_AIO;
	cb[3]->aio_sigevent.sigev_value.sival_int = 3;

        aio_read(cb[0]);
	aio_read(cb[1]);
	aio_read(cb[2]);
	aio_read(cb[3]);

int i;
 for(i=0;i<5;i++) sleep(1);


}
