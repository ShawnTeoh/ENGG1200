#include <Servo.h>

#define temp_pin1 0
#define temp_pin2 1
#define flow_pin1 2
#define flow_pin2 3
#define end_pin 11
#define target_temp 28

// Temperature sensors
float temp1;
float temp2;

// Flow meters
float calibrationFactor=4.5;
volatile byte pulse_count1;
volatile byte pulse_count2;
float flow_rate1;
float flow_rate2;
unsigned int flow_mlitres1;
unsigned int flow_mlitres2;
unsigned long total_mlitres1;
unsigned long total_mlitres2;
unsigned long old_time1;
unsigned long old_time2;

// Servo
Servo servo1;
Servo servo2;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
  
  servo1.attach(9);
  servo2.attach(10);
  
  pinMode(end_pin, INPUT);
  digitalWrite(end_pin, HIGH);
  
  pulse_count1=0;
  pulse_count2=0;
  flow_rate1=0.0;
  flow_rate2=0.0;
  flow_mlitres1=0;
  flow_mlitres2=0;
  total_mlitres1=0;
  total_mlitres2=0;
  old_time1=0;
  old_time2=0;
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

void pulse_counter1()
{
  pulse_count1++;
}

void pulse_counter2()
{
  pulse_count2++;
}

void loop(){
  temp1=temp_get(temp_pin1);
  Serial.print(temp1); Serial.println(" degrees C");
  delay(1000);
  
  if((millis() - old_time1) > 1000){ 
    detachInterrupt(0);

    flow_rate1=((1000.0/(millis()-old_time1))*pulse_count1)/calibrationFactor;

    old_time1=millis();

    flow_mlitres1=(flow_rate1/60)*1000;

    total_mlitres1 += flow_mlitres1;
    total_mlitres2 += flow_mlitres2;

    unsigned int frac;

    Serial.println(int(flow_rate1));

    frac = (flow_rate1-int(flow_rate1))*10;
    Serial.println(frac, DEC) ;
    Serial.println(flow_mlitres1);

    Serial.println(total_mlitres1);
    Serial.println(total_mlitres2);
    attachInterrupt(0, pulse_counter1, FALLING);
  }

  if (temp1 >= target_temp){
    run_servo(1000,1);
  } else if (temp1 <= 26){
    run_servo(2000,0);
  }

  if (digitalRead(end_pin)==LOW){
    while(1);
  }
}

