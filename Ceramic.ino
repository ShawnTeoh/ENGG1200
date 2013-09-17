#include <Servo.h>

#define temp_pin 0
#define end_pin 11
#define target_temp 28
float temp;

Servo servo1;
int pos = 0;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
  servo1.attach(9);
  pinMode(end_pin, INPUT);
  digitalWrite(end_pin, HIGH);
}

float mapf(float x,float in_min,float in_max,float out_min,float out_max){
  return (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}

float temp_get(int pin){
  float voltage=0;

  for (int i=0;i < 10;i++){
    float tmp_voltage = mapf(analogRead(pin),0,1023,0,3.3);
    voltage+=tmp_voltage;
  }
  voltage=voltage/10;
  float out=(voltage-0.5)*100 ;
  
  return out;
}

void run_servo(int time,int drc){
  if (drc == 0){
    servo1.write(0);
    delay(time);
    servo1.write(90);
  } else {
    servo1.write(180);
    delay(time);
    servo1.write(90);
  }
}

void loop(){
  temp=temp_get(temp_pin);
  Serial.print(temp); Serial.println(" degrees C");
  delay(1000);
  if (temp >= target_temp){
    run_servo(1000,1);
  } else if (temp <= 26){
    run_servo(2000,0);
  }
  
  if (digitalRead(end_pin)==LOW){
    while(1);
  }
}

