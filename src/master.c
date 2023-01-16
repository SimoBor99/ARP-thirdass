#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>

// declaration pids of two proccesses
pid_t pid_procA, pid_procB;

// stuff for shared memory
char* ptr;
int shm_fd;
char * shm_name="/AOS";
int mode;

// size of shared memory
const int dim_mem=1600*600;

// create two processes
int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0) {
    perror("Exec failed");
    return 1;
    }
  }
}


int main() {
 	
  	// menù where user decides in which mode they want to run the program
  	char decision[20];
	char host[80];
	char port_number[20];
  	printf("%s\n","Welcome to this program! :)");
  	printf("%s\n","Press 1 in order to run this program in client mode;");
  	printf("%s\n","Press 2 in order to run this program in server mode;");
  	printf("%s\n","Press 3 in order to run this program in local mode;");
  	printf("%s\n","press any other key button to exit from the program.");
  	printf("%s","> ");
  	fgets(decision,sizeof(decision),stdin);
  	if (decision[0] == '1' && strlen(decision) == 2) {
		printf("Insert host and port number");
		if (fgets(host, sizeof(host), stdin)==0)
			perror("No input");
		if (fgets(port_number, sizeof(port_number), stdin)==0)
			perror("No input");
  		// this prevents to close the program on terminal window
  	 	struct sigaction s_allert;
  	 	memset(&s_allert, 0, sizeof(s_allert));
  	 	s_allert.sa_handler=SIG_IGN;
		
  	 	// catch signal handler
  	 	if (sigaction(SIGINT, &s_allert, 0)==-1) {
  	    		perror("Can't catch the signal");
  	 	}
  	 	
  	 	// define list of arguments of processes
  		char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA_client", host, port_number, NULL };
  		
  		pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  		
  		int status;
  		
  		// wait that procA ends
  		if (waitpid(pid_procA, &status, 0)==-1) {
  	  		perror("Error in waitpid 1!");
  	  		return -1;
  		}
  		
  		printf ("Main program exiting with status %d\n", status);
  		return 0;
  	}
  	else if (decision[0] == '2' && strlen(decision) == 2) {
  		printf("%s","S");
  		exit(1);
  	}
  	else if (decision[0] == '3' && strlen(decision) == 2) {
  		// create segment of shared memory
  		shm_fd=shm_open(shm_name, O_CREAT | O_RDWR, 0666);
  	
  		// check if the segment memory is valid
  		if (shm_fd==-1) {
    			printf("Shared memory segment failed\n");
    			return shm_fd;
  		}
	
  		// truncate to the correct size
  		if (ftruncate(shm_fd, dim_mem)==-1) {
    			perror("Cannot truncate!");
    			return shm_fd;
  		}
  	
  		// map the address process
  		ptr= (char*) mmap(0, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	
  		// check if the mapping has done
  		if (ptr == MAP_FAILED) {
    			printf("Map failed\n");
    			exit(EXIT_FAILURE);
  		}
  	
   		// this prevents to close the program on terminal window
  	 	struct sigaction s_allert;
  	 	memset(&s_allert, 0, sizeof(s_allert));
  	 	s_allert.sa_handler=SIG_IGN;
		
  	 	// catch signal handler
  	 	if (sigaction(SIGINT, &s_allert, 0)==-1) {
  	    		perror("Can't catch the signal");
  	 	}
  	
  		// define list of arguments of processes
  		char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", shm_name, NULL };
  		char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", shm_name, NULL };
	
  		pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  		pid_procB = spawn("/usr/bin/konsole", arg_list_B);
  	
  		int status;
  	
  		// wait that procA ends
  		if ( waitpid(pid_procA, &status, 0)==-1) {
  	  		perror("Error in waitpid 1!");
  	  		return -1;
  		}
  		usleep(20000);
	
  		// wait that procA ends
  		if (waitpid(pid_procB, &status, 0)==-1) {
  	  		perror("Error in waitpid 2!");
  	  		return -1;
  		}
	
  		// close segment of shared memory
  	 	if (close(shm_fd)==-1) {
  	  		perror("Cannot close the the file descriptor");
  	  		exit(EXIT_FAILURE);
  		}
	
  		//remove the mapping
  		if (munmap(ptr, sizeof(char))==-1) {
  	  		perror("Cannot unmapp the address");
  	  		exit(EXIT_FAILURE);
  		}
  	
  		// unlinking the shared memory
  		if (shm_unlink(shm_name)==-1) {
  		  	printf("Error removing %s\n", shm_name);
  		  	exit(EXIT_FAILURE);
  		}
  	
  		printf ("Main program exiting with status %d\n", status);
  		return 0;
  	}
  	else {
  		int status;
  		printf ("Main program exiting with status %d\n", status);
  		return 0;
  	}
}

