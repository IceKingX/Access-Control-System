// set pin numbers
//const int buttonPin = 4;  // the number of the pushbutton pin
const int ledPin =  4;    // the number of the LED pin
const int lightSensorPin = 14; //光敏电阻

// variable for storing the pushbutton status 
int ledState = 0;

void setup() {
  Serial.begin(115200);  
  // initialize the pushbutton pin as an input
  //pinMode(buttonPin, INPUT);
  // initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
  delay(1000);
}

void loop() {
  // read the state of the pushbutton value
  ledState = digitalRead(ledPin);
  Serial.println(ledState);
  int sensorValue = analogRead(lightSensorPin); // read analog input gpio 14
  Serial.println(sensorValue);
  if(sensorValue > 1000){
    digitalWrite(ledPin, HIGH);
    delay(5000);
  }else{
    digitalWrite(ledPin, LOW);
  }
}
