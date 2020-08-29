       
                        _ _   _   __   _   _    _   
                       | | | / \ / _| / \ | |  | |  
                       | V || o |\_ \| o || |_ | |_ 
                        \_/ |_n_||__/|_n_||___||___|
 
ABSTRACT

  Vasall is a free-to-play MMO-RPG which focuses mainly on combat and 
  exploration. Note that this project is still in the early stages of
  developement.

TECHNICAL DESCRIPTION

  The whole game is written in C89 and only relies on the SDL-frameworks for
  creating the window and handeling IO, aswell as OpenGL for rendering. 
  Currently the game will most likely only run on Linux as the core system 
  hasn't been ported to Windows and macOS yet. A port will come in the near
  future.
  
NOTICE

  This is only the client-application which requires a server to run. For 
  several reasons the server-application will stay private.
  Furthermore the project is still in developement and service may not always
  available.

INSTALLATION

  >> Install on Ubuntu and Debian

  Installing can be done using the terminal. Note that some of the following
  commands need to be executed as root.
  
  First of all you have to get the basic tools to download and build the game:
    $ apt install git make bash subversion

  Now that the necessary tools have been installed, we now have to get the
  external libraries:
    $ apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev freeglut3-dev
    $ apt install libgmp-dev libssl-dev

  After installing the external libraries, we can now clone the reposity:
    $ git clone https://github.com/Vasall/vasall-client.git

  Change into the created folder:
    $ cd vasall-client

  Create the necessary folders for the object-files and the final binary:
    $ mkdir bin obj lib
    
  Then change the directory and clone the networking-library:
    $ cd lib && git clone https://github.com/Vasall/lcp
   
  Additionally we have to get a module:
    $ cd lcp
    $ svn checkout https://github.com/miniupnp/miniupnp/trunk/miniupnpc

  Build the module using the following command:
    $ cd miniupnpc && make && cd ..
    
  Compile the networking-library:
    $ mkdir obj && make && cd ../..

  Compile the sourcecode:
    $ make

  Now that everything is done, we can finally start the client and start
  playing the game using this command:
    $ ./bin/vasall-client
 
