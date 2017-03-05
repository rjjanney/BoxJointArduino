# BoxJointArduino
Arduino custom interface to control woodworking box joint jig. Uses Nokie 5110 display, rotary encoder with click, stepper motor.

I had built this woodworking box joint jig from plans by Matthias Wandel - the woodworking genius. http://woodgears.ca/box_joint/jig.html
I was having trouble getting the gears to mesh without splintering and grinding to pieces, probably because I had inferior
birch plywood. 
The idea is that you use different gears depending on how much you want to move the sled to cut box joints.

This code replaces the hand cranked gears with a stepper motor controlled by an Arduino.

You enter the desired slot width, the kerf of your blade, and the controller moves the sled and pauses for you to cut.

It was a fun project to program, but I wasn't able to get the stepper motor to line up precisely with the threaded rod, 
or maybe I just need a more powerful stepper motor. I haven't made any box joints with this jig so far.
