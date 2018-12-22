# intro-to-embedded-final-project-team-one

The purpose of this system is to enroll and identify a user with a fingerprint sensor and sending that data
from the sensor to the arduino to the MSP. 

Materials:

-MSP430F5529
-Arduino Uno
-GT511-C1R fingerprint sensor
-2 different color LEDs
-breadboard
-2 1k resistors for each LED
-Laptop or other Personal Computer with atleast 2 COM ports. 
-Realterm to show data being recieved from the MSP

How it works:

When an enrolled user is identified by the scanner, it will send a pulse from the Arduino to a pin in the MSP.
When the MSP recieves that pulse, it will trigger an interrupt and toggle an LED. There are two enrolled users
in this design. When user 1 is identified, the red LED will trigger. When user 2 is identified, the blue
LED will trigger. 