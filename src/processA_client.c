#include "./../include/processA_utilities.h"

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
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_RIGHT) {
        	mvprintw(LINES - 1, 1, "Right key button pressed");
                refresh();
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_UP) {
        	mvprintw(LINES - 1, 1, "Up key button pressed");
                refresh();
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
        else if(cmd == KEY_DOWN) {
        	mvprintw(LINES - 1, 1, "Down key button pressed");
                refresh();
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                }
        }
    }
    
    endwin();
    return 0;
}
