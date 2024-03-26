#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>//https客户端连接库
#include <Base64.h>
//#include <U8g2lib.h>
#include <esp_task_wdt.h>
#include <SPI.h>
#include <SD.h>
#include <errno.h>
// #ifdef U8X8_HAVE_HW_SPI
// #include <SPI.h>
// #endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif
/*ESP32环境下*/

//设置gpio口
#define key 15//ESP8266:0 / ESP32:15
#define ADC 39 //ESP32:39 /ESP9266:A0
#define LED_BUILTIN 2//ESP8266 LED_BUILTIN ESP32:2     //SCL=5,SDA=4
#define speaker 4
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//常量设置
String token="24.2dd8f242f4f448e4febbc3a81800db2f.2592000.1711328785.282335-45702264",payload = "";
const char* adc_temp="/adc_8k.pcm",*pcm_temp="/pcm.txt";
/***********标志位设置************/
uint8_t adc_start_flag=0,adc_complete_flag=0;       //adc采样开始标志
/*容器设置*/
uint32_t time1=0,time2=0;int n_recording=0;//caiyangshuju he shijian
/******函数声明******/
String ch_to_en(const char* filename);                       //中文语音to英文文本
//void draw(String text);
void ADC_Sampling(const char* filename);
void TO_connect_internet();
void encodeFileToBase64(const char* inputFile, const char* outputFile);
void setup()
{
    Serial.begin(115200);//串口设置波特率
    //引脚初始化 按键：上拉输入 led灯、输出功放：输出 adc采样口：输入
    pinMode(key,INPUT_PULLUP);//pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(speaker,OUTPUT);
    pinMode(ADC, INPUT);
    WiFi.mode(WIFI_OFF);//先关闭WIFI功能避免影响采样数据
    // u8g2.begin();
    // u8g2.enableUTF8Print();//开启UTF-8字符打印
    //digitalWrite(LED_BUILTIN,0);//关灯
    Serial.println("Initializing SD card...");
    while (!SD.begin()) {//初始化SD卡
      Serial.println("Card failed, or not present");delay(500);
    }
    Serial.println("card initialized.");
}

void loop()
{
  ADC_Sampling(adc_temp);
  Serial.println("开始编码");
  encodeFileToBase64(adc_temp,pcm_temp);
  Serial.println("开始上传给网站");
  ch_to_en(pcm_temp); 
}

String ch_to_en(const char* filename)    
{
    File File1;
    String url = "https://aip.baidubce.com/rpc/2.0/mt/v2/speech-translation?access_token="+token;Serial.print("url:");  Serial.println(url);
    // /*语音翻译url：https://aip.baidubce.com/rpc/2.0/mt/v2/speech-translation
    // 文本翻译：https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1
    String test = "{\"from\": \"zh\",\"to\": \"en\",\"format\": \"pcm\",\"voice\":\"";//+base64::encode((uint8_t *)adc_temp,sizeof(adc_temp))+"\"}";
    // String request = "POST " + url + " HTTP/1.1\r\n" +
    //                 "Host: aip.baidubce.com\r\n" +
    //                 "Connection: close\r\n" +
    //                 "Content-Type: application/json\r\n" +
    //                 "Content-Length: " + test.length() + "\r\n" +
    //                 "\r\n"+test;
    // client.print(request);  // 发送请求
    TO_connect_internet();//连接WiFi
    WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象
    // 设置时间
    //setClock();
    // 设置根证书
    //BearSSL::X509List cert(rootCACertificate);
    // client.setCACert(rootCACertificate);
    // client.setTimeout(15000);  
    // 禁用证书验证（绕过证书验证）
    client.setInsecure();
    delay(1000);
    
    Serial.print("begin to connect your host\n");
    if (!client.connect("aip.baidubce.com", 443)) {  // 建立与服务器的连接
        Serial.println("Connection failed");
        Serial.println(client.getWriteError());
        return "";
    }
    Serial.println("connect successed");
    File1 = SD.open(filename,"r");
    if (File1) {
      Serial.println("成功打开 adc.txt 文件:");
      int fileSize = File1.size();//获取文件大小
      client.print("POST ");
      client.print(url.c_str());
      client.print(" HTTP/1.1\r\nHost: aip.baidubce.com\r\nConnection: close\r\nAccept: application/json\r\nContent-Type: application/json\r\nContent-Length: ");
      client.print(String(test.length()+fileSize+2).c_str());
      client.print("\r\n\r\n");
      client.print(test.c_str());
      // 从文件中读取内容
      while (File1.available()) {
        char buffer[500]; // 优化为较小的缓冲区大小，根据需求调整
        size_t len = File1.readBytes(buffer, sizeof(buffer));
        // 将从文件中读取的数据直接写入客户端
        client.write((const uint8_t*)buffer, len);
      }
      client.print("\"}");
    } else {
      // 如果文件打开失败，打印错误原因
      Serial.print("打开文件失败，错误原因：");
      Serial.println(errno);
      return "";
    }
    // 关闭文件
    File1.close();
    //Serial.println(request);
    delay(1000);  // 等待一段时间以确保服务器发送所有数据
    if (!client.connected()) {
        // 连接已关闭，退出循环
        Serial.print("client is closed");
        return "";
    }
    while (client.available()) {
        String line = client.readStringUntil('\r');  // 读取从服务器返回的数据
        Serial.println(line);
        int startPos = line.indexOf("\"target\":\"");
        if (startPos != -1) 
        {
          startPos += 10; // 跳过 "\"dst\":\"" 的长度
          int endPos = line.indexOf("\"", startPos);
          if (endPos != -1) {
            String dst = line.substring(startPos, endPos);//draw(dst);
            Serial.println("Extracted dst: " + dst);
            //Serial.printf("draw successed?%s",dst.c_str());
            payload = dst;
          }
        }
    }
    //delay(10);
    client.stop();  // 关闭与服务器的连接
    Serial.printf("Recognition complete\r\n");
    return payload;
}
void ADC_Sampling(const char* filename)
{
  while(digitalRead(key)) //等待按键按下
    esp_task_wdt_reset(); 
  SD.remove(filename);//移除上一次录入的数据
  File File_test = SD.open(filename,FILE_WRITE);
  if (File_test) {
     Serial.print("打开/创建文件成功");
  } 
  else {
      // 如果文件打开失败，打印错误原因
      Serial.print("打开文件失败，错误原因：");
      Serial.println(errno);
      return;
  }
  time1=0,time2=0,n_recording=0;
  time1=micros();
  while(!digitalRead(key))
  {
    uint16_t adc_value = analogRead(ADC);
    File_test.write((uint8_t*)&adc_value, sizeof(adc_value));
    n_recording++;
  }
  time2=micros()-time1; 
  File_test.close();
  Serial.printf("\nlen:%d,stime:%fHZ\n",n_recording,n_recording/(time2/1000000.00f));
}
void TO_connect_internet()
{
  //建立WiFi连接
    //WiFi.setAutoReconnect(true);//设置断开连接后重连
    //通过addAp函数存储  WiFi名称       WiFi密码
    Serial.println("kaishi");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);// 启用自动重连功能
    if(WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin("JUNMOXIE", "ABCD1949");
      while (WiFi.status() != WL_CONNECTED) {//等待WiFi连接
        delay(500);
        Serial.print(".");
      }
      Serial.print("\n");
      Serial.println("Connect Succesed");
    }
}
void play_the_adc(const char* filename)
{
  Serial.println("player!");
  // 打开文件
  File file = SD.open(filename,"r");
  if (!file) {
    Serial.println("打开文件失败");
    return;
  }
  // 获取文件大小
  int fileSize = file.size();
  Serial.print("文件大小: ");
  Serial.print(fileSize);
  Serial.println(" 字节");
  for(int i=0;i<fileSize;i++)
  {
    analogWrite(speaker,file.read());delayMicroseconds(125); // 延时125微秒
  }
  analogWrite(speaker,0);
  // 关闭文件
  file.close();
  //analogWrite(speaker, 0);
  Serial.println("end!");
}
void encodeFileToBase64(const char* inputFile, const char* outputFile)
{
  // 打开输入文件
  File inFile = SD.open(inputFile, FILE_READ);
  if (!inFile) {
    Serial.println("无法打开输入文件");
    return;
  }
  // 创建输出文件
  File outFile = SD.open(outputFile, FILE_WRITE);
  if (!outFile) {
    Serial.println("无法创建输出文件");
    inFile.close();
    return;
  }
  // 逐块读取 PCM 数据并进行 Base64 编码
  while (inFile.available()) {
    String line = inFile.readString();
    // 编码数据并写入输出文件
    String encodedData = base64::encode(line);
    outFile.print(encodedData);
  }
  // 关闭文件
  inFile.close();
  outFile.close();
  Serial.println("文件编码完成，按下按键上传给网站");
}