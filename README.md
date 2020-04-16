# Magic Trout Imaginarium V2
ESP8266 based controller for talking/singing trout.

This is a fork of a previous project that ran the same idea off a raspberry pi controller. Here we have a perfboard with an mp3 module and the motor controllers all in one.

The board takes 12v, and can be run off pretty much any 12v battery or power source. It draws 1 amp at max load when talking.

The 
# Structure
## Docs
I've included all the PDFs for the components on the board, except for a generic 5v regulator that powers the hbridge and the audio chip.

## Audio/sd_card
The audio files which are included on the SD card on the mp3 player inside the control box. More can be added, which will be randomly chosen when the button is pushed.
Files must be be contained in SD card in specific structure.
The welcome file must be in the root directory, and be the first file coppied
onto the SD card. If you want to replace it, you must delete all other files
first. The files to be randomly chosen at a button push must be in a folder
called "02"
Example below:
/001.mp3 *Welcome file
/02/001.mp3 *Random file
/02/002.mp3 *Random file
/02/003.mp3 *Random file

## lib
External libraries that are used to control the trout, such as timing and mp3 player control. These are automatically pulled in with platform.io

## src
Source code that controls the fish. Inside the main.cpp file you can find adjustable variables to control timings of the fish.