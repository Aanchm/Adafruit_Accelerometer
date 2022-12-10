
#include <SdFat.h>
#include <SPI.h>
#include <Adafruit_ISM330DHCX.h>
#include <stdio.h>
#include <Arduino.h>
#include <stdlib.h>

SdFat sd;
Adafruit_ISM330DHCX ism330dhcx;

#define LSM_CS 10

const int chipSelect = 4;

int button_on = 5;                  // Sets button pins
int button_off = 6;
int led = 9;                        // Sets LED pin
int on = 0;
int off = 0;
int flag = 0;                      // Datasetting flag

int filecount = 1;                 // Count for filenames
String filename;

int bufsz = 512;            // Buffer size

File dataFile;

typedef struct {
  unsigned long timest;
  float data[6];
} datatyp;

datatyp databuf[512];

/* SETUP CODE*/
void setup() {
  delay(1000);
  SPI.begin();

  //   If SD isnt present and can't be initialized do nothing
  if (!sd.begin(chipSelect, SD_SCK_MHZ(10))) {
    sd.initErrorHalt();
  }

  // If accel isnt present and can't be initialized do nothing
  if (!ism330dhcx.begin_SPI(LSM_CS)) {
    while (1) {
      delay(10);
    }
  }
  // Set accelerometer parameters
  ism330dhcx.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
  ism330dhcx.setGyroRange(ISM330DHCX_GYRO_RANGE_4000_DPS);
  ism330dhcx.setAccelDataRate(LSM6DS_RATE_6_66K_HZ);
  ism330dhcx.setGyroDataRate(LSM6DS_RATE_6_66K_HZ);

  ism330dhcx.configInt1(false, false, true); // accelerometer DRDY on INT1
  ism330dhcx.configInt2(false, true, false); // gyro DRDY on INT2

  pinMode(button_on, INPUT);
  pinMode(button_off, INPUT);
  pinMode(led, OUTPUT);

  filename = (String)"datalog" + filecount + (String)".txt";
}


/* MAIN CODE*/
void loop() {

  // If on button is presse: create filename and set flag
  if (digitalRead(button_on) == LOW && flag == 0) {
    flag = 1;
    filename = (String)"datalog" + filecount + (String)".txt";
    dataFile = sd.open(filename, O_CREAT | O_WRITE);
    digitalWrite(led, HIGH);
    delay(60000);
  }

  // If off button is pressed: unset flag, print OVER at the end of the file, increase filecount
  if (digitalRead(button_off) == LOW && flag == 1) {
    flag = 0;
    digitalWrite(led, LOW);
    if (dataFile) {
      dataFile.print("OVER");
      dataFile.close();
      filecount += 1;
    }
  }

  // While flag is pressed collect data and write it to the SD card
  if (flag == 1) {
    int count = 0;
    while (count < bufsz) {
      sensors_event_t accel;
      sensors_event_t gyro;
      sensors_event_t temp;
      ism330dhcx.getEvent(&accel, &gyro, &temp);
      databuf[count].timest = micros();
      databuf[count].data[0] = accel.acceleration.x;
      databuf[count].data[1] = accel.acceleration.y;
      databuf[count].data[2] = accel.acceleration.z;
      databuf[count].data[3] = gyro.gyro.x;
      databuf[count].data[4] = gyro.gyro.y;
      databuf[count].data[5] = gyro.gyro.z;
      count++;
    }
    dataFile.write((const uint8_t *)&databuf, sizeof(databuf));
  }
}
