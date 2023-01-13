#include "./../include/processA_utilities.h"

char btn_pressed[10];

int main(int argc, char *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // print welcome message
    mvprintw(1,(5*COLS)/11, "Waiting for command...");

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
                mvprintw(1, (5*COLS)/11, "Waiting for command...");
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT) {
        	
        }
        else if(cmd == KEY_RIGHT) {
        	
        }
        else if(cmd == KEY_UP) {
        	
        }
        else if(cmd == KEY_DOWN) {
        	
        }
    }
    
    endwin();
    return 0;
}
