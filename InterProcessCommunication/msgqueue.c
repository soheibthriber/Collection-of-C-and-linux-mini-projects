/* Pthread with msgque*/
#include <pthread.h>
#include <stdio.h>
#include  <stdlib.h>
# include <unistd.h>
# include <fcntl.h>	
# include <sys/stat.h>	
# include <mqueue.h>
# include <string.h>

#define NB_THREADS	4
# define MSG_LEN 512
# define MQ_NAME "/my_queue"

int a[400],b[400];


void *print()
{
ssize_t len_recv; // the size of the msg
char recv[MSG_LEN]; // to store the string from mg queue 
mqd_t thrqd; //descriptor of queue
int result=0,i=0;   
thrqd = mq_open( MQ_NAME , O_RDONLY); // open the msg queue 

for(i=0;i<4;i++){
                 memset(recv, 0, MSG_LEN); // initialise the recv string  
                 len_recv = mq_receive(thrqd, recv, MSG_LEN, NULL); // get the msg
                 result+= atoi(recv);  // covert the string to integer 
}                   
      printf("le resultat est %d\n",result);

pthread_exit((void *) 0);   
}


void *calcul(void *msg)
{
   int tmp=0;
   int first,i;
   first= *(int *)msg;
   char *send_ptr; // the pointer used to send msg
   mqd_t thrqd; //descriptor of queue
   char message[MSG_LEN];

   thrqd = mq_open( MQ_NAME , O_WRONLY);

   for (i=0; i<100; i++)
                       {
                       tmp +=a[i+ first ]*b[i + first];
                       }

  sprintf(message,"%d", tmp);
 send_ptr = strdup(message);  // returns a ptr to a string 
  mq_send(thrqd, send_ptr, strlen(send_ptr), 0); 
  free(send_ptr); // the memory occcupid by sen_ptr (via strdup()) was obtained by malloc, hence the need to free it. 
   

pthread_exit((void *) 0);
}

int main (int argc, char *argv[])
{
   pthread_t thread[NB_THREADS],printer;
   pthread_attr_t attr;
   int rc, t,i;
   void *status;

	struct mq_attr	mqattr;
  	mqd_t My_MQ;
	

	mqattr.mq_flags = 0;
	mqattr.mq_maxmsg = 5;
	mqattr.mq_msgsize = MSG_LEN;
	mqattr.mq_curmsgs = 0;

//open and create the msg queue  
  My_MQ = mq_open(MQ_NAME, O_RDWR | O_CREAT, 0644, &mqattr);
  if ((int) My_MQ == -1)
  {
    printf("Error : mq_open failed !\n");
    return(-1);
  }





// vectors initialization 
   for(i=0;i<400;i++){
                      
                    /* a[i]=random()%100; b[i]=random()%100;µ*/  a[i]=1; b[i]=1;
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
   
        

                         
  mq_close(My_MQ); 
  mq_unlink(MQ_NAME); 

 printf("\nfinished\n"); 

 pthread_exit(NULL);

}


