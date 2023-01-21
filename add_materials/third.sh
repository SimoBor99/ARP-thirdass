
#!/bin/sh
mkdir -p bin
gcc src/master.c -o bin/master -lrt -lm
gcc src/processA.c -lncurses -o bin/processA -lbmp -lrt -pthread -lm
gcc src/processB.c -lncurses -o bin/processB -lbmp -lrt -pthread -lm
gcc src/processA_client.c -lncurses -o bin/processA_client -lbmp -lrt -pthread -lm
gcc src/processA_server.c -lncurses -o bin/processA_server -lbmp -lrt -pthread -lm
mkdir -p out
./bin/master
