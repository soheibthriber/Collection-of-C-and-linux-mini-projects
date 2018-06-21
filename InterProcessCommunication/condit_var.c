#include <pthread.h>
#include <stdio.h>
#include  <stdlib.h>
#define NB_THREADS	4
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct shared_memory {
    int a[400];
    int b[400];
    int tmp[4];
    int result;
    int counter;
};

struct shared_memory *shm_ptr;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;



void *print()
{
printf("printer thread\n"); 
    
      pthread_mutex_lock( &lock);
      while( shm_ptr -> counter<4)
      pthread_cond_wait( &condition_var, &lock);                  
      printf("le resultat est %d\n", shm_ptr -> result);           
      pthread_mutex_unlock( &lock);
    printf("\n" );             
              
pthread_exit((void *) 0);   
}


void *calcul(void *msg)
{
   int first,i;
   first= *(int *)msg;
   int tmp_id=first/100; 
   for (i=0; i<100; i++)
                       {
                       shm_ptr -> tmp[tmp_id] += shm_ptr -> a[i+ first ]* shm_ptr ->b[i + first];
                       }
  pthread_mutex_lock(&lock);
   shm_ptr -> result += shm_ptr -> tmp[tmp_id];
   shm_ptr -> counter++;
  if( shm_ptr ->counter>=4)  pthread_cond_signal(&condition_var); 
  pthread_mutex_unlock(&lock);

pthread_exit((void *) 0);
}


int main (int argc, char *argv[])
{
   pthread_t thread[NB_THREADS],printer;
   pthread_attr_t attr;
   int rc, t,i;
   void *status;
   int fd_shm;
   


 if ((fd_shm = shm_open ("/my_shm", O_RDWR | O_CREAT, 0660)) == -1)
         printf("shm_open error");

 if (ftruncate (fd_shm, sizeof (struct shared_memory)) == -1)
       printf("ftruncate error");


  if ((shm_ptr = mmap (NULL, sizeof (struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED,
            fd_shm, 0)) == MAP_FAILED)
       printf("mmap error");
    
// Initialize the shared memory
    shm_ptr -> result  = shm_ptr -> result = 0;
    for(i=0;i<4;i++){
                       shm_ptr -> tmp[i]=0;
   		      }    

    for(i=0;i<400;i++){
                     /*shm_ptr -> a[i]=random()%100; shm_ptr -> b[i]=random()%100;*/ shm_ptr -> a[i]=1; shm_ptr -> b[i]=1;
   		     }


  








   
   

   // Initialisation et activation d’attributs 
   pthread_attr_init(&attr); //valeur par défaut
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); //attente du thread possible

   


	for(t=0; t<NB_THREADS; t++)
	   {
             int msg[4];
             msg[t]=t*100;
	      printf("Creation du thread %d\n", t);
    	      rc = pthread_create(&thread[t], &attr, calcul,(void *)&msg[t]); 
  		    if (rc)
     			  {
         		  printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        		  exit(-1);
     			  }
  
          }

   
   if (pthread_create(&printer, &attr, print,NULL))
     			  {
         		  printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        		  exit(-1);
     			  }

    


   // liberation des attributs et attente de la terminaison des threads 

   pthread_attr_destroy(&attr);
     rc=pthread_join( printer, NULL);
       for(t=0; t<NB_THREADS; t++)
           {
                 rc = pthread_join(thread[t], &status);
                     if (rc)
                           {
                 	    printf("ERROR; le code de retour du pthread_join() est %d\n", rc);
         		    exit(-1);
       			    }
             printf("le join a fini avec le thread %d et a donne le status= %ld\n",t, (long)status);
            }
   
        
       if (munmap (shm_ptr, sizeof (struct shared_memory)) == -1)
        printf (" not closed: munmap error");
        
      shm_unlink("/my_shm");                  

 pthread_exit(NULL);

}


