# Magic Trout Imaginarium V2

Rewrite of the Magic Trout control software to be run on an ESP8266 microcontroller. Could easily be adapted to run on other microcontrollers.

# Cases
Parts on the back are 3d printed. Parts are designed in Onshape. You can copy the projects at these links. The rendered out STL files are included in the parts directory.
Control box:
https://cad.onshape.com/documents/043c3a39a97f1b821955c03b/w/93248ce4fd4cac564a92f262/e/5e92760116738c28ccedb01d

Mount for the connector on the back of the fish.
https://cad.onshape.com/documents/b16a1caab3e71b815caed591/w/0d1a3dc45dbcdb8519a24483/e/a3f8292bcb0fa2a3ff2797b2
This is epoxyed to the connector and to the back of the trout.

# Circuitry
Running on 12v so the system can be powered from a standard lead acid battery.

The tail and mouth are both just on/off but the head is controlled with a bidirectional h-bridge. The head can be driven both forward and backward. The tail and mouth are returned by spring.
10w resistors are used to moderate the speed of the motors. This is wasteful and generates heat, but simple and easily adjustable.

See Schematic PNG
