IMPORTANT: You can only build and run this project on Linux.

/* =-=-=-=-=-=-=-=-=-=-=-=-= */
/* BUILD AND RUN THE PROJECT */
/* =-=-=-=-=-=-=-=-=-=-=-=-= */

Before you can actually compile and run this project, you 
first have to install the required dependencies using the
following commands:

$ sudo apt update && sudo apt upgrade
$ sudo apt install make git pkg-config libsdl2-dev

To get the source-code, you have to clone the 
reposity into a folder, and change into the newly 
created directory by running:

$ git clone https://github.com/Vasall/vasall-client.git
$ cd ./vasall-client

Additionally some other libraries are needed:

$ git clone https://github.com/Vasall/XSDL.git
$ git clone https://github.com/grimfang4/SDL_FontCache.git

After this is done, you're now able to build the 
project yourself using this command:

$ bash ./build.sh

When the project has been build successfully,
you should now be able to run the executable
which is located in the build-directory. So
to run the binary from the terminal:

$ ./build/game
