// 引入 wifi 模块，并实例化，不同的芯片这里的依赖可能不同
#include <WiFi.h>
static WiFiClient espClient;

// 引入阿里云 IoT SDK
#include <AliyunIoTSDK.h>

// 设置产品和设备的信息，从阿里云设备信息里查看
#define PRODUCT_KEY "h2hkCBnfQ1a"
#define DEVICE_NAME "ESP32"
#define DEVICE_SECRET "7421978db738b015b41d646c31fae038"
#define REGION_ID "cn-shanghai"

// 设置 wifi 信息
#define WIFI_SSID "刘家大院内网"
#define WIFI_PASSWD "201820040507"


//引入舵机和蓝牙串口库
#include <Servo.h>
#include "BluetoothSerial.h"

//ESP32 EN按钮使能 蓝牙连接
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//红外测温MLX90614
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();//实例化红外测温

#define servoPin 13 //舵机Pin口
BluetoothSerial SerialBT;//蓝牙串口
Servo myServo;  

#define Buzzer 15      //蜂鸣器

#define ledPin  4    //  LED pin
#define lightSensorPin  36 //光敏电阻


//超声波传感器引脚
#define Trig 5 //引脚Tring 连接 IO 5
#define Echo 18 //引脚Echo 连接 IO 18

//标识符   人员统计
int sig = 0;//门状态 0关1开
int ledState = 0;//LED状态
int normal = 0;
int abnormal = 0;
int acsdata = 0;
int facepass = 0;
int facefail = 0;


void setup() {
  Serial.begin(115200); //监听软串口
  
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!"); 
  
  myServo.attach(servoPin); //舵机控制
  myServo.write(0); //初始为关
  
  mlx.begin();
  
  pinMode(Trig, OUTPUT);//触发输入引脚
  pinMode(Echo, INPUT);//回声输出引脚

  pinMode(Buzzer,OUTPUT);//设置蜂鸣器数字 IO 脚模式，OUTPUT 为输出   
  digitalWrite(Buzzer,HIGH);//初始为不响

  pinMode(ledPin, OUTPUT);//设置LED数字 IO 脚模式，OUTPUT 为输出
  digitalWrite(ledPin, LOW);//初始为不亮
  
  
  // 初始化 wifi
  wifiInit(WIFI_SSID, WIFI_PASSWD);
    
  // 初始化 iot，需传入 wifi 的 client，和设备产品信息
  AliyunIoTSDK::begin(espClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);
  
  // 发送一个数据到云平台，RunningState 是在设备产品中定义的物联网模型的 id
  AliyunIoTSDK::send("ACSdata", 0);//上报门禁人数
  AliyunIoTSDK::send("Normal", 0);//正常体温人数上报
  AliyunIoTSDK::send("Abnormal", 0);//异常体温人数上报
  AliyunIoTSDK::send("FacePass", 0);//人脸通过人数上报
  AliyunIoTSDK::send("FaceFail", 0);//人脸检测失败人数上报
}

void loop() {

  AliyunIoTSDK::loop();//检查连接和定时发送信息
  AliyunIoTSDK::send("DoorState", sig);//上报门锁状态
  AliyunIoTSDK::send("LightState", ledState);//上报灯状态
  digitalWrite(Buzzer, HIGH);
  int sensorValue = analogRead(lightSensorPin); // read analog input gpio 14
  //Serial.println(sensorValue);
  if(sensorValue > 1000){//暗光环境下把灯打开
    digitalWrite(ledPin, HIGH);
    ledState = 1;
    //delay(1000);
  }else{
    digitalWrite(ledPin, LOW);
    ledState = 0;
  }
  
  while(SerialBT.available()){//树莓派端检测到人员门禁系统启动，蓝牙串口读取到数据
    float bodyTemp = temperature();
    AliyunIoTSDK::send("Temperature", bodyTemp);//上报对象温度
    AliyunIoTSDK::send("ACSdata", ++acsdata);//上报门禁人数
    //AliyunIoTSDK::send("DoorState", sig);//上报门锁状态
    char c;
    c = SerialBT.read();  //读取串口数据
    Serial.println(c);
    switch(c)
    {
      case '1':servo_init();///初始化舵机
      break;
      case '2':
      AliyunIoTSDK::send("FacePass", ++facepass);//人脸通过人数上报
      if(bodyTemp < 37.3){
        open_the_door();sig = 1;
        Serial.println("门开了");
        AliyunIoTSDK::send("Normal", ++normal);//正常体温人数上报
        delay(3000);
      }
      else if(bodyTemp >= 37.3){
        Serial.println("HOT!!!");
        AliyunIoTSDK::send("Abnormal", ++abnormal);//异常体温人数上报
        digitalWrite(Buzzer, LOW);
        delay(1000);
      }
      break;
      case '3':
      AliyunIoTSDK::send("FaceFail", ++facefail);//人脸检测失败人数上报
      close_the_door();sig = 0;
      Serial.println("3.门关了");
      if(bodyTemp >= 37.3){
        Serial.println("HOT!!!");
        AliyunIoTSDK::send("Abnormal", ++abnormal);//异常体温人数上报
        digitalWrite(Buzzer, LOW);
        delay(1000);
      }
      else if(bodyTemp < 37){
        Serial.println("normal");
        AliyunIoTSDK::send("Normal", ++normal);//正常体温人数上报
      }
      break;
    }
  }
  
  //判断是否关门（人员是否已进入）
  if(sig == 1 && echo() > 30){
      close_the_door();sig = 0;
      Serial.println("4.2门关了");
  }
  delay(200);
}

// 初始化 wifi 连接
void wifiInit(const char *ssid, const char *passphrase)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passphrase);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
    Serial.println("Connected to AP");
}

//红外测温
float temperature() {
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
  Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
  Serial.println();
  float c = mlx.readObjectTempC();
  return c;
}

//超声波测距功能性函数
float echo() {
  float cm; //距离变量
  float t; //中间值
  //给Trig发送一个低高低的短时间脉冲,触发测距
  digitalWrite(Trig, LOW); //给Trig发送一个低电平
  delayMicroseconds(2);    //等待 2微妙
  digitalWrite(Trig,HIGH); //给Trig发送一个高电平
  delayMicroseconds(10);    //等待 10微妙
  digitalWrite(Trig, LOW); //给Trig发送一个低电平
  
  t = float(pulseIn(Echo, HIGH)); //存储回波等待时间,
  //pulseIn函数会等待引脚变为HIGH,开始计算时间,再等待变为LOW并停止计时
  //返回脉冲的长度
  
  //声速是:340m/1s 换算成 34000cm / 1000000μs => 34 / 1000
  //因为发送到接收,实际是相同距离走了2回,所以要除以2
  //距离(厘米)  =  (回波时间 * (34 / 1000)) / 2
  //简化后的计算公式为 (回波时间 * 17)/ 1000
  cm = (t * 17 )/1000; //把回波时间换算成cm
 
  Serial.print("Echo =");
  Serial.print(t);//串口输出等待时间的原始数据
  Serial.print(" | | Distance = ");
  Serial.print(cm);//串口输出距离换算成cm的结果
  Serial.println("cm");
  return cm;
}

void open_the_door()  {//舵机开门

  myServo.write(100);
}
void close_the_door() { //舵机关门

  myServo.write(0);
}
void servo_init() { //舵机初始化

  myServo.write(10);
}
