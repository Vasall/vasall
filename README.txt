/* =-=-=-=-=-=-=-=-=-=-=-=-= */
/* BUILD AND RUN THE PROJECT */
/* =-=-=-=-=-=-=-=-=-=-=-=-= */

Before you can actually compile and run this project, you 
first have to install the required dependencies using the
following commands:

$ sudo apt update && sudo apt upgrade
$ sudo apt install make pkg-config libsdl2-dev

After this is done, you're now able to build the 
project yourself using this command:

$ bash ./build.sh

When the project has been build successfully,
you should now be able to run the executable
which is located in the build-directory. So
to run the binary from the terminal:

$ ./build/game
