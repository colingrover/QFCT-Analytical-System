# QFCT-Analytical-System
QFCT Analytical System - APSC100

Okay, since this isn't a proper report, here's a more straightforward description of how this thing works.

## Tachometer
We're using an IR sensor for this. It is found within the grey 3D printed case. As we don't know the details of your team's wheel, we are simply using this device to report "Spokes Per Minute" or SPM. Essentially this is counting the rate at which spokes of the wheel are detected by the sensor (since we are assuming you'll still be using a bike tire as you have previously). This metric is available so you can derive RPM (sorry for making you do the work on this one). Divide SPM we report by how many spokes your tire has and boom there's your RPM. Mount this sensor such that the bulbs are pointing where the spokes will be passing by. If the sensor is not detecting the spokes, we have made the sensitivity-adjusting potentiometer accessible without removing the sensor from its case. If this change doesn't work, the line in the code ```if (IRstate == false && millis() - currentTime > 20) {``` can be changed (reduce 20). If you believe the spokes are moving too fast or are too close to each other, this should help.
## Vibration Sensor
## Temperature Sensor
## UI

## Troubleshooting tip: 
"SD failure" probably means battery is low or disconnected, try fixing this before bothering with any software troubleshooting
