//v0.1
//PERIPHERAL Arduino for taking parallel strain gauge measurements.
//Each (full bridge) Wheatstone bridge is connected to an HX711 analogue to digital converter (ADC).
//Each HX711 has a DATA (DT) pin connected to a unique pin on the Arduino.
//Each HX711 is also connected to a common CLOCK (SCK) pin.
//Vcc is Arduino logic level (+3.3V for Nano).

//Measure and transfer strain gauge values through I2C when requested from the CONTROLLER

//Author David P. Reid

#include "HX711.h"
#include <Wire.h>

const int numGauges = 7;
const int GAUGE_0_DT = 3; //DATA pins
const int GAUGE_1_DT = 4;
const int GAUGE_2_DT = 5;
const int GAUGE_3_DT = 6;
const int GAUGE_4_DT = 7;
const int GAUGE_5_DT = 8;
const int GAUGE_6_DT = 9;

const int data_pins[numGauges] = {GAUGE_0_DT, GAUGE_1_DT, GAUGE_2_DT, GAUGE_3_DT, GAUGE_4_DT, GAUGE_5_DT, GAUGE_6_DT};

const int SCK_PIN = 2;  //Common CLOCK pin

HX711 scale_0;
HX711 scale_1;
HX711 scale_2;
HX711 scale_3;
HX711 scale_4;
HX711 scale_5;
HX711 scale_6;

const int scale_load = -15184;
const int scale_factor_1 = 4050;

const int scale_factor_2 = scale_factor_1*1.030;
const int scale_factor_3 = scale_factor_1*0.977;
const int scale_factor_4 = scale_factor_1*0.947;
const int scale_factor_5 = scale_factor_1*0.818;
const int scale_factor_6 = scale_factor_1*0.924;

HX711 gaugeScales[numGauges] = {scale_0, scale_1, scale_2, scale_3, scale_4, scale_5, scale_6};

const unsigned long reading_interval = 1000;
unsigned long current_time = millis();

int next_index = 0;

typedef union
{
  float number;
  uint8_t bytes[4];
} FLOATUNION;

FLOATUNION d0;
FLOATUNION d1;
FLOATUNION d2;
FLOATUNION d3;
FLOATUNION d4;
FLOATUNION d5;
FLOATUNION d6;

FLOATUNION data[numGauges] = {d0, d1, d2, d3, d4, d5, d6};



void setup() {
 initialiseScales();
 setGain(128);
 
 //gaugeScales[0].set_scale(412);
 gaugeScales[0].set_scale(scale_load);   //calibrated with the load cell on the real truss -> OUTPUTS force in newtons
 gaugeScales[1].set_scale(scale_factor_1);          //member 1, calibrated with truss member 1  -> outputs strain in micro-strain

  gaugeScales[2].set_scale(scale_factor_2);          //member 2
  gaugeScales[3].set_scale(scale_factor_3);          //member 3
  gaugeScales[4].set_scale(scale_factor_4);          //member 4
  gaugeScales[5].set_scale(scale_factor_5);          //member 5
  gaugeScales[6].set_scale(scale_factor_6);          //member 6

  
 tareAllScales();

  //I2C communication with CONTROLLER arduino
 Wire.begin(8);
 Wire.onRequest(requestHandler);
 Wire.onReceive(receiveHandler);

// Serial.begin(57600);
// while(!Serial);
  
}

void loop() {
  if(millis() >= current_time + reading_interval)
  {
    for(int i=0; i<numGauges;i++){
    
      FLOATUNION d;
      
      if(gaugeScales[i].wait_ready_timeout(100)){
        
        d.number = gaugeScales[i].get_units(5);
        data[i] = d;
//        Serial.print("gauge ");
//        Serial.print(i);
//        Serial.print(": ");
//        Serial.print(d.number);
//        Serial.print("\t");
        
      } else{
        
        d.number = -999;
        data[i] = d;
//        Serial.print("gauge ");
//        Serial.print(i);
//        Serial.print(": ");
//        Serial.print(d.number);
//        Serial.print("\t");
      }
  
      //delay(100);
    }

//    Serial.println("");

    current_time = millis();
  }
  
  
}

//Only send one gauge value on any request
void requestHandler(){
  
  Wire.write(data[next_index].bytes, 4);
  
  next_index = (next_index + 1) % numGauges;

  
}

void receiveHandler(int numBytes){
  char c = Wire.read();
  if(c == 't')
  {
    tareAllScales();
  } 
  else if(c == '0')
  {
    next_index = 0;
  }
  else if(c == 'r')
  {
    fullReset();
  }
}

void initialiseScales(){
  for(int i=0;i<numGauges;i++){
    gaugeScales[i].begin(data_pins[i], SCK_PIN);
  }
}

void setGain(int gain){
  for(int i=0;i<numGauges;i++){
    gaugeScales[i].set_gain(gain);
  }
}

void tareAllScales(){
  for(int i=0;i<numGauges;i++){
    gaugeScales[i].tare();
   }
}

void fullReset(){
  initialiseScales();
  setGain(128);
 
  //gaugeScales[0].set_scale(412);
  gaugeScales[0].set_scale(scale_load);   //calibrated with the load cell on the real truss -> OUTPUTS force in newtons
  gaugeScales[1].set_scale(scale_factor_1);          //member 1, calibrated with truss member 1  -> outputs strain in micro-strain
  gaugeScales[2].set_scale(scale_factor_2);          //member 2
  gaugeScales[3].set_scale(scale_factor_3);          //member 3
  gaugeScales[4].set_scale(scale_factor_4);          //member 4
  gaugeScales[5].set_scale(scale_factor_5);          //member 5
  gaugeScales[6].set_scale(scale_factor_6);          //member 6

  
  tareAllScales();

  next_index = 0;
}
