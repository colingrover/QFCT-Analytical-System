# QFCT Analytical System

Okay, since this isn't a proper report, here's a more straightforward description of how this thing works.

## Tachometer
We're using an IR sensor for this. It is found within the grey 3D printed case. As we don't know the details of your team's wheel, we are simply using this device to report "Spokes Per Minute" or SPM. Essentially this is counting the rate at which spokes of the wheel are detected by the sensor (since we are assuming you'll still be using a bike tire as you have previously). This metric is available so you can derive RPM (sorry for making you do the work on this one). Divide SPM we report by how many spokes your tire has and boom there's your RPM. 

Mount this sensor such that the bulbs are pointing where the spokes will be passing by. If the sensor is not detecting the spokes, we have made the sensitivity-adjusting potentiometer accessible without removing the sensor from its case. If this change doesn't work, the line in the code ```if (IRstate == false && millis() - currentTime > 20) {``` can be changed (reduce 20). If you believe the spokes are moving too fast or are too close to each other, this should help.

## Vibration Sensor
We're using an MPU6050 accelerometer for this. It's axes are labelled on the box. Not much to say about it here.

## Temperature Sensor
We're using a thermistor. It is to be mounted to the fuel cell stack such that the bare minimum amount of wire is touching the hot elements (just for safety, and because I don't know how much you can trust the electrical tape that's on that thing), but the end of thermistor obviously must be on what you want to measure.

## UI
We have three screens available. Acceleration is in m/s^2 and temperature is in degrees C. You can add your own if you want to dive through the code, it's in the only switch-case statement in there. If you do add a screen or remove a screen, be sure to change ```numDisplays``` on line 45 to be accurate to your change.

The buttons are labelled. The < button goes to the previous screen, the ⏸︎ button pauses the display (data is still recorded to microSD card). The > button goes to the next screen. The ⚑ button places a checkpoint in the .csv file (a * in the flag column). This will hopefully allow you to quickly find relevant data without having to give the box a big bump to indicate a test start or something.

!! TURN OFF SYSTEM BEFORE REMOVING MICROSD CARD !!

## Troubleshooting tip: 
"SD failure" probably means battery is low or disconnected, try fixing this before bothering with any software troubleshooting

Make sure not to let too many files pile up on the microSD card, I believe the max is 100 before errors will arise.

Breadboard might be kind of loose, if a data wire comes out refer to the pin diagram in the team's phase 4 report to find where it goes. If a power one comes loose, everything is 5V except the MPU and RTC, which take 3.3V (sorry we didn't have time to make a full wiring diagram).
