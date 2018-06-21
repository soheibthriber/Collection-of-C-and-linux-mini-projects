/* reading a file asynchrounously */
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

#define SIG_AIO SIGRTMIN+1
// declaring the struct list
struct aiocb * cb[2]; 
// creating the reading buffers 
char * rbuf[2];
// the wrinting buffer

char wbuf[]={"this is a neumeratical palindrome \"if such thing exist\"\n" };


//Signal handler called when an AIO operation finishes
void aio_handler(int signal, siginfo_t *info, void*uap)
{
	int cbNumber = info->si_value.sival_int;
	printf("AIO operation %d completed returning %zu\n",cbNumber, aio_return(cb[cbNumber])); // print the return value coressponding to tha aio opperation s 
      
       if (cbNumber<2) printf("%s\n",rbuf[cbNumber]); //printf the read buffer 
       
}

int main(void)
{
struct sigaction action;

//Create a buffer to store the read data
         rbuf[0] = calloc(1,10);
         rbuf[1] = calloc(1,10);
//Set up the signal handler
	action.sa_sigaction = aio_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIG_AIO, &action, NULL);


// create a file 

 FILE *fp = fopen("file1.txt", "a+");



//Allocate space for the aio control blocks
	cb[0] = calloc(1,sizeof(struct aiocb));
	cb[1] = calloc(1,sizeof(struct aiocb));
        cb[2] = calloc(1,sizeof(struct aiocb));
//Somewhere to store the result
	cb[0]->aio_buf = rbuf[0];
	cb[1]->aio_buf = rbuf[1];
        cb[2]->aio_buf = wbuf;
//The file to read/write from/to
	cb[0]->aio_fildes = fileno(fp);
	cb[1]->aio_fildes = fileno(fp);
        cb[2]->aio_fildes = fileno(fp); //writing to the same file

// get the write string lenght 
int WBUF_SIZE=strlen(wbuf);
//The number of bytes to read/write, and the offset
	cb[0]->aio_nbytes = 10;
	cb[1]->aio_nbytes = 10;
        cb[2]->aio_nbytes = WBUF_SIZE;

	cb[0]->aio_offset = 0;
	cb[1]->aio_offset = 10;
        cb[2]->aio_offset = O_APPEND;
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

        aio_read(cb[0]);
	aio_read(cb[1]);
        aio_write(cb[2]);

int i;
 for(i=0;i<5;i++) sleep(1);

fclose(fp);
}
