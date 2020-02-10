#include <LOLIN_I2C_MOTOR.h>
#include <Wire.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define KEYA D3
#define KEYB D4
#define OLED_RESET 1

#define GESREACTION_TIME 500
#define GESENTRY_TIME 800
#define GES_QUIT_TIME 1000
#define I2C_ADDRESS 0x43
#define I2C_ADDRESS2 0x44

#define DELAY 2000
boolean detectOn = false;
long lastTime = 0;

#include "paj7620.h"

#define PWM_FREQUENCY 1000

Adafruit_SSD1306 display(OLED_RESET);
LOLIN_I2C_MOTOR motor(DEFAULT_I2C_MOTOR_ADDRESS); //I2C address 0x30

void setup() {
  // put your setup code here, to run once:

  //all of this just to initialize the system
  while(motor.PRODUCT_ID != PRODUCT_ID_I2C_MOTOR){
    motor.getInfo();
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(KEYA, INPUT);
  pinMode(KEYB, INPUT);

  Serial.begin(115200);
  uint8_t error = paj7620Init();
  if (error) {
    Serial.print("INIT ERROR, code: ");
    Serial.println(error);
  }
}

void showOled(String input){  //show input on the oled screen with white text
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.println(input);
    display.display();
}

void runMotor(){    //run the motor for a few seconds
        motor.changeFreq(MOTOR_CH_BOTH, PWM_FREQUENCY);
        motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CCW);
        for (int duty = 40; duty <= 100; duty +=1){
          motor.changeDuty(MOTOR_CH_B, duty);
        }
        motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_STANDBY);
}

bool checkState(int state, uint8_t data){
  //check if the input value (data) and its corresponding state is valid to move on the next state
  //we have 3 state: 0:Left - 1:Left - 2: Left
  //Explanation: Sequence left-left-left is created for the sake of usability (the simpler the better)
  //state 3 is the final state, we run the motor
  //output: false if the data is not valid for the state, true otherwise
  
  switch (state){
    case 0: 
      if (data == GES_LEFT_FLAG) {
        return true;
      }
      else return false;
      break;
    case 1: if (data == GES_LEFT_FLAG) {
        return true;
      }
      else return false;
      break;
    case 2: if (data == GES_LEFT_FLAG) {
        return true;
      }
      else return false;
      break;
    default: break;
  }
}

void changeState(int state){
  //input for the sensor
  uint8_t data = 0, data1 = 0, error;
  error = paj7620ReadReg(I2C_ADDRESS, 1, &data); //read gesture

  int wrongsq = 0;  //the number of errors
  bool wrong1 = false;  //check if the user has reached the maximum error (wrong sequence entirely)
    
  // gesture detection
  while(!wrong1){
    error = paj7620ReadReg(I2C_ADDRESS, 1, &data); //read gesture
    if (state > 0 && state < 3) { //show on the oled that it enter state 1 or more
      showOled("Processing");
    }
    if (state == 3) {   //show on the oled that it is done, candies coming out
      showOled("Done");
      runMotor();
      break;
    }

  //check data for sensor
  //Explanation: because the correct sequence is left-left-left, every other data will be treated as an error regardless the current state
  //So we only set wrongsq++; for every other data, this could be changed easily by changing the sequence in the checkState function and 
  //run checkState for the necessary data. We also implement an additional property where the user can have 1 error and still get the candies
  //example: left-right-left-left will still produce the candies while left-left-right-right will not
  if (!error) {
    switch (data){
      case GES_RIGHT_FLAG:
        Serial.println("RIGHT");
        wrongsq++;
        break;
      case GES_LEFT_FLAG:
        Serial.println("LEFT");
        if (checkState(state, data)) state++;
        else wrongsq++;
        break;
      case GES_UP_FLAG:
        Serial.println("UP");
        wrongsq++;
        break;
      case GES_DOWN_FLAG:
        Serial.println("DOWN");
        wrongsq++;
        break;
      case GES_FORWARD_FLAG:
        Serial.println("FORWARD");
        wrongsq++;
        break;
      case GES_BACKWARD_FLAG:
        Serial.println("BACKWARD");
        wrongsq++;
        break;
      case GES_CLOCKWISE_FLAG:
        Serial.println("CLOCKWISE");
        wrongsq++;
        break;
      case GES_COUNT_CLOCKWISE_FLAG:
        Serial.println("ANTI_CLOCK");
        wrongsq++;
        break;
      default:
        paj7620ReadReg(I2C_ADDRESS2, 1, &data);
        if (data == GES_WAVE_FLAG) {
          Serial.println("wave");
          wrongsq++;
        }else {
          Serial.print("?");
        }
        break;
      }
    }
    if (wrongsq >= 2){  //if the number of errors larger than 1 (>=2), wrong1 is true (the sequence of input is wrong)
        wrong1 = true;
        showOled("WRONG~!");
        delay(2000);
        break;
    }
    delay(10);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int state = 0;
   
  //hand gesture
  changeState(state);

  //end the session
  Serial.println("End!");
  delay(1000);
}
