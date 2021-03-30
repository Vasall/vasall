# README

Vasall is a free-to-play MMO-RPG which focuses mainly on combat and 
teamplay. Note that this project is still in the early stages of
developement.

## Technical Description

The whole game is written in C89 and only relies on the SDL-frameworks for
creating the window and handeling IO, aswell as OpenGL and Vulkan for
rendering.  The game will most likely only run on Linux as the core system
hasn't been ported to Windows and macOS yet. A port will come in the near
future.
  
## Notice

This is only the client-application which requires a server to run. For several
reasons the server-application will stay private.
THE SERVER IS CURRENTLY UNAVAILABLE!

## Installation

### Install on Ubuntu and Debian

Installing can be done using the terminal. Note that some of the following
commands need to be executed as root.
  
First of all you have to get the basic tools to download and build the game:<br/>
> $ apt install git make bash subversion

Now that the necessary tools have been installed, we need to get the
external libraries:<br/>
> $ apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev freeglut3-dev  
> $ apt install libgmp-dev libssl-dev libvulkan-dev glslang-tools  

Optional for vulkan debugging:<br/>
> $ apt install vulkan-validationlayers

After installing the external libraries, we can now clone the reposity:<br/>
> $ git clone https://github.com/clusterwerk/vasall.git

Change into the created folder:<br/>
> $ cd vasall

For the automatic installation of a few libraries not in Debian/Ubuntu:<br/>
> $ ./configure

Compile the sourcecode:<br/>
> $ make

Now that everything is done, we can finally start the client and start
playing the game using this command:<br/>
> $ ./bin/client
 
## Contact
   
 - Email: admin@vasall.net
 - Discord: [https://discord.gg/ahnbSfB](https://discord.gg/ahnbSfB)
