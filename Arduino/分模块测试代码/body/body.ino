#define body_sensor 36
#define buzzer 15

int body_detected = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer, HIGH);
  
 // pinMode(body_sensor,INPUT);
}

void loop(){
  body_detected = analogRead(body_sensor);
  if (body_detected > 1000){
    Serial.println("someone");
    digitalWrite(buzzer, HIGH); 
    
  }
  else{
    Serial.println("nobody..");
    digitalWrite(buzzer, LOW);
    delay(1000);
  }
}
