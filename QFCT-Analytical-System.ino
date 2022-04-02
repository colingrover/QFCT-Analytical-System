#include <Adafruit_MPU6050.h> // For accelerometer
#include <Adafruit_Sensor.h> // For accelerometer
#include <Wire.h> // For I2C
#include <SPI.h>
#include "SdFat.h" // For microSD reader
#include <LiquidCrystal_I2C.h> // For display
#include <DS1302.h> // For RTC 
#include <EEPROM.h> // For storing which display screen was selected when the system was last turned off

// Pin macros
#define CHIP_SELECT SS // Pin for CS wire from SD reader
#define PREVIOUS_SCREEN_BUTTON 8 // pin for buttton to decrement displaySelect
#define PAUSE_SCREEN_BUTTON 7 // pin for button that pauses display
#define NEXT_SCREEN_BUTTON 6 // pin for buttton to increment displaySelect
#define FLAG_DATA_BUTTON 5 // pin for button that adds a flag in line of data
#define THERMISTOR_PIN A0 // Thermistor
#define IRPIN A1 // IR Sensor

// Values for thermistor calculations
#define THERMISTOR_RESISTANCE 10000.0 // Nominal resistance of thermistor (10k ohms)
#define THERMISTOR_BETA_COEFFICIENT 3950.0 // If we can't find this, we can calculate it ourselves with a known temperature (https://www.ametherm.com/wp-content/uploads/2017/07/Beta.jpg)
#define THERMISTOR_NOMINAL_TEMPERATURE 298.15 // Temperature at which nominal resistance of thermistor is accurate (in C)

// X, Y, Z offsets to account for accelerometer miscalibration
#define X_OFF -0.6 // acclerometer x calibration term
#define Y_OFF 0.25 // acclerometer y calibration term
#define Z_OFF 2.6 // acclerometer z calibration term

// File name for csv
#define FILE_BASE_NAME "Data_"


// Necessary object declarations for hardware
SdFat microSD; // microSD card/reader
SdFile dataFile; // File on microSD card
Adafruit_MPU6050 mpu; // Accelerometer
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD display, at I2C address 0x27, size 16x2
DS1302 rtc(2,3,4); // RTC: RST, DAT, CLK


Time rtcTime; // Holds time retrieved from rtc

// Variables for UI 
int displaySelect = EEPROM.read(0); // read which screen to display based off of previous selection
int numDisplays = 3; // Number of different screens in the UI
bool previous, pause, next, flag; // Button states
bool lastPrevious, lastPause, lastNext, lastFlag; // previous button states
bool isPaused = false; // is the display paused

// Variables for getting IR sensor readings
bool IRstate = false; 
long unsigned int time1 = 0, time2 = 0, time3 = 0, currentTime = 0, rotationTime = 0; 
short spm; 

// Only update LCD every few loops
short LCDLoopCounter = 0;
short LCDLoopMax = 20; // Every LCDLoopMax iterations of the loop function, update the LCD screen

// Function prototypes
void microSDSetup();
float getTemperature();
int getSPM(); // Gets spokes per minute
void buttonPresses(); // Handles button events
void printDataToLCD(sensors_event_t a, float temp, int spm); // Displays instantaneous measurements on display
void printDataToSD(sensors_event_t a, float temp, int spm); // Saves measurements to .csv file on MicroSD card
void sdTimeCallback(uint16_t* date, uint16_t* time); // For setting file timestamp


void setup() {
  // Boot LCD screen
  lcd.init(); // Initialize LCD screen
  lcd.clear(); // Clear LCD screen
  lcd.backlight(); // Turn on LCD backlight

  // Initialize the buttons (for UI)
  pinMode(PREVIOUS_SCREEN_BUTTON, INPUT_PULLUP);
  pinMode(PAUSE_SCREEN_BUTTON, INPUT_PULLUP);
  pinMode(NEXT_SCREEN_BUTTON, INPUT_PULLUP);
  pinMode(FLAG_DATA_BUTTON, INPUT_PULLUP);

  // Initialize, name, and date csv file
  microSDSetup();
  
  // Try to initialize mpu6050
  if (!mpu.begin()) {
    lcd.print("Failed to find MPU");
    while (1) {
      delay(10);
    }
  }

  // set accelerometer range to +-2G (Options: 2, 4, 8, 16)
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

  // set low pass filter bandwidth to 21 Hz (Options: 5, 10, 21, 44, 94, 184, 260)
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

// ------------------------- MAIN LOOP ------------------------- //
void loop() {
  // Get new sensor events with the readings
  sensors_event_t a, g, t; // Acceleration, gyroscopic reading, temperature (from MPU6050)
  mpu.getEvent(&a, &g, &t);

  float temp = getTemperature();
  spm = getSPM(); // spokes per minute read from IR sensor

  buttonPresses(); // Event handler for button presses

  // Refresh LCD screen every LCDLoopMax iterations of the loop function (unless paused)
  if (!isPaused && LCDLoopCounter > LCDLoopMax) {
    printDataToLCD(a, temp, spm);
    LCDLoopCounter = 0;
  }
  
  printDataToSD(a, temp, spm);

  // Force data to microSD and save the file to avoid data loss. (if unsuccesful, error msg on LCD)
  if (!dataFile.sync()) {
    lcd.setCursor(0, 0);
    lcd.print("write error");
  }

  LCDLoopCounter++;
}
// ------------------------ ^MAIN LOOP^ ------------------------ //

void microSDSetup() {
  const int BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1; // Number of characters in base name
  char fileName[13] = FILE_BASE_NAME "00.csv";

  // Initialize microSD reader at 50 MHz. If error (ex. no microSD inserted), do not continue, inform user
  if (!microSD.begin(CHIP_SELECT, SD_SCK_MHZ(50))) {
    lcd.print("SD Failure");
    microSD.initErrorHalt();
    while (1) {}
  }

  // Timestamp file with date/time from RTC
  rtcTime = rtc.getTime();
  SdFile::dateTimeCallback(sdTimeCallback);

  if (BASE_NAME_SIZE > 6) {
    lcd.print("file name error");
  }

  // Iterates through file names until it finds unused one
  while (microSD.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      lcd.print("Can't name file");
    }
  }

  // Open/create file to write only, if error then print on LCD
  if (!dataFile.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    lcd.print("File open error");
    while (1) {} // Halt program
  }

  // Print headers
  dataFile.println("Time (s),X Acceleration (m/s^2),Y Acceleration (m/s^2),Z Acceleration (m/s^2),Spokes Per Minute (SPM),Temperature (C),Flag");
}

float getTemperature() {
  float voltage = analogRead(THERMISTOR_PIN); // Reading from thermistor
  float resistance = THERMISTOR_RESISTANCE * ((1023.0 / voltage) - 1.0); // Resistance reading from thermistor

  // Beta parameter equation (from Steinhart-Hart equation): 1/T = (1/T_0) + (1/B)ln(R/R_0)
  // After solving for T: T = (B(T_0)) / (B + (T_0)ln(R/R_0))
  //  T_0 = thermistor nominal temperature
  //  R_0 = thermistor nominal resistance
  //  R = resistance reading
  //  B = thermistor beta coefficient
  float temp = THERMISTOR_BETA_COEFFICIENT * (THERMISTOR_NOMINAL_TEMPERATURE);
  temp /= THERMISTOR_BETA_COEFFICIENT + ((THERMISTOR_NOMINAL_TEMPERATURE) * log(resistance / THERMISTOR_RESISTANCE));

  return temp - 273.15-45; // Convert to celcius before returning (-45 to account for sensor error of unknown source)
}

int getSPM(){
  //IR sensor --> spm (spokes per min)
  IRstate = digitalRead(IRPIN); 
   if (IRstate == false && millis() - currentTime > 20) {  
      time3 = time2; 
      time2 = time1; 
      time1 = currentTime; 
      currentTime = millis(); 
      rotationTime = (currentTime - time3)/3; //takes the time it takes for the wheel to do 1.5 rotations, and divides bit by 1.5. 
      spm = 60000/rotationTime; 
      return spm;
}
}

void buttonPresses() {
  previous = !digitalRead(PREVIOUS_SCREEN_BUTTON);
  pause = !digitalRead(PAUSE_SCREEN_BUTTON);
  next = !digitalRead(NEXT_SCREEN_BUTTON);
  flag = !digitalRead(FLAG_DATA_BUTTON);

  // if "previous display" button pressed, decrement display state
  if (previous && !lastPrevious && displaySelect != 0) {
    displaySelect--;
    EEPROM.write(0,displaySelect); // store current display selection to EEPROM
  }
  // if "pause" button pressed, invert pause state
  if (pause && !lastPause) {
    isPaused = !isPaused;
  }
  // if "next display" button pressed, increment display state
  if (next && !lastNext && displaySelect != numDisplays-1) {
    displaySelect++;
    EEPROM.write(0,displaySelect); // store current display selection to EEPROM
  }
  // if flag button pressed, add flag ("*") to csv
  if (flag && !lastFlag) {
      dataFile.print(millis() / 1000.0);
      dataFile.println(",,,,,,*");
  }
    
  lastPrevious = previous;
  lastPause = pause;
  lastNext = next;
  lastFlag = flag;
}

void printDataToLCD(sensors_event_t a, float temp, int spm) {
  // Display different data depending on screen selection
  switch (displaySelect) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Z:");
      lcd.print(a.acceleration.z + Z_OFF);
      lcd.print("     ");
    
      lcd.setCursor(8, 0);
      lcd.print("T:");
      lcd.print(temp);
      lcd.print("     ");
    
      lcd.setCursor(0, 1);
      lcd.print("SPM:");
      lcd.print(spm);
      lcd.print("               ");
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("X:");
      lcd.print(a.acceleration.x + X_OFF);
      lcd.print("     ");
    
      lcd.setCursor(8, 0);
      lcd.print("Y:");
      lcd.print(a.acceleration.y + Y_OFF);
      lcd.print("     ");
    
      lcd.setCursor(0, 1);
      lcd.print("Z:");
      lcd.print(a.acceleration.z + Z_OFF);
      lcd.print("     ");

      lcd.setCursor(8, 1);
      lcd.print("  m/s^2     ");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("QFCT     SCI '25");
      lcd.setCursor(0,1);
      lcd.print(" CG ET GM JD SA");
      break;
  }
}

void printDataToSD(sensors_event_t a, float temp, int spm) {
  dataFile.print(millis() / 1000.0); // Time in seconds
  dataFile.print(",");
  dataFile.print(a.acceleration.x + X_OFF); // X-Acceleration in m/s^2
  dataFile.print(",");
  dataFile.print(a.acceleration.y + Y_OFF); // Y-Acceleration in m/s^2
  dataFile.print(",");
  dataFile.print(a.acceleration.z + Z_OFF); // Z-Acceleration in m/s^2
  dataFile.print(",");
  dataFile.print(spm); // Wheel spokes per minute
  dataFile.print(",");
  if (temp > -273.15) { // voltage reading has been returning 0 which makes function output -318.15 if it didn't work
    dataFile.print(temp); // temperature in C
  }
  dataFile.println(",");
}

void sdTimeCallback(uint16_t* date, uint16_t* time) {
 *date = FAT_DATE(rtcTime.year, rtcTime.mon, rtcTime.date);
 *time = FAT_TIME(rtcTime.hour, rtcTime.min, rtcTime.sec);
}
