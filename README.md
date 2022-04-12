# QFCT Analytical System

Sorry that the code is overcommented and underorganized - we were under the impression it would remain internal until quite late in the project. As such, we tried to make it legible but reach out if there's a part that isn't.

Okay, since this isn't a proper report, here's a more straightforward description of how this thing works.

## Tachometer
We're using an IR sensor for this. It is found within the grey 3D printed case. As we don't know the details of your team's wheel, we are simply using this device to report "Spokes Per Minute" or SPM. Essentially this is counting the rate at which spokes of the wheel are detected by the sensor (since we are assuming you'll still be using a bike tire as you have previously). This metric is available so you can derive RPM (sorry for making you do the work on this one). Divide SPM we report by how many spokes your tire has and boom there's your RPM. 

Mount this sensor in such a way that the bulbs are pointing where the spokes will be passing by.

## Vibration Sensor
We're using an MPU6050 accelerometer for this. It's axes are labelled on the box. Not much to say about it here.

## Temperature Sensor
We're using a thermistor. It is to be mounted to the fuel cell stack such that the bare minimum amount of wire is touching the hot elements (just for safety, and because I don't know how much you can trust the electrical tape that's on that thing), but the end of thermistor naturally must be on what you want to measure.

## UI
We have three screens available. Acceleration is in m/s<sup>2</sup> and temperature is in °C. You can add your own if you want to dive through the code, it's in the only switch-case statement in there. If you do add a screen or remove a screen, be sure to change ```numDisplays``` on line 45 to be accurate to your change.

If the display brightness/contrast is not ideal for your use case, there is a potentiometer on the back of the LCD which adjusts this (if it was previously at an acceptable level and lighting hasn't drastically changed, this may be an indicator of a low battery).

The buttons are labelled. The < button goes to the previous screen, the ⏸︎ button pauses the display (data is still recorded to microSD card). The > button goes to the next screen. The ⚑ button places a checkpoint in the .csv file (a * in the flag column). This will hopefully allow you to quickly find relevant data without having to give the box a big bump to indicate a test start or something.

**_!! TURN OFF SYSTEM BEFORE REMOVING MICROSD CARD !!_**

## Troubleshooting Tips: 
* "SD failure" probably means battery is low or disconnected, try fixing this before bothering with any software troubleshooting

* Make sure not to let too many files pile up on the microSD card, I believe the max is 100 before errors will arise.

* Breadboard might be kind of loose, if a data wire comes out refer to the pin diagram below to find where it goes. If a power one comes loose, everything is 5V except the MPU and RTC, which take 3.3V (sorry we didn't have time to make a full wiring diagram).
  ![Pin Usage Diagram](https://user-images.githubusercontent.com/102879108/163043017-eb9ee404-8940-4d83-b7ba-a3cadc461d9f.png)

* If the IR sensor readings are innacurate, there could be a few causes, we recommend you: 
  1. Check for obstructions in the sensor casing
  2. Ensure bulbs are pointing in a roughly parallel direction outwards from the casing
  3. Adjust sensor sensitivity via the potentiometer which we have made accessible on the top of the casing
  4. Test the system indoors. If this works, create shade where the sensor will be placed during outdoor testing, as the sunlight is likely interfering with the sensor.
  5. Change line 190 of the code ```if (IRstate == false && millis() - currentTime > 20) {``` (by reducing 20 to a lower integer). If you believe the spokes are moving too fast or are too close to each other, this should help.
