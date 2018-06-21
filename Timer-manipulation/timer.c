/* creating a timer and timer handlar with sigaction 
  changing the schediuling policis and caclculate the jitter time of the timer */
  
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#define SIG SIGRTMIN
#define CLOCKID CLOCK_REALTIME


struct timespec timenow, prevtime;
double contigu_time=0.0;
double times[100],var=0.0,moy=0.0,ectp=0.0; 
int i=0;


void timer_handler ()	
{


if(clock_gettime( CLOCK_REALTIME, &timenow) == -1 ) printf( "clock gettime error" );      
//printf("time is %li s and %li ns \n",timenow.tv_sec,timenow.tv_nsec);

contigu_time = (( timenow.tv_sec - prevtime.tv_sec )*1000.0f) + (( timenow.tv_nsec - prevtime.tv_nsec )/1000000.0f);
if(i>=1 && i<101){
times[(i-1)]=contigu_time;
}
//printf("contiguious time is  %f and i is %d \n",contigu_time,i);



prevtime=timenow;
i++; 
}


void calcul(double data[])
{
    double S = 0.0;

    int j;

    for(j=0; j<100; ++j)
    {
        S+= data[j];
    }

    moy = S/100.0f;

    for(j=0; j<100; ++j)
    var += pow(data[j] - moy, 2);

    ectp=sqrt(var/100.0f);
}



int main()
{
  pid_t mypid=getpid();

char buf[32];
sprintf(buf, "taskset -c -p 2 %d", mypid);
system(buf); 



 int policy;
  char temp[512],w[40],w2[40];
  int policy2;

   struct sched_param sp = { .sched_priority = 50 };
   int rvalue;

           timer_t timerid;
           struct sigevent sevnt;
           struct itimerspec tmrspec;
           struct sigaction sigac;

// setting the schedular to fifo
rvalue = sched_setscheduler(mypid, SCHED_FIFO,&sp);
if (rvalue == -1) {
  printf("setting sched impossible \n");
  return 1;
}

// getting the prio from /proc
	FILE* f=fopen("/proc/self/sched","r");

	while(fgets(temp,500, f)!=NULL){ if(strstr(temp, "prio")!= NULL) break;
			if(strstr(temp, "policy")!= NULL){
				sscanf(temp, "%s %s %d",w,w2, &policy2);
			}
       }

	fclose(f);

   printf("your policy is \"%d\" /proc send best regards \n",policy2);

  
// using the getscheduler function
printf("Scheduler Policy for PID: %d  -> ", mypid);

  policy = sched_getscheduler(mypid);	

  switch(policy) {
    case SCHED_OTHER: printf("SCHED_OTHER\n"); break;
    case SCHED_RR:   printf("SCHED_RR\n"); break;
    case SCHED_FIFO:  printf("SCHED_FIFO\n"); break;
    default:   printf("Unknown...\n");
  }


// defining the conventionnal priority
setpriority(PRIO_PROCESS,mypid, 120);
int prio=getpriority(PRIO_PROCESS,mypid);
if (prio == -1) {
  perror("sched_setscheduler");
  return 1;
}
// get the real time priority via  getparam
int rp = sched_getparam(0, &sp);
printf("your conventional prio is: %d \n",prio);
printf("your real time prio is: %d \n",sp.sched_priority);





// creating the timer 

  sevnt.sigev_notify = SIGEV_SIGNAL;   // on expiration time generate sigev_signo
  sevnt.sigev_signo = SIG;
  sevnt.sigev_value.sival_ptr = &timerid;
  
if (timer_create(CLOCKID, &sevnt, &timerid) == -1)
               printf("Cannot create  timer");

  printf("timer ID is 0x%lx\n", (long) timerid);


           tmrspec.it_value.tv_sec = 0;
           tmrspec.it_value.tv_nsec = 100000000;
           tmrspec.it_interval.tv_sec = tmrspec.it_value.tv_sec;
           tmrspec.it_interval.tv_nsec = tmrspec.it_value.tv_nsec;

 if (timer_settime(timerid, 0, &tmrspec, NULL) == -1)
                printf("timer_settime error");

// assigning the action
memset (&sigac, 0, sizeof (sigac));
sigac.sa_handler = &timer_handler;
sigaction (SIG, &sigac, NULL);
if (sigaction(SIG, &sigac, NULL) == -1)
               printf("sigaction error");


 
while(i<102);

calcul(times);


printf("mean is %f \n",moy);
printf("Standart deviation is  %f \n",ectp);
return 0;
 }
