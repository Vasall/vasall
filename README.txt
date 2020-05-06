       
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
  creating the window and handeling IO. Additionally OpenGL is used for 
  rendering. Currently the game will most likely only run on Linux as the
  core system hasn't been ported to Windows and macOS yet, but will come in
  the near future.

INSTALLATION

  Installing can be done using the terminal. Note that some of the following
  commands need to be executed as root.
  
  First of all you have to get the basic tools to download and build the game:
    $ apt install git make bash

  Now that the necessary tools have been installed, we now have to get the
  external libraries:
    $ apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev freeglut3-dev libgmp-dev libssl-dev

  After installing the external libraries, we can now clone the reposity:
    $ git clone https://github.com/Vasall/vasall-client.git

  Create the necessary folders for the object-files and the final binary:
    $ mkdir bin obj

  Now that everything is done, we can finally start the client and start
  playing the game using this command:
    $ ./bin/vasall-client
  
