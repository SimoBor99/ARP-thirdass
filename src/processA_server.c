#include "./../include/processA_utilities.h"
#include <stdio.h>
#include <bmpfile.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <signal.h>
#include <semaphore.h>
#define SEM_PATH_1 "/sem_AOS_1"
#define SEM_PATH_2 "/sem_AOS_2"

void write_log(char * log_text, const char * fn) {
	
	FILE *fp_log;
	fp_log = fopen(fn,"w"); 
	if (fp_log==NULL) {
		perror("Cannot read the file");
	}
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	fclose(fp_log);
}


// we initialize stuff...
int n_snapshot = 0;
int posx = 0;
int posy = 0;
int width = 1600;
int height = 600;
int depth = 4;
const int SIZE = 960000;
int i, shm_fd;
char * ptr;
int sockfd;
int newsockfd;

// initialization of semaphores
sem_t * sem_id1;
sem_t * sem_id2;

// we declare bmp file, bmp_vec and pixel
bmpfile_t *bmp;
rgb_pixel_t pixel = {255, 0, 0, 0};
rgb_pixel_t * pixelv;
char bmp_vec[960000];

// function to be executed when the 'P' button is pressed
void print_circle(int px, int py) {
	// we initialize what is needed...
	px *= 20;
	py *= 20;
	bmpfile_t *bmpp;
	// blue pixel
  	rgb_pixel_t pixelp = {255, 0, 0, 0};
  	// white pixel
  	rgb_pixel_t pixelw = {0, 0, 0, 0};
  	// path for location where snapshot will be saved
  	char path[100] = "out/snap";
  	char num[20];
  	// we create a new bmp file
	bmpp = bmp_create(width+60, height+60, depth);
  	int radius = 30;
  	// we set every pixel inside the circle radius to blue
	for(int x = -radius; x <= radius; x++) {
		for(int y = -radius; y <= radius; y++) {
	      		if(sqrt(x*x + y*y) < radius) {
          			bmp_set_pixel(bmpp, (width/2) + 30 + x + px, (height/2) + 30 + y - py, pixelp);
      			}
				else {
      				bmp_set_pixel(bmp, (width/2) +30 + x, (height/2) + 30 + y, pixelw);
      			}
    		}
  	}
  	sprintf(num,"%d",n_snapshot);
  	strcat(path,num);
  	strcat(path,".bmp");
  	// we save the bmp file into the out folder
  	bmp_save(bmpp, path);
  	n_snapshot += 1;
  	// we destroy the bmp file
  	bmp_destroy(bmpp);
}

// function to transform the bmp file into a vector to be loaded into the shared memory
void vectorize() {
	int val;
	int count = 0;
	// we check the color of every pixel
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++) {
			// we get the pixel and check the color blue value in his rgba values
			pixelv = bmp_get_pixel(bmp,i,j);
			val = pixelv->blue;
			// if blue value is 255, the pixel is part of the circle
			if (val == 255) {
				bmp_vec[count]='1';
			}
			// if not, the pixel is outside the circle
			else {
				bmp_vec[count]='0';
			}
			count += 1;
		}
	}	
}

// function to initialize the bmp file
void init_bmp() {
	// blue pixel
  	rgb_pixel_t pixelb = {255, 0, 0, 0};
  	// white pixel
  	rgb_pixel_t pixelw = {0, 0, 0, 0};
  	int countz = 0;
  	char num[20];
  	// we create a bmp file
	bmp = bmp_create(width, height, depth);
  	int radius = 30;
  	// we set every pixel to white
	for(int a = 0; a < width; a++) {
		for (int b = 0; b < height; b++) {
			bmp_set_pixel(bmp, a, b, pixelw);
		}
	}
	// we change every pixel inside the circle from white to blue
	for(int x = -radius; x <= radius; x++) {
    		for(int y = -radius; y <= radius; y++) {
      			// If distance is smaller, point is within the circle
      			if(sqrt(x*x + y*y) < radius) {
          			bmp_set_pixel(bmp, width/2 + x, height/2 + y, pixelb);
      			}
    		}
  	}
}

void write_shm (int shm_fd) {
	// write into memory segment
	for (int i=0; i<SIZE; i++) {
		ptr[i] = bmp_vec[i];
	}
	
}
// function to modify the bmp file if the circle has been moved
void move_bmp(int posx, int posy) {
	// initializing what is needed
	posx *= 20;
	posy *= 20;
	// blue pixel
	rgb_pixel_t pixelb = {255, 0, 0, 0};
	// white pixel
  	rgb_pixel_t pixelw = {0, 0, 0, 0};
  	int countz = 0;
  	char num[20];
  	int radius = 30;
  	// we set every pixel to white
	for(int a = 0; a < width; a++) {
		for (int b = 0; b < height; b++) {
			bmp_set_pixel(bmp, a, b, pixelw);
		}
	}
	// we change every pixel inside the circle from white to blue
	for(int x = -radius; x <= radius; x++) {
    		for(int y = -radius; y <= radius; y++) {
      			// If distance is smaller, point is within the circle
      			if(sqrt(x*x + y*y) < radius) {
          			bmp_set_pixel(bmp, width/2 + x + posx, height/2 + y - posy, pixelb);
      			}
    		}
  	}
}

// handler for terminating everything
void kill_handler (int signum) {
	
	// check the number of signal
	if (signum==SIGINT) {
		if (close(shm_fd)==-1) {
    		perror("Cannot close the the file descriptor");
			exit(EXIT_FAILURE);
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
		if (close(sockfd)==-1) {
			perror("Cannot close file descriptor");
			exit(EXIT_FAILURE);
		}

		if (close(sockfd)==-1) {
			perror("Cannot close file descriptor");
			exit(EXIT_FAILURE);
		}
	}
	exit(1);
}

// main function
int main(int argc, char *argv[]) {	

    if (argc!=3) {
		perror("Not enough arguments!");
		exit(EXIT_FAILURE);
    }
    const char * shm_name=argv[1];
    
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
 
    // shared memory fd
    shm_fd=shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd==-1) {
    	printf("Shared memory segment failed\n");
    	return shm_fd;
    }

	// map the address process
	ptr=(char*)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	
	if (ptr==MAP_FAILED) {
		printf("Map failed\n");
		exit(EXIT_FAILURE);
	}

    // Initialize UI
    init_console_ui();
    
    // print welcome message
    mvprintw(1,(5*COLS)/11, "Waiting for command...");
    
    // we initialize the bmp file of process A
    init_bmp();
    vectorize();
    ftruncate(shm_fd, SIZE);
    write_shm(shm_fd);
	// signal for unlink anything of process A
    struct sigaction s_kill;
	memset(&s_kill, 0, sizeof(s_kill));
	s_kill.sa_handler=&kill_handler;
	
	// catch signal handler
	if (sigaction(SIGINT, &s_kill, 0)==-1) {
    	perror("Can't catch the signal");
	}
	
	// we open first semaphore
	sem_id1=sem_open(SEM_PATH_1, O_CREAT, S_IRUSR | S_IWUSR, 1);
	if ( sem_id1==SEM_FAILED) {
		perror("semaphore id1 cannot be opened");
		if (sem_unlink(SEM_PATH_1)==-1) {
			perror("Cannot unlink sem_id1");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
	}

	// we open second semaphore
	sem_id2=sem_open(SEM_PATH_2, O_CREAT, S_IRUSR | S_IWUSR, 1);
	if ( sem_id2==SEM_FAILED) {
		perror("semaphore id2 cannot be opened");
		if (sem_unlink(SEM_PATH_2)==-1) {
			perror("Cannot unlink sem_id2");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
	}

	//initialized to 1 the first semaphore
	if (sem_init(sem_id1, 1, 1)==-1) {
		perror("Cannot initialize sem_id1");
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

	//initialized to 0 the second semaphore 
	if (sem_init(sem_id2, 1, 0)==-1) {
		perror("Cannot initialize sem_id2");
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

    int port_num, clilen, n;
    char input[2];
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
     }
     port_num=atoi(argv[2]);

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port_num);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
              perror("ERROR on binding");
              exit(EXIT_FAILURE);
     }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
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
                mvprintw(1, (5*COLS)/11, "Waiting for command...");
            }
        }
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
          perror("ERROR on accept");
          exit(EXIT_FAILURE);
        }
		bzero(input, sizeof(input));
        n = read(newsockfd,input,2);
		write_log("tt\n", "oo.txt");
        if (n < 0)  {
            perror("ERROR reading from socket");
            exit(EXIT_FAILURE);
        }
		// it arrives one command from the client

		if(strcmp(input, "l")==0 || strcmp(input, "r")==0|| strcmp(input, "u")==0 || strcmp(input, "d")==0 || (strcmp(input, "p")==0)) {

			if (strcmp(input, "l")==0) {
            	write_log(input, "server.txt");
				// horizonthal position is decremented
            	// we check if we are inside the limited area where we can move
            	if (posx > -40) {
            		posx -= 1;
            		move_circle(cmd);
            		draw_circle();
            		// we create new bmp with circle moved
            		move_bmp(posx, posy);
            		// we prepare the data to be written into the shared memory
            		vectorize();
				
					// decrement the counter of first semaphore
					if (sem_wait(sem_id1)==-1) {
						perror("It is not possible execute wait");
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
					// critic region: process writes on shared memory
            		write_shm(shm_fd);
					// increment the counter of second semaphore
					if (sem_post(sem_id2)==-1) {
						perror("It is not possible execute post");
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
          			usleep(20000);
            	}
			}

            else if (strcmp(input, "r")==0) {
            		write_log(input, "server.txt");
				// horizonthal position is incremented
            	// we check if we are inside the limited area where we can move
            	if (posx < 39) {
            		posx += 1;
            		move_circle(cmd);
            		draw_circle();
            		// we create new bmp with circle moved
            		move_bmp(posx, posy);
            		// we prepare the data to be written into the shared memory
            		vectorize();
					// decrement the counter of first semaphore
					if (sem_wait(sem_id1)==-1) {
						perror("It is not possible execute wait");
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
					
					// critic region: process writes on shared memory
            		write_shm(shm_fd);
					write_log(input, "server.txt");
					// increment the counter of second semaphore
					if (sem_post(sem_id2)==-1) {
						perror("It is not possible execute post");
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
            		usleep(20000);
            	}
			}

			else if (strcmp(input, "d")==0) {
            		write_log(input, "server.txt");
				// vertical position is decremented
            	// we check if we are inside the limited area where we can move
            	if (posy > -14) {
            		posy -= 1;
            		move_circle(cmd);
            		draw_circle();
            		
					// we create new bmp with circle moved
            		move_bmp(posx, posy);
            		
					// we prepare the data to be written into the shared memory
            		vectorize();
					
					// decrement the counter of first semaphore
					if (sem_wait(sem_id1)==-1) {
						perror("It is not possible execute wait");
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
					
					// critic region: process writes on shared memory
            		write_shm(shm_fd);
					
					// increment the counter of second semaphore
					if (sem_post(sem_id2)==-1) {
						perror("It is not possible execute post");
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
            		usleep(20000);
            	}	
			}
            
			else if (strcmp(input, "u")==0) {
				write_log(input, "server.txt");
            	// vertical position is incremented
            	// we check if we are inside the limited area where we can move
            	if (posy < 15) {
            		posy += 1;
            		move_circle(cmd);
            		draw_circle();
            		
					// we create new bmp with circle moved
            		move_bmp(posx, posy);
            		
					// we prepare the data to be written into the shared memory
            		vectorize();
					
					// decrement the counter of first semaphore
					if (sem_wait(sem_id1)==-1) {
						perror("It is not possible execute wait");
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
					
					// critic region: process writes on shared memory
            		write_shm(shm_fd);
					
					// increment the counter of second semaphore
					if (sem_post(sem_id2)==-1) {
						perror("It is not possible execute post");
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
            		usleep(20000);
            	}
            }
        }        
    	usleep(20000);
    }
    endwin();
    return 0;
}
