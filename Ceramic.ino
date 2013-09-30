#include <Servo.h>

#define temp_pin_cld1 0
#define temp_pin_cld2 1
#define temp_pin_hot1 2
#define temp_pin_hot2 3
#define temp_pin_out 4
#define flow_pin_cld 2
#define flow_pin_hot 3
#define servo_pin1 9
#define servo_pin2 10
#define end_pin 11
#define target_temp 28

// Temperature sensors
float temp_cld1;
float temp_cld2;
float temp_cld_avr;
float temp_hot1;
float temp_hot2;
float temp_hot_avr;
float temp_out;

// Flow meters
volatile unsigned int pulse_count1;
volatile unsigned int pulse_count2;
float flow_rate_cld;
float flow_rate_hot;
float flow_litres1;
float flow_litres2;
float total_litres;
unsigned long old_time;

// Servo
Servo servo1;
Servo servo2;
int pos1;
int pos2;

// Control system
float cld_ratio;
float hot_ratio;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);

  // Servo
  servo1.attach(servo_pin1);
  servo2.attach(servo_pin2);
  pos1=0;
  pos2=0;

  // TWS
  pinMode(end_pin, INPUT);
  digitalWrite(end_pin, HIGH);

  // Flow meters
  pulse_count1=0;
  pulse_count2=0;
  flow_rate_cld=0.0;
  flow_rate_hot=0.0;
  flow_litres1=0.0;
  flow_litres2=0.0;
  total_litres=0.0;
  old_time=0;
  pinMode(flow_pin_cld, INPUT);
  pinMode(flow_pin_hot, INPUT);
  digitalWrite(flow_pin_cld,HIGH);
  digitalWrite(flow_pin_hot,HIGH);
  attachInterrupt(0, pulse_counter1, FALLING);
  attachInterrupt(1, pulse_counter2, FALLING);
  
  // Control system
  cld_ratio=0.0;
  hot_ratio=0.0;
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

void pulse_counter1(){
  pulse_count1++;
}

void pulse_counter2(){
  pulse_count2++;
}

void loop(){
  temp_cld1=temp_get(temp_pin_cld1);
  temp_cld2=temp_get(temp_pin_cld2);
  temp_hot1=temp_get(temp_pin_hot1);
  temp_hot2=temp_get(temp_pin_hot2);
  temp_out=temp_get(temp_pin_out);

  temp_cld_avr=(temp_cld1+temp_cld2)/2; // Used var
  temp_hot_avr=(temp_hot1+temp_hot2)/2; // Used var
  Serial.print(temp_cld_avr); Serial.println(" degrees C (cold)");
  Serial.print(temp_hot_avr); Serial.println(" degrees C (hot)");
  Serial.print(temp_out); Serial.println(" degrees C (out)");
  delay(100);

  if((millis()-old_time) > 1000){ 
    detachInterrupt(0);
    detachInterrupt(1);

    flow_litres1=pulse_count1/3300;
    flow_litres2=pulse_count2/3300;
    
    flow_litres1=flow_litres1/old_time*60;
    flow_litres2=flow_litres2/old_time*60;
    
    flow_rate_cld=flow_litres1*60; // Used var
    flow_rate_hot=flow_litres2*60; // Used var

    old_time=millis();

    total_litres+=flow_litres1;
    total_litres+=flow_litres2;

    Serial.print(flow_rate_cld); Serial.println(" litres/min (cold)");
    Serial.print(flow_rate_hot); Serial.println(" litres/min (hot)");
    Serial.print(total_litres); Serial.println(" litres (total)");

    pulse_count1=0;
    pulse_count2=0;
    attachInterrupt(0, pulse_counter1, FALLING);
    attachInterrupt(1, pulse_counter2, FALLING);
  }

  // DOING: Translate control algorithm into Arduino code
  float temp_ratio_tmp1=(target_temp-temp_cld_avr)/(temp_hot_avr-target_temp);
  float temp_ratio_tmp2=temp_ratio_tmp1/(temp_ratio_tmp1+1);
  float temp_ratio=temp_ratio_tmp2/(1-temp_ratio_tmp2); // Used var
  float flow_ratio=(flow_rate_hot/60)/(flow_rate_cld/60); // Used var

  if (temp_ratio < flow_ratio){
    hot_ratio=temp_ratio/flow_ratio; // Output var
    cld_ratio=1.0; // Output var
  } else {
    cld_ratio=flow_ratio/temp_ratio; // Output var
    hot_ratio=1.0; // Output var
  }

  // 90 is fully open, 0 is fully closed
  if (cld_ratio == 1.0){
    servo1.write(90);
    pos1=90;
  } else{
    servo1.write(90);
    pos1=90;
  }

  if (hot_ratio == 1.0){
    servo2.write(90);
    pos2=90;
  } else{
    servo2.write(90);
    pos2=90;
  }

  // Prevent both valves from closing at same time
  if (pos1 == 0 && pos2 == 0){
    servo1.write(45);
    pos1=45;
    servo2.write(45);
    pos2=45;
  }

  if (digitalRead(end_pin) == LOW){
    // TODO: Shut down sequence
    servo1.write(0);
    servo2.write(0);
    while(1);
  }
}

