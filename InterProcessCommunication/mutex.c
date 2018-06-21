/* Pthread with mutex*/
#include <pthread.h>
#include <stdio.h>
#include  <stdlib.h>
#define NB_THREADS	4
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int a[400],b[400],r=0;

void *calcul(void *msg)
{
   int tmp=0;
   int first,i;
   first= *(int *)msg; // receive the first index 
   printf("%d \n",first);

   for (i=0; i<100; i++)
                       {
                       tmp+=a[i+ first ]*b[i + first]; // start from the value of the first index 0 or 100 or 200 or  300.  
                       }
 
  pthread_mutex_lock(&lock);
  r+=tmp;
  pthread_mutex_unlock(&lock);


pthread_exit((void *) 0);
}

int main (int argc, char *argv[])
{
   pthread_t thread[NB_THREADS];
   pthread_attr_t attr;
   int rc, t,i;
   void *status;
   
// vectors initialization 
   for(i=0;i<400;i++){
                      
                     /*a[i]=random()%100; b[i]=random()%100;*/ a[i]=1; b[i]=1;
   		     }
   
   

   // Initialisation et activation d’attributs 
   pthread_attr_init(&attr); //valeur par défaut
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); //attente du thread possible

   


	for(t=0; t<NB_THREADS; t++)
	   {
             int msg[4];
             msg[t]=t*100;// send the first index  for each thread 0 or 100 or 200 or 300
              
	      printf("Creation du thread %d\n", t);
    	      rc = pthread_create(&thread[t], &attr, calcul,(void *)&msg[t]); 
  		    if (rc)
     			  {
         		  printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        		  exit(-1);
     			  }
  
          }





   // liberation des attributs et attente de la terminaison des threads 
   pthread_attr_destroy(&attr);
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
 
                      printf("le resultat est :%d\n",r); // after all thread joined print the result 
                     
                     printf("\nfinished\n"); 

 pthread_exit(NULL);

}


