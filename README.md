# ARP Third assignment

Installing and running
----------------------------------------------
Before running the program with shell file, it is fundamental to install two libraries:
lncurses and libbitmap.

For installing lncurses, you can run on the terminal the following command:

``` sudo apt-get install libncurses-dev```

For installing libbitmap, you can download the repository from github, by doing:

``` git clone https://github.com/draekko/libbitmap```

Navigate to the root directory of the downloaded repo and run the configuration through command ``` ./configure```. Configuration might take a while. While running, it prints some messages telling which features it is checking for.

Type ``` make``` to compile the package.

Run ``` sudo make install``` to install the programs and any data files and documentation.
Upon completing the installation, check if the files have been properly installed by navigating to /usr/local/lib, where you should find the libbmp.so shared library ready for use.
In order to properly compile programs which use the libbitmap library, you first need to notify the linker about the location of the shared library. To do that, you can simply add the following line at the end of your .bashrc file:

``` export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"```

After doing that, it is the time to create the binary of all 5 processes and run the programm. Before running it, you need to make third.sh executable, by using on the terminal:

``` chmod u+x third.sh```

You can find second.sh into ```add_materials``` folder, but, when launched, this file must be in the same folder of the project. 
Then launch it to create all binary and execute the program, by the command:

```./third.sh```

Once you have done this, the program is successfully launched!

Description of the program ( user guide)
----------------------------------------------

This program simulates a typical vision system, able to track an object in a 2-D plane.
After starting, you can find three different options for execute the program:
* **Option 1**: if you press '1' on keyboard, you are asked to insert the host name, which is the ip of the server ( if you run the program on the same machine enter ```localhost```, otherwise use the ip of the other computer; you can know it by press on terminal ```hostname -I```) and the port number ( typicaly the port number above the 20000 is avaiable, but pay attention to inseet the same value of the server) for allowing the client to connect on that port; this option allows you to execute the program in client mode, that implies you can controll the green cross of another computer, by clicking the one of four arrows and you can make a screenshot of the current position, by click on the print button, which is in the console window. 
**Important remark**: you can launch the program in client mode if and only if there is a server; so someone have had to launch the same program in server mode, otherwise the client program crash istantaneously. In addition the server and client must be connected on the same net.

* **Option 2**: if you press '2' on keyboard, you are asked to insert only the port number, and then it spawns two windows, that are the same of second assignment; this option allows you to execute the program in server mode, that means you cannot control directly the green cross, but someone else, who is connected on the same net, can controll it. In this mode you can simply see the cross moves and, consequently, the '0' and hyphens of other window, which stands for the trajectory of the centre.


* **Option 3**: is the same option of the second assignment. So the program spawns two different konsole windows: one for the camera where a RGB image of the object will be displayed (process A) and one for simulating the extraction of a feature from the image (process B).
More precisely, in the process A window it is possible to do two things:
* moving the image, using the arrow keys on your keyboard;
* pressing the 'P' button, which will make the program take a snaphot of the image and save it on a .bpm file, inside the 'out' folder; if you press several times the P button, you will see many captures in the out folder, named as **snapN.bmp**, where N stands for the number of screenshot.

The movement of the image is not completely free: in fact, there are horizonthal and vertical bounds, which will stop the image from going further (IMPORTANT NOTE: the walls are NOT visible, the image will just stop and won't go any further in that direction).

On the process B window, the center of the image will be shown, which is the extracted feature.
If we move the image on the process A, the center of the image on process B will also move, and leave the image trajectory behind itself.

The program can be terminated in several ways:
* if you are server, you can press ctrl+c on the two **konsole** windows; if you press involuntary ctrl+c on the **terminal** window, do not be worry: the program will not end unless you press, as it has written before, ctrl+c on the two **konsole** windows;
* if you are client, you can press ctrl+c on the only one **konsole** window; if you press involuntary ctrl+c on the **terminal** window, do not be worry: the program will not end unless you press, as it has written before, ctrl+c on the one **konsole** window;
* if you are in the initial menu, any other key button that is pressed let you to exit from the whole program.

Development of the program
----------------------------------------------

The program consists in five processes:

* **Master process** This is the process that spwans and executes, according to the choice of the user, the different processes. In client mode, it creates and executes the process **Client A**; in the server mode it creates **Server A** and executes it and **Process B**, which is quite the same of the second assignment. In the "third" mode it creates, truncates the segment of shared memory and starts everything: after starting its execution, this process spawns two other processes, passing them, as argument, the name of shared memory. When all two children ends their execution ( user presses ctrl+c), it closes the file descriptor, unmap its memory address and then unlink the shared segment memory. 

* **Process A_client** This process shows a grapich interface with a blue print button. It creates a socket, for sending to a server, which can be the same machine if the user wants, the arrow button that has been pressed or the click on 'P' button. It is important to underline that the server must be exist or, in other words, the other machine have had to execute the program in server mode; all arrows and the mouse click, that are  obviously pressed,  corresponds to a integer number, which is  converted into a string and then sent via socket to the server. When you press ctrl+c on the window, a signal is send to close the file descriptor associated to the socket and write on it 'disc' word, for informing the server that a connection is shutdowned.

* **Process A_server** This process shows two grapich interface: one in which you can see a green cross, the other one contains the current position of the centre. The task of server is to listen if someone would to connect, then has to accept it in order to establish a connection via **handshake way** with the client; then it reads from the socket file descriptor and, on the base of the interger that is read, moves the cross or take a snapshot; obviously if no client would to connect to the server, it stays in a waiting state unless someone tries to connect. When you press ctrl+c on the two windows, a signal is send to close the two file descriptors associated to the socket. 

* **Process A** This process shows a grapich interface, where there are a sort of green "cross" and a P button. If the P button is pressed, a screenshot of the image is captured and saved. Process A has the task to register as bmp file, in the shared memory, the current position of the circle in the 80x30 px area. Firstly it creates the image of the circle, with a readius of 30 px and spawns it in the middle of the area, then, every time the user presses any arrow buttons on the keyboard, it creates a new image of the current position of the figure, which replaces the old one. The resolution of image is 1600x600 px, so there is a factor of 20x in the building of it. Actually the process does not share the bmp file in the shared memory, because it is a dynamic structure; there is one more step for doing that: it copies, bit for bit, the whole structure into an array of char and, obviously, shares it. The vector is built in the same way: if it finds colored pixel, sets '1', otherwise '0'. When the user presses ctrl+C on keyboard, this process closes all file descriptor, unlinks semaphores and terminates its own execution.

* **Process B** This process shows a grapich interface, where at the beggining there is a 0 in the middle. It continuosly reads the data from shared memory, converts them into a bitmap file, which size is now 80x30 px, so it resizes the image dividing by a factor of 20x; then it has to find the current center of the circle, and, finally, draws the trajectory of it. For finding the center of the circle, process B scans the whole vector associates to the bitmap structure, which has 960000 elements, and, after finding 59 adjacent colored pixel ( setted as character '1' in the vector), it register the coordinates of the center; if the 59 consecutive pixels are not found, it means that we are close to the upper/lower border and in this case we check where the maximum number of consecutive blue pixels was located, then we have found the center. Trajectory consists in a sort of vertical and horizontal hyphens, and 0 represents where is the current center. Actually the position of the center  in an almost real time, because there is some delay in reading the shared memory and in rebuilding the image. When the user presses ctrl+C on keyboard, this process closes all file descriptor, unlinks semaphores and terminates its own execution.
NOTE: DO NOT RESIZE THIS PROCESS WINDOW, IT WILL IMMEDIATELY CRASH THE PROCESS!
