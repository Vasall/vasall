# vasall-client
A coop-hunting-game similar to Monster Hunter. Still in developement.

## How to build the game yourself:

### Debian and Ubuntu
- Install the necessary dependencies: ``sudo apt install git make libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev``
- Clone the repository: ``git clone https://github.com/Vasall/vasall-client``
- Change into the folder: ``cd vasall-client``
- Create the necessary folders: ``mkdir lib``
- Clone the XSDL-repository into the lib-folder: ``cd lib && git clone https://github.com/enudstudios/XSDL.git && cd ..``
- Then start building: ``make libs && bash build.sh``
- Now you can run the code: ``./bin/vasall-client``
