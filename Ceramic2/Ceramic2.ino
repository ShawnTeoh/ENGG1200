#include <Servo.h>

#define temp_pin_cld1 0
#define temp_pin_cld2 1
#define temp_pin_hot1 2
#define temp_pin_hot2 3

#define flow_pin_cld 2
#define flow_pin_hot 3
#define target_pin 8
#define servo_pin1 9
#define servo_pin2 10
#define end_pin 11

// Temperature sensors
float target_temp;
float temp_cld1;
float temp_cld2;
float temp_cld_avr;
float temp_hot1;
float temp_hot2;
float temp_hot_avr;
float temp_cld_tmp;
float temp_hot_tmp;

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
int angle;

// Control system
float cld_ratio;
float hot_ratio;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
  
  // Temperature
  temp_cld_tmp=15.0;
  temp_hot_tmp=56.5;

  // Servo
  servo1.attach(servo_pin1);
  servo2.attach(servo_pin2);
  pos1=0;
  pos2=0;
  
  // Target temperature
  target_temp=27.0;
  pinMode(target_pin, INPUT);
  digitalWrite(target_pin, LOW);

  // TWS
  pinMode(end_pin, INPUT);
  digitalWrite(end_pin, LOW);

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

int round_angle(float ratio){
  float angle_tmp=ratio*75;
  int angle_trunc=(int) angle_tmp;
  float diff=angle_tmp-angle_trunc;
  
  if (diff <= 0.5){
    return floor(angle_tmp);
  } else{
    return ceil(angle_tmp);
  }
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
  if (digitalRead(target_pin) == HIGH){
    target_temp=35.0;
  }

  if(millis() < 10000){
    temp_cld_avr=temp_cld_tmp;
    temp_hot_avr=temp_hot_tmp;
  } else{
    temp_cld1=temp_get(temp_pin_cld1);
    temp_cld2=temp_get(temp_pin_cld2);
    temp_hot1=temp_get(temp_pin_hot1);
    temp_hot2=temp_get(temp_pin_hot2);

    temp_cld_avr=(temp_cld1+temp_cld2)/2; // Used var
    temp_hot_avr=(temp_hot1+temp_hot2)/2; // Used var
  }
  
  Serial.print(temp_cld_avr); Serial.println(" degrees C (cold)");
  Serial.print(temp_hot_avr); Serial.println(" degrees C (hot)");
  delay(100);

  if((millis()-old_time) > 1000){ 
    detachInterrupt(0);
    detachInterrupt(1);
	
	if(millis() < 10000){
	  temp_cld_tmp+=0.5;
	  temp_hot_tmp-=0.5;
	}

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

  Serial.print(target_temp);Serial.println(" target");
  Serial.print(flow_ratio);Serial.println(" ratio(flow)");
  Serial.print(temp_ratio);Serial.println(" ratio(temp)");
  
  if (temp_ratio < flow_ratio){
    hot_ratio=temp_ratio/flow_ratio; // Output var
    angle=round_angle(hot_ratio)-80;
    angle=abs(angle); // Output var
    cld_ratio=1.0; // Output var
  } else{
    cld_ratio=flow_ratio/temp_ratio; // Output var
    angle=round_angle(cld_ratio)-140;
    angle=abs(angle); // Output var
    hot_ratio=1.0; // Output var
  }

  // servo2: 5 (open) - 80 (closed)
  // servo1: 65 (open) - 140 (closed)
  if (cld_ratio == 1.0){
    servo1.write(65);
    pos1=65;
  } else{
    servo1.write(angle);
    pos1=angle;
  }

  if (hot_ratio == 1.0){
    servo2.write(5);
    pos2=5;
  } else{
    servo2.write(angle);
    pos2=angle;
  }
  delay(100);
  
  Serial.print(pos1);Serial.println(" angle (cld)");
  Serial.print(pos2);Serial.println(" angle (hot)");

  // Prevent both valves from closing at same time
  if (pos1 == 140 && pos2 == 80){
    servo1.write(95);
    pos1=38;
    servo2.write(43);
    pos2=38;
    delay(100);
  }

  if (digitalRead(end_pin) == HIGH){
    // TODO: Shut down sequence
    servo1.write(140);
    servo2.write(80);
    while(1);
  }
}

