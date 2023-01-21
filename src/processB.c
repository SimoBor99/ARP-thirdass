#include "./../include/processB_utilities.h"
#include <stdio.h>
#include <bmpfile.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <signal.h>
#define SEM_PATH_1 "/sem_AOS_1"
#define SEM_PATH_2 "/sem_AOS_2"

// we initialize stuff...
int posx;
int posy;
int old_x;
int old_y;
int width = 1600;
int height = 600;
int depth = 4;
const int SIZE = 960000;
int i, fd_shm;
char change = 's';
char * ptr;

// initialization of semaphores
sem_t * sem_id2;
sem_t * sem_id1;

// we declare bmp file, bmp_vec and pixel
bmpfile_t *bmp;
rgb_pixel_t pixelb = {255, 0, 0, 0};
rgb_pixel_t pixelw = {0, 0, 0, 0};
char bmp_vec[960000];

// function to initialize the bmp file
void init_bmp() {
	// blue pixel
  	rgb_pixel_t pixelb = {255, 0, 0, 0};	
  	// white pixel
  	rgb_pixel_t pixelw = {0, 0, 0, 0};	
  	int countz = 0;
  
  	char num[20];
  	// the bmp file is created
	bmp = bmp_create(width, height, depth);		
  	int radius = 30;
  	// every pixel is set to white
	for(int a = 0; a < width; a++) {		
		for (int b = 0; b < height; b++) {
			bmp_set_pixel(bmp, a, b, pixelw);
		}
	}
	// every pixel inside the circle is changed from white to blue
	for(int x = -radius; x <= radius; x++) {	
    		for(int y = -radius; y <= radius; y++) {
      		// If distance is smaller, point is within the circle
      			if(sqrt(x*x + y*y) < radius) {
        	  		bmp_set_pixel(bmp, width/2 + x, height/2 + y, pixelb);
      			}
    		}
  	}
}

// function for building the bmp file of process B
void build_bmp() {
	// initializing what is needed...
	old_x = posx;
	old_y = posy;
	int count = 0;
	// this is the number of pixels of a circle of radius equal to 30 pixels
	int count_blue = 0;
	int n_pixel_blue = 2808;
	int x_max = 0;
	int y_max = 0;
	int not_full_vert = 0;
	int consecutive = 0;
	int max_consecutive = 0;			
	// we set the pixels of the new bmp file
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++) {
			// if the entry is '1', the correspondent pixel is blue
			if (bmp_vec[count] == '1') {	
				bmp_set_pixel(bmp, i, j, pixelb);
				count_blue += 1;
				consecutive += 1;
				if (max_consecutive < consecutive) {
					max_consecutive = consecutive;
					x_max = i;
					y_max = j;
				}
			}
			// if the entry is not '1', the correspondent pixel is white
			else {				
				bmp_set_pixel(bmp, i, j, pixelw);
				consecutive = 0;
			}
			count += 1;
			// if we find a certain amount of vertical consecutive blue pixels, we found the center!
			if (consecutive == 59) {
				posx = i;			
				posx /= 20;
				posx = round(posx);
				posy = j - 30;
				posy /= 20;
				posy = round(posy);
				not_full_vert = 1;
			}
		}
	}
	// if we did not manage to find enough vertical blue pixels, we check the horizonthal ones
	if (not_full_vert == 0) {
		posx = x_max + 10;			
		posx /= 20;
		posx = round(posx);
		if (max_consecutive > 31) {
			posy = y_max - 30;
		}
		// this double case is done to prevent some errors
		else {
			posy = y_max - 59;
		}
		posy /= 20;
		posy = round(posy);
	}
	// we check what position changed (horizonthal or vertical) and how (increased or decreased) and we set change to a char that identify how we moved
	if (old_x < posx) {					
		change = 'r';					
	}
	else if (old_x > posx) {
		change = 'l';
	}
	else if (old_y < posy) {
		change = 'u';
	}
	else if (old_y > posy) {
		change = 'd';
	}
}

// function for reading from shared memory
void read_shm() {
	// we copy into the bmp vector the data in the shared memory
	strcpy(bmp_vec, ptr);		
	// we build a new bmp file	
	build_bmp();					
}

// handler for terminating everything
void kill_handler (int signum) {

	// check the number of signal
	if (signum==SIGINT) {
		if (close(fd_shm)==-1) {
    	perror("Cannot close the the file descriptor");
  	}
		
		// close file descriptor of semaphore
		if (sem_close(sem_id1)==-1) {
			if (sem_unlink(SEM_PATH_1)==-1) {
				perror("Cannot unlink sem_id1");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_FAILURE);
		}

		// close file descriptor of semaphore
		if (sem_close(sem_id2)==-1) {
			if (sem_unlink(SEM_PATH_2)==-1) {
				perror("Cannot unlink sem_id2");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_FAILURE);
		}

		// unlink segment memory of semaphore
		if (sem_unlink(SEM_PATH_1)==-1) {
			perror("Cannot unlink sem_id1");
			exit(EXIT_FAILURE);
		}

		// unlink segment memory of semaphore
		if (sem_unlink(SEM_PATH_2)==-1) {
			perror("Cannot unlink sem_id2");
			exit(EXIT_FAILURE);
		}

  		//remove the mapping
  		if (munmap(ptr, sizeof(char))==-1) {
    		perror("Cannot unmapp the address");
    		exit(EXIT_FAILURE);
  		}
	}
	exit(1);
}

// main function
int main(int argc, char const *argv[]) {	

	int first_move = 0;

	if ( argc!=2) {
		perror("Not enough arguments!");
		exit(EXIT_FAILURE);
	}
	const char * shm_name = argv[1];
	
    // Utility variable to avoid trigger resize event on launch
    	int first_resize = TRUE;

    // Initialize UI
    	init_console_ui();

	posx = COLS/2;
    	posy = LINES/2;
	mvaddch(LINES/2, COLS/2, '0');
    	refresh();

	sleep(1);

	// we open the shared memory
	fd_shm = shm_open(shm_name, O_RDONLY, 0666);
	if (fd_shm == -1) {
		printf("Shared memory segment failed\n");
		return fd_shm;
	}

	// we map the shared memory
	ptr = (char*) mmap(0, SIZE, PROT_READ, MAP_SHARED, fd_shm, 0);
	
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	// we initialize semaphores
	sem_id1=sem_open(SEM_PATH_1, 0);
	
	if ( sem_id1==SEM_FAILED) {
		perror("semaphore id1 cannot be opened");
		exit(EXIT_FAILURE);
	}
	
	sem_id2=sem_open(SEM_PATH_2, 0);
	if ( sem_id2==SEM_FAILED) {
		perror("semaphore id2 cannot be opened");
		exit(EXIT_FAILURE);
	}

	// signal for unlink anything of process B
	struct sigaction s_kill;
	memset(&s_kill, 0, sizeof(s_kill));
	s_kill.sa_handler=&kill_handler;
	
	// catch signal handler
	if (sigaction(SIGINT, &s_kill, 0)==-1) {
    		perror("Can't catch the signal");
	}

	// we initialize the bmp file
	init_bmp();
	
	// we set the center position
	posx = 40;
	posy = 14;
    
	// Infinite loop
    	while (TRUE) {

        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        	if(cmd == KEY_RESIZE) {
            		if(first_resize) {
                		first_resize = FALSE;
            		}
            		else {
            		    	reset_console_ui();
            		}
        	}
		
		// we costantly read from the shared memory, in order to get our current position
        	else {
        		if (sem_wait(sem_id2)==-1) {
				perror("It is not possible execute wait");
				if (sem_close(sem_id2)==-1) {
					if (sem_unlink(SEM_PATH_2)==-1) {
						perror("Cannot unlink sem_id2");
						exit(EXIT_FAILURE);
					}
					exit(EXIT_FAILURE);
				}
				if (sem_unlink(SEM_PATH_2)==-1) {
					perror("Cannot unlink sem_id2");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_FAILURE);
			}

        	        read_shm();
	
			if (sem_post(sem_id1)==-1) {
				perror("It is not possible execute post");
				if (sem_close(sem_id1)==-1) {
					if (sem_unlink(SEM_PATH_1)==-1) {
						perror("Cannot unlink sem_id1");
						exit(EXIT_FAILURE);
					}
					exit(EXIT_FAILURE);
				}
				if (sem_unlink(SEM_PATH_1)==-1) {
					perror("Cannot unlink sem_id1");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_FAILURE);
			}
        	}
        		
        	// if change is no longer equal to 's', we move to our new position
        	if (change == 'r') {
        	// we moved to the right
			if (first_move == 0) {
				first_move = 1;
				mvaddch(LINES/2, COLS/2, '-');
			}
			// we mark our trajectory
        		mvaddch(LINES/2 + old_y - 14, COLS/2 + old_x - 40, '-');
        		refresh();
        		change = 's';
        	}
	
        	else if (change == 'l') {
        	// we moved to the right
			if (first_move == 0) {
				first_move = 1;
				mvaddch(LINES/2, COLS/2, '-');
			}
			// we mark our trajectory
        		mvaddch(LINES/2 + old_y - 14, COLS/2 + old_x - 40, '-');
        		refresh();
        		change = 's';
        	}
	
        	else if (change == 'u') {
        	// we moved upwards
			if (first_move == 0) {
				first_move = 1;
				mvaddch(LINES/2, COLS/2, '|');
			}
			// we mark our trajectory
        		mvaddch(LINES/2 + old_y - 14, COLS/2 + old_x - 40, '|');
        		refresh();
        		change = 's';
        	}
        	
			else if (change == 'd') {
			// we moved downwards
			if (first_move == 0) {
				first_move = 1;
				mvaddch(LINES/2, COLS/2, '|');
			}
			// we mark our trajectory
        		mvaddch(LINES/2 + old_y - 14, COLS/2 + old_x - 40, '|');
        		refresh();
        		change = 's';
        		}
        	// we set our new center position
		mvaddch(LINES/2 + posy - 14, COLS/2 + posx - 40, '0');        	
    	}
	endwin();
    	return 0;
}
