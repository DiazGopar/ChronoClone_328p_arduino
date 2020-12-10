#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include "Arduino.h"
#include "serial_command.h"
#include <Encoder.h>  //https://github.com/PaulStoffregen/Encoder
#include <Adafruit_NeoPixel.h> //https://github.com/adafruit/Adafruit_NeoPixel
#include <DebounceEvent.h> //https://github.com/xoseperez/debounceevent
#include "force.h"


#define ENCODER_PHASE_A_PIN     2
#define ENCODER_PHASE_B_PIN     3
#define LED_PIN                 8
#define NUM_PIXELS              1
#define BUTTON_PIN              4
#define JUMP_PIN                9

#define ENCODER_STATE           0
#define JUMP_RACE_STATE         1
#define FORCE_STATE             2

//Global Variables
uint8_t state = ENCODER_STATE;
uint8_t debounce = 50; //Debounce value for jump race
Encoder encoder(ENCODER_PHASE_A_PIN, ENCODER_PHASE_B_PIN);
Adafruit_NeoPixel led(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
int32_t position = 0;
int32_t last_position = 0;
uint32_t last_event_time = 0;
uint32_t current_time = 0;
uint32_t last_time = 0;
uint8_t uart_command_state = UART_WAITING_COMMAND;

void jump_race_callback(uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
  //*1000 / 8  => Convert in 1/8 of microsencods
  if(state == JUMP_RACE_STATE) 
  {
    char buffer[5];
    uint32_t actual_time = micros();  
    uint32_t difference_time = (actual_time - last_event_time) / 8; // Adapt to chronojump software that expect eigths of microseconds
    buffer[0] = FRAME_CHANGE_START;
    buffer[2] = (uint8_t) ((difference_time & 0x00FF0000) >> 16);
    buffer[3] = (uint8_t) ((difference_time & 0x0000FF00) >> 8);
    buffer[4] = (uint8_t) (difference_time & 0x000000FF);
    if(event == EVENT_PRESSED) 
    {
      buffer[1] = 0x01;
      Serial.write(buffer,5);
      last_event_time = actual_time;
    } 
    else if(event == EVENT_RELEASED) 
    {
      buffer[1] = 0x00;
      Serial.write(buffer,5);
      last_event_time = actual_time;
    }
  } 
}

void change_led_state()
{
  led.clear();
  led.setPixelColor(0, led.Color(state == JUMP_RACE_STATE ? 15 : 0, //Red
                                state == ENCODER_STATE ? 15 : 0,    //Green
                                state == FORCE_STATE ? 15 : 0));    //Blue
  led.show();
}

void change_state()
{
  change_led_state();
  
  switch (state)
  {
    case ENCODER_STATE:
      stop_capturing(); //Stop output force data to serial port 
      Serial.end();
      Serial.begin(115200);
      break;

    case JUMP_RACE_STATE:
      stop_capturing();
      Serial.end();
      Serial.begin(9600);
      break;

    case FORCE_STATE:
      Serial.end();
      Serial.begin(115200);

    default:
      break;
  }
}

void button_callback(uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
  if(event == EVENT_RELEASED) {
    //Change state
    state++;
    state %= 3;
    change_state();
  }  
}

DebounceEvent button = DebounceEvent(BUTTON_PIN, button_callback, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);
DebounceEvent jump_race = DebounceEvent(JUMP_PIN, jump_race_callback, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP, 50);

//We receive 10th of ms
void button_set_debounce(uint8_t value)
{
  jump_race = DebounceEvent(JUMP_PIN, jump_race_callback, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP, value*10);
  debounce = value * 10;  
}

uint8_t button_get_debounce()
{
  return debounce;
}

void setup() { 
  Serial.begin(115200);
  
  init_scale(); //Initialize Scale

  led.begin();
  led.clear();
  led.setPixelColor(0, led.Color(0,15,0));
  led.show();  
}

void loop() {
  //Encoder Position
  //long position;
  button.loop();
  
  switch (state)
  {
    case JUMP_RACE_STATE:
      jump_race.loop();
      break;

    case FORCE_STATE:
      if (is_capturing()) {
        scale_loop();
      }
      break;

    case ENCODER_STATE:
      current_time = micros();
      if(current_time - last_time >= 1000) {
        position = encoder.read() >> 1; //From 4X to 2X => Same as divide by 2
        Serial.write(last_position - position);
        last_position = position;
        last_time = current_time;
      }
      break;
    
    default:
      break;
  }
}

void serialEvent()
{
  if(state == FORCE_STATE) 
  {  //Force Sensor Mode
    String inputString = Serial.readString();
    String commandString = inputString.substring(0, inputString.lastIndexOf(":"));
    if (commandString == "start_capture") {
      start_capture();
    } else if (commandString == "end_capture") {
      end_capture();
    } else if (commandString == "get_version") {
      get_version();
    } else if (commandString == "get_calibration_factor") {
      get_calibration_factor();
    } else if (commandString == "set_calibration_factor") {
      set_calibration_factor(inputString);
    } else if (commandString == "calibrate") {
      calibrate(inputString);
    } else if (commandString == "get_tare") {
      get_tare();
    } else if (commandString == "set_tare") {
      set_tare(inputString);
    } else if (commandString == "tare") {
      tare();
    } else if (commandString == "get_transmission_format") {
      get_transmission_format();
    } else {
      Serial.println("Not a valid command");
    }
    inputString = "";
  } 
  else if(state == JUMP_RACE_STATE || state == ENCODER_STATE) 
  { //JUMP RACE or ENCODER Mode   
    uint8_t command = Serial.read();
    switch(uart_command_state) 
    {
      case UART_WAITING_COMMAND:
        switch(command)
        {
          case PORT_SCANNING:
                Serial.print(PORT_SCANNING);
          break;

          case GET_VERSION:
                Serial.print("1.2");
          break;

          case GET_STATUS:
                Serial.write(GET_STATUS);
                jump_race.pressed() ? Serial.write(0x01) : Serial.write(0x00);
                //Serial.write(jump_race.pressed() ? 0x01 : 0x00);//Value 0 or 1 of jump/Race connector input     
          break;

          case GET_DEBOUNCE_TIME:
                Serial.write('0' + button_get_debounce()/10);
          break;

          case SET_DEBOUNCE_TIME:
            uart_command_state = UART_WAITING_DEBOUNCEVALUE;
          break;

          case C1:
          case C2:
          case C3:
          case C4:
          case C5:
          case C6:
          case C7:
          case C8:
          case C9:
          case C10:
          case C11:
          case C12:
          case C13:
          case C14:
          case C15:
            //Reset debouncer to default time
          break;

          default:
            //-1
            Serial.print("-1");  
          break;
        
        }
      break;

      case UART_WAITING_DEBOUNCEVALUE:
        //set debounce value
        button_set_debounce(command);
        uart_command_state = UART_WAITING_COMMAND;
      break;

      default:
        uart_command_state = UART_WAITING_COMMAND;
      break;

    }
  }
}
