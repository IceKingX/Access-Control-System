/******************************
 *   门禁系统（室外温湿度检测）   *
 *   V2.0       2021-6-27     *
 *****************************/
//#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>     //RC522读卡器
#include <Servo.h>        //舵机
#include "DHT.h"
#include <Adafruit_GFX.h>  //图形库
#include <Adafruit_SSD1306.h> //专门用于SSD1306芯片的驱动库

#define RST_PIN      27        // 读卡机重置脚位
#define SS_PIN       26        // 芯片选择脚位

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DuoPIN   13    //舵机引脚

// DHT11
#define DHTPIN 32
#define DHTTYPE DHT11   // DHT 11

float temperature = 0;
float humidity = 0;
long lastMsg = 0;

const char* ssid = "IOT";
const char* password = "201820040507";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.181.2";

WiFiClient espClient;
PubSubClient client(espClient);//初始化客户端实例
DHT dht(DHTPIN, DHTTYPE);//定义DHT11温湿度传感器
Servo myDuoJi;  //定义舵机myDuoJi

struct RFIDTag {    // 定义结构体
   byte uid[4];
   char *name;
};

struct RFIDTag tags[] = {  // 初始化UID-Key资料
   {{0x19, 0x16, 0x6F, 0xE4}, "Right Key1"},
   {{0xF2, 0xED, 0x96, 0x3B}, "Right Key2"},
};
 
byte totalTags = sizeof(tags) / sizeof(RFIDTag);
 
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件

// 连接到I2C (SDA, SCL pins)的SSD1306显示器声明
Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT, &Wire, -1);

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/door") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      display.setTextSize(2);
      display.setTextColor(WHITE); 
      display.setCursor(0,30);
      display.printf("WELCOME!");
      //myDuoJi.attach(DuoPIN);
      myDuoJi.write(80);
      //myDuoJi.detach();
      display.display();
      delay(1500); // wait 1500ms
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      display.setTextSize(2);
      display.setTextColor(WHITE); 
      display.setCursor(0,30);  
      display.printf("CLOSE!");  // 显示关锁
      myDuoJi.write(175);
      display.display();
      delay(1500); // wait 1500ms
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client-1")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/door");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void AC_system(){
  myDuoJi.attach(DuoPIN);
  display.clearDisplay();     
  display.setTextSize(1);
  display.setCursor(0,0);  
  display.println("Please verify~ ");
  display.display();
  // 确认key的正确性
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
    byte idSize = mfrc522.uid.size;   // 取得UID的长度
    bool foundTag = false;            // 是否匹配，预设为「否」。
      
    for (byte i=0; i<totalTags; i++) {
      if (memcmp(tags[i].uid, id, idSize) == 0 && myDuoJi.read()== 175) {
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,20);  
        display.printf(tags[i].name);  // 显示Right Key
        display.setCursor(0,50);
        display.printf("OPEN!");
        //myDuoJi.attach(DuoPIN);
        myDuoJi.write(80);
        //myDuoJi.detach();
        display.display();
        delay(1500); // wait 1500ms
          
        foundTag = true;  // 设定成「匹配！」
        break;            // 退出循还
      }
   
      if (memcmp(tags[i].uid, id, idSize) == 0 && myDuoJi.read() == 80) {
          display.setTextSize(2);
          display.setTextColor(WHITE); 
          display.setCursor(0,20);  
          display.printf("CLOSE!");  // 显示关锁
          myDuoJi.write(175);
          display.display();
          delay(1500); // wait 1500ms
            
          foundTag = true;  // 设定成「匹配！」
          break;            // 退出循还
      }
    }
 
    if (!foundTag) {    // 不匹配则显示"Wrong Key!"。
      display.setTextSize(2); 
      display.setCursor(0,20);
      display.printf("Wrong Key!"); //显示错误Key
      myDuoJi.write(175);
      display.display();
      delay(1500); // wait 1500ms
    }
 
    mfrc522.PICC_HaltA();  // 让卡片进入停止模式      
  }
}

void setup()
{
  Serial.begin(115200); // starts the serial port at 9600

  SPI.begin();
  mfrc522.PCD_Init();   // 初始化MFRC522读卡器

  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)) {    
    Serial.println(F("SSD1306 allocation failed"));    
    for(;;);  }
  delay(2000);  
  
  display.clearDisplay();  
  display.setTextColor(WHITE);

  myDuoJi.attach(DuoPIN);
  myDuoJi.write(175);  //舵机初始值设为为175度（关闭状态）

  // dht11 begin
  dht.begin();
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

 void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  AC_system();

  long now = millis();
  if (now - lastMsg > 1000) {//一秒发布一次数据
    lastMsg = now;
    
    if(myDuoJi.read() == 80){
      client.publish("esp32/door_status", "on");
    }
    else if(myDuoJi.read() == 175 ){
      client.publish("esp32/door_status", "off");
    }
    
    // Temperature in Celsius
    temperature = dht.readTemperature();   
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/outTemperature", tempString);

    humidity = dht.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/outHumidity", humString);
  }

  //传递舵机状态给串口
  //Serial.println(myDuoJi.read());
}
