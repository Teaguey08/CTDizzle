//I've removed the temperature compensation code as the most recent tests have proven to be inaccurate.

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "TSYS01.h" //Visit BlueRobotics for documentation.
#include "MS5837.h" //Visit BlueRobotics for documentation.
#include <SparkFunDS3234RTC.h>  //Visit SparkFun for documentation

SoftwareSerial ecSerial(9,8);  //rx,tx for EZO.
float EC_float = 0; //Visit Atlas-Scientific for documentation on EZ EZO.
char EC_data[48];
char *EC;
byte received_from_sensor = 0;
byte string_received = 0;
TSYS01 tsensor; //Define temperature functions.
MS5837 psensor;   //Define pressure functions.

File datafile;

void setup() {      //Start your engines.
  Serial.begin(9600);     //Set the baud rate to 9600.
  ecSerial.begin(9600);     //Communicate with the EZO at 9600 bps.
  Wire.begin();
  
  if (SD.begin(4)) {  //If an SD card is detected on pin 4.
  char filename[]= "rawCTD00.CSV";
  for(uint8_t i=0;i<100;i++){
    filename[6]=i/10+'0';
    filename[7]=i%10+'0';
    if(!SD.exists(filename)){
      datafile=SD.open(filename,FILE_WRITE); //Create a file with a name in series. Example: RAWCTD00.csv, RAWCTD01.csv
      break;
    }
  }
  }  

  delay(1000);     //Wait a second before continuing. 
  tsensor.init();   //Initialize the temperature sensor.
  psensor.init();   //Initialize the pressure sensor.
  psensor.setModel(MS5837::MS5837_30BA);    //Define the model of the pressure sensor.
  psensor.setFluidDensity(1024); //Set approximate fluid density for pressure sensor.
  rtc.begin(5);    //Chip select for RTC is pin 5.
  //rtc.autoTime();  //Time updates with IDE time everytime you upload. Comment out after time sync to have rtc maintain time.
  delay(1000);   //Wait half a second before continuing.
}



void loop() {     //And around we go.
  tsensor.read();  //Read what the temperature is and hold it.
  delay(10);
  psensor.read();  //Read what the pressure is and hold it.
  delay(10);
  if (ecSerial.available() > 0) {     //If comms are established with the EC EZO.
    delay(100);
    received_from_sensor = ecSerial.readBytesUntil(13,EC_data,48);  //Read incoming data from the EZO.
    EC_data[received_from_sensor] = 0;    //Null terminate the data.
    } 
  if ((EC_data[0] >= 48) && (EC_data[0] <= 57)){
    parse_data(); //If incoming EZO data is a digit and not a letter, parse it. 
  }
  delay(10);
  rtc.update();   //Update the time.
  
  if (datafile) {
    datafile.print(String(rtc.month()) + "/" + String(rtc.date()) + "/" + String(rtc.year())); //Print date to SD card.
    datafile.print(",");   //Comma delimited.
    datafile.print(String(rtc.hour()) + ":" + String(rtc.minute())+":"+String(rtc.second()));  //Print hours, minutes, seconds to SD card.
    datafile.print(",");
    datafile.print(EC); //Print EC to SD card.
    datafile.print(",");
    datafile.print(tsensor.temperature());   //Print temperature to SD card.
    datafile.print(",");
    datafile.println(psensor.pressure());   //Print pressure to SD card.
    datafile.flush();   //Close the file.

    Serial.print(String(rtc.month()) + "/" + String(rtc.date()) + "/" + String(rtc.year())); //Print date to SD card.
    Serial.print(",");   //Comma delimited.
    Serial.println(String(rtc.hour()) + ":" + String(rtc.minute())+":"+String(rtc.second()));  //Print hours, minutes, seconds to SD card.
    Serial.print(",");
    Serial.print(EC);
    Serial.print(",");
    Serial.print(tsensor.temperature());
    Serial.print(",");
    Serial.println(psensor.pressure());
  }
  delay(750); //Wait 750 milliseconds.
  //Total loop time is ~1 second.
}

void parse_data() {   //If the parse_data function is called, parse incoming EZO data at the commas.
  EC = strtok(EC_data, ",");
}



