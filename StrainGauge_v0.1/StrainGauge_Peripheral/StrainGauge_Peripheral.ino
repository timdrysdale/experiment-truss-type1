//v0.1
//PERIPHERAL Arduino for taking parallel strain gauge measurements.
//Each (full bridge) Wheatstone bridge is connected to an HX711 analogue to digital converter (ADC).
//Each HX711 has a DATA (DT) pin connected to a unique pin on the Arduino.
//Each HX711 is also connected to a common CLOCK (SCK) pin.
//Vcc is Arduino logic level (+3.3V for Nano).

//Measure and transfer strain gauge values through I2C when requested from the CONTROLLER

#include "HX711.h"
#include <Wire.h>

const int numGauges = 2;
const int GAUGE_A_DT = 3; //DATA pins
const int GAUGE_B_DT = 4;
const int SCK_PIN = 2;  //Common CLOCK pin

HX711 scale_0;
HX711 scale_1;

HX711 gaugeScales[numGauges] = {scale_0, scale_1};

typedef union
{
  float number;
  uint8_t bytes[4];
} FLOATUNION;

FLOATUNION data[numGauges];



void setup() {
 
 gaugeScales[0].begin(GAUGE_A_DT, SCK_PIN);
 gaugeScales[1].begin(GAUGE_B_DT, SCK_PIN);

 for(int i=0;i<numGauges;i++){
  gaugeScales[i].set_gain(128);
 }
 
 gaugeScales[0].set_scale(-15184);   //calibrated with the load cell on the real truss -> OUTPUTS force in newtons
 gaugeScales[1].set_scale(-3231);         //calibrated with truss member 1  -> outputs strain in micro-strain

 tareAllScales();

  //I2C communication with CONTROLLER arduino
 Wire.begin(8);
 Wire.onRequest(requestHandler);
 Wire.onReceive(receiveHandler);
  
}

void loop() {

}

void requestHandler(){
  //get scale data
  for(int i=0; i<numGauges;i++){
    
    FLOATUNION d;
    
    if(gaugeScales[i].is_ready()){
      
      d.number = gaugeScales[i].get_units(10);
      data[i] = d;
      
    } else{
      
      d.number = 0.0;
      data[i] = d;
    }
  }

  //write scale data over I2C
  
  for(int i=0;i<numGauges;i++){
    
    Wire.write(data[i].bytes, 4);
  
  }
  
}

void receiveHandler(int numBytes){
  char c = Wire.read();
  if(c == 't'){
    tareAllScales();
  }
}

void tareAllScales(){
  for(int i=0;i<numGauges;i++){
    gaugeScales[i].tare();
   }
}
