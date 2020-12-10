#include "Arduino.h"
#include "force.h"
#include <HX711.h>
#include <EEPROM.h>

//Version number //it always need to start with: "Force_Sensor-"
String version = "Force_Sensor-0.4";

int tareAddress = 0;
int calibrationAddress = 4;

HX711 scale;

//Data comming from the cell after resting the offset weight
float offsetted_data = 0;

//Data resulting of appying the calibration_factor to the offsetted_data
float scaled_data = 0;

//The weight used to calibrate the cell
float weight = 0.0;

//Wether the sensor has to capture or not
boolean capturing = false;

//wether the tranmission is in binary format or not
boolean binaryFormat = false;

unsigned long lastTime = 0;
unsigned long currentTime = 0;
unsigned long elapsedTime = 0;
unsigned long totalTime = 0;


void init_scale()
{
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    long tare = 0;
    
    EEPROM.get(tareAddress, tare);
    if (tare == -151) {
        scale.set_offset(10000);// Usual value  in Chronojump strength gauge
        EEPROM.put(tareAddress, 10000);
    } else {
        scale.set_offset(tare);
    }


    //The factor to convert the units coming from the cell to the units used in the calibration
    float calibration_factor = 0.0f;
    EEPROM.get(calibrationAddress, calibration_factor);
    if (isnan(calibration_factor)) {
        scale.set_scale(915.0);// Usual value  in Chronojump strength gauge
        EEPROM.put(calibrationAddress, 915.0);
    } else {
        scale.set_scale(calibration_factor);
    }
}

void start_capture()
{
  Serial.println("Starting capture...");
  totalTime = 0;
  lastTime = micros();
  capturing = true;
}

void end_capture()
{
  stop_capturing();
  Serial.print("Capture ended:");
  Serial.println(scale.get_offset());
}
void get_version()
{
  Serial.println(version);
}

void get_calibration_factor()
{
  Serial.println(scale.get_scale());
}

void set_calibration_factor(String inputString)
{
  //Reading the argument of the command. Located within the ":" and the ";"
  String calibration_factor = get_command_argument(inputString);
  //Serial.println(calibration_factor.toFloat());
  scale.set_scale(calibration_factor.toFloat());
  float stored_calibration = 0.0f;
  EEPROM.get(calibrationAddress, stored_calibration);
  if (stored_calibration != calibration_factor.toFloat()) {
    EEPROM.put(calibrationAddress, calibration_factor.toFloat());
  }
  Serial.println("Calibration factor set");
}

void calibrate(String inputString)
{
  //Reading the argument of the command. Located within the ":" and the ";"
  String weightString = get_command_argument(inputString);
  float weight = weightString.toFloat();
  //mean of 255 values comming from the cell after resting the offset.
  double offsetted_data = scale.get_value(50);
  //offsetted_data / calibration_factor
  float calibration_factor = offsetted_data / weight / 9.81; //We want to return Newtons.
  scale.set_scale(calibration_factor);
  EEPROM.put(calibrationAddress, calibration_factor);
  Serial.print("Calibrating OK:");
  Serial.println(calibration_factor);
}

void tare()
{
  scale.tare(50); //Reset the scale to 0 using the mean of 255 raw values
  EEPROM.put(tareAddress, scale.get_offset());
  Serial.print("Taring OK:");
  Serial.println(scale.get_offset());
}

void get_tare()
{
  Serial.println(scale.get_offset());
}

void set_tare(String inputString)
{
  String tare = get_command_argument(inputString);
  long value = tare.toInt();
  scale.set_offset(value);
  long stored_tare = 0;
  EEPROM.get(tareAddress, stored_tare);
  if (stored_tare != value) {
    EEPROM.put(tareAddress, value);
    Serial.println("updated");
  }
  Serial.println("Tare set");
}

String get_command_argument(String inputString)
{
  return (inputString.substring(inputString.lastIndexOf(":") + 1, inputString.lastIndexOf(";")));
}

void get_transmission_format()
{
  if (binaryFormat) {
    Serial.println("binary");
  } else {
    Serial.println("text");
  }
}

bool is_capturing()
{
    return capturing;
}

void stop_capturing() //Dont output data on Serial
{
    capturing = false;
}

void scale_loop()
{
    currentTime = micros();

    //Managing the timer overflow
    if (currentTime > lastTime){      //No overflow
      elapsedTime = currentTime - lastTime;
    } else if (currentTime <= lastTime) {  //Overflow
      elapsedTime = (4294967295 - lastTime) + currentTime; //Time from the last measure to the overflow event plus the currentTime
    }
    totalTime += elapsedTime;
    lastTime = currentTime;
    
    Serial.print(totalTime);
    Serial.print(";");
    Serial.println(scale.get_units(), 2); //scale.get_units() returns a float
}