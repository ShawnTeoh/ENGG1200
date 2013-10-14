#define temp_pin_cld1 0
#define temp_pin_cld2 1
#define temp_pin_hot1 2
#define temp_pin_hot2 3

int target_temp;
float temp_cld1;
float temp_cld2;
float temp_cld_avr;
float temp_hot1;
float temp_hot2;
float temp_hot_avr;

void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
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

void loop(){
  temp_cld1=temp_get(temp_pin_cld1);
  temp_cld2=temp_get(temp_pin_cld2);
  temp_hot1=temp_get(temp_pin_hot1);
  temp_hot2=temp_get(temp_pin_hot2);

  temp_cld_avr=(temp_cld1+temp_cld2)/2; // Used var
  temp_hot_avr=(temp_hot1+temp_hot2)/2; // Used var
  
  Serial.print(temp_cld1); Serial.println(" degrees C (cold1)");
  Serial.print(temp_cld2); Serial.println(" degrees C (cold2)");
  //Serial.print(temp_cld_avr); Serial.println(" degrees C (cold)");
  Serial.print(temp_hot1); Serial.println(" degrees C (hot1)");
  Serial.print(temp_hot2); Serial.println(" degrees C (hot2)");
  //Serial.print(temp_hot_avr); Serial.println(" degrees C (hot)");
  delay(1000);
}
