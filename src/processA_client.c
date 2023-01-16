#include "./../include/processA_utilities.h"
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>

int sockfd;

void kill_handler (int signum) {
    if (close(sockfd)==-1)  {
        perror("Cannot close the file descriptor");
        exit(EXIT_FAILURE);
    }
    exit(1);
}


int main(int argc, char *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();
    
    // make the circle disappear
     move_circle(KEY_UP);

    // print welcome message
    mvprintw(1,COLS/10, "Welcome! Move the cross with the arrow keys or take a snaphot of the circle!");

    // stuff for socket
    int port_num=atoi(argv[2]);
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int n;

    // command input
    char command[20];
    // create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    server=gethostbyname(argv[1]);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    if (argc!=2) {
        perror("No such arguments");
        exit(EXIT_FAILURE);
    }

    // set all values in a buffer to zero
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);

     // assign the port number to the struct
    serv_addr.sin_port = htons(port_num);

    // establish a connection to the server
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }
    bzero(command,20);

    // signal for closing file descriptor of process A
	struct sigaction s_kill;
	memset(&s_kill, 0, sizeof(s_kill));
	s_kill.sa_handler=&kill_handler;
	
	// catch signal handler
	if (sigaction(SIGINT, &s_kill, 0)==-1) {
    		perror("Can't catch the signal");
            exit(EXIT_FAILURE);
	}
    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
                mvprintw(1, COLS/5, "Welcome! Move the cross with the arrow keys or take a snaphot of the circle!");
                move_circle(KEY_UP);
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    strcpy(command, "p");
                    n = write(sockfd,command,strlen(command));
                    if (n < 0) {
                        perror("ERROR writing to socket");
                        exit(EXIT_FAILURE);
                    }
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT) {
        	mvprintw(LINES - 1, 1, "Left key button pressed");
                refresh();
                strcpy(command, "l");
                n = write(sockfd,command,strlen(command));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    exit(EXIT_FAILURE);
                }
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_RIGHT) {
        	mvprintw(LINES - 1, 1, "Right key button pressed");
                refresh();
                strcpy(command, "r");
                n = write(sockfd,command,strlen(command));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    exit(EXIT_FAILURE);
                }
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_UP) {
        	mvprintw(LINES - 1, 1, "Up key button pressed");
                refresh();
                strcpy(command, "u");
                n = write(sockfd,command,strlen(command));
                 if (n < 0) {
                    perror("ERROR writing to socket");
                    exit(EXIT_FAILURE);
                }
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_DOWN) {
        	mvprintw(LINES - 1, 1, "Down key button pressed");
                refresh();
                strcpy(command, "d");
                n = write(sockfd,command,strlen(command));
                 if (n < 0) {
                    perror("ERROR writing to socket");
                    exit(EXIT_FAILURE);
                }
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
    }
    
    endwin();
    return 0;
}

