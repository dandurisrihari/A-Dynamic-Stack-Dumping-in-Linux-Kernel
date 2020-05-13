#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <errno.h>
#include <pthread.h>
#include<sched.h>

#define INSDUMP_SYSCALL 359
#define RMDUMP_SYSCALL 360

#define SYMBOL_NAME_1 "sys_open"
#define SYMBOL_NAME_2 "sys_close"


struct dumpmode_t {
	unsigned int mode;
};
typedef struct dumpmode_t dumpmode_t;


void* thread_fun(void* input);


int main(int argc, char *argv[]) {
	pthread_t my_thread;
	struct dumpmode_t input_1,input_2;
	int dumpstackid_1 = 0,dumpstackid_2=0, fd;
	pid_t pid1;


	//setting dumpstack mode to 0 to test for parent process
	input_1.mode=0;
	//setting dumpstack mode to 1 to test for parent and child process
	input_2.mode=1;

	dumpstackid_1 = syscall(INSDUMP_SYSCALL,SYMBOL_NAME_1, &input_1);
	dumpstackid_2 = syscall(INSDUMP_SYSCALL,SYMBOL_NAME_2, &input_2);

	pid1 = fork();	


	if(pid1 == 0){
		sleep(1);
		printf("IN CHILD PROCESS\n");
		//it doesnt print any dumpstack because fork creates entire new process
		fd = open("/home/", O_RDWR);
		close(fd);
		//if child tries to call rmdump which it didnot create it will result in -ENVIAL
		if(syscall(RMDUMP_SYSCALL,dumpstackid_1)<0){
		    printf("ERROR WHILE REMOVING THE DUMPSTACK\n");		
		}		
	}
	else{
		sleep(3);
		//since dumpstackmode is 0 it can print the dumpstack for parent
		fd = open("/home/", O_RDWR);
		//parent can remove sys_open the insdump probe by calling rmdump
		syscall(RMDUMP_SYSCALL,dumpstackid_1);
		sleep(1);
		//process exiting so it will remove the sys_close probes
		//printf("IN PARENT PROCESS\n");
	}

	pthread_create(&my_thread,NULL, thread_fun, (void *) NULL);
	pthread_join(my_thread,NULL);

	return 0;

}


void* thread_fun(void* input){
     int fd;
     printf("OPENING THE FILE FROM THREAD\n");
     //dumpstack for open will not be printed since mode is 0
     fd = open("/home/", O_RDWR);
     //dumpstack for close will be printed as mode is 1
     close(fd);
     return 0;
}