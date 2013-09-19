#include <Servo.h>

#define temp_pin1 0
#define temp_pin2 1
#define flow_pin1 2
#define flow_pin2 3
#define servo_pin1 9
#define servo_pin2 10
#define end_pin 11
#define target_temp 28

// Temperature sensors
float temp1;
float temp2;

// Flow meters
volatile byte pulse_count1;
volatile byte pulse_count2;
float flow_rate1;
float flow_rate2;
unsigned int flow_litres1;
unsigned int flow_litres2;
unsigned long total_litres;
unsigned long old_time;

// Servo
Servo servo1;
Servo servo2;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
  
  servo1.attach(servo_pin1);
  servo2.attach(servo_pin2);
  
  pinMode(end_pin, INPUT);
  digitalWrite(end_pin, HIGH);
  
  pulse_count1=0;
  pulse_count2=0;
  flow_rate1=0.0;
  flow_rate2=0.0;
  flow_litres1=0;
  flow_litres2=0;
  total_litres=0;
  old_time=0;
  pinMode(flow_pin1, INPUT);
  pinMode(flow_pin2, INPUT);
  attachInterrupt(0, pulse_counter1, FALLING);
  attachInterrupt(1, pulse_counter2, FALLING);
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

void pulse_counter1(){
  pulse_count1++;
}

void pulse_counter2(){
  pulse_count2++;
}

void loop(){
  temp1=temp_get(temp_pin1);
  temp2=temp_get(temp_pin2);
  Serial.print(temp1); Serial.println(" degrees C");
  Serial.print(temp2); Serial.println(" degrees C");
  delay(100);
  
  if((millis()-old_time) > 1000){ 
    detachInterrupt(0);
    detachInterrupt(1);
    
    flow_litres1=pulse_count1/3300;
    flow_litres2=pulse_count2/3300;
    flow_rate1=flow_litres1/((millis()-old_time)/60);
    flow_rate2=flow_litres2/((millis()-old_time)/60);

    old_time=millis();
    
    total_litres+=flow_litres1;
    total_litres+=flow_litres2;

    Serial.print(flow_rate1); Serial.println(" litres/min");
    Serial.print(flow_rate2); Serial.println(" litres/min");
    Serial.print(total_litres); Serial.println(" litres");
    
    pulse_count1=0;
    pulse_count2=0;
    attachInterrupt(0, pulse_counter1, FALLING);
    attachInterrupt(1, pulse_counter2, FALLING);
  }
  
  // TODO: Translate control algorithm into Arduino code
  if (temp1 >= target_temp){
    run_servo(1000,1);
  } else if (temp1 <= 26){
    run_servo(2000,0);
  }

  if (digitalRead(end_pin) == LOW){
    // TODO: Shut down sequence
    while(1);
  }
}

