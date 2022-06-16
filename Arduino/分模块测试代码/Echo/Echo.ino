#define Buzzer 23      //蜂鸣器
#define Trig 5 //引脚Tring 连接 IO 5
#define Echo 18 //引脚Echo 连接 IO 18
float cm; //距离变量
float temp; //中间值

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(Buzzer,OUTPUT);//设置数字 IO 脚模式，OUTPUT 为输出   
  digitalWrite(Buzzer,HIGH);//初始为不响
}
 
void loop() {
  
  //给Trig发送一个低高低的短时间脉冲,触发测距
  digitalWrite(Trig, LOW); //给Trig发送一个低电平
  delayMicroseconds(2);    //等待 2微妙
  digitalWrite(Trig,HIGH); //给Trig发送一个高电平
  delayMicroseconds(10);    //等待 10微妙
  digitalWrite(Trig, LOW); //给Trig发送一个低电平
  
  temp = float(pulseIn(Echo, HIGH)); //存储回波等待时间,
  //pulseIn函数会等待引脚变为HIGH,开始计算时间,再等待变为LOW并停止计时
  //返回脉冲的长度
  
  //声速是:340m/1s 换算成 34000cm / 1000000μs => 34 / 1000
  //因为发送到接收,实际是相同距离走了2回,所以要除以2
  //距离(厘米)  =  (回波时间 * (34 / 1000)) / 2
  //简化后的计算公式为 (回波时间 * 17)/ 1000
  
  cm = (temp * 17 )/1000; //把回波时间换算成cm
 
      
  Serial.print("Echo =");
  Serial.print(temp);//串口输出等待时间的原始数据
  Serial.print(" | | Distance = ");
  Serial.print(cm);//串口输出距离换算成cm的结果
  Serial.println("cm");

  //测试用
  if(cm < 4){
    digitalWrite(Buzzer,LOW);
  }
  else{
    digitalWrite(Buzzer,HIGH);
  }
  delay(1000);
  
/*
  //如果锁的状态为关且超声波测距的大小返回值大于5，则判断为箱门没关好，触发警报
  if(receive() == 175 && cm > 5){
    digitalWrite(Buzzer,LOW);
  }
  else{
    digitalWrite(Buzzer,HIGH);
  }
  Serial.println(receive());
  delay(1000);
 */
}
