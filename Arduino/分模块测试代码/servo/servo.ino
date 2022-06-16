#include <Servo.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define servoPin 13

BluetoothSerial SerialBT;
Servo myServo;  

void setup() {
  Serial.begin(115200); //监听软串口
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  myServo.attach(servoPin); //舵机控制
  myServo.write(0);
//  delay(10000); 
}

void loop() {
  
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  /*
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  */
  while(SerialBT.available()){
    char c;
    c = SerialBT.read();  //读取串口数据
    Serial.println(c);
    switch(c)
    {
      case '1':servo_init();
      break;
      case '2':open_the_door();
      break;
      case '3':close_the_door();
    }
  }
    /*
    while(Serial.available()){
    char c;
    c = Serial.read();  //读取串口数据
    Serial.println(c);
    switch(c)
    {
      case '1':servo_init();
      break;
      case '2':open_the_door();
      break;
      case '3':close_the_door();
    }
  }*/
  delay(200);


}


void open_the_door()  //舵机开门
{
  myServo.write(170);
}
void close_the_door()  //舵机关门
{
  myServo.write(0);
}
void servo_init()  //舵机初始化
{
  myServo.write(10);
}
