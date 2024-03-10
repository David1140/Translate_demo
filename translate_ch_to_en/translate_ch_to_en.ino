#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>//http客户端链接
#include <WiFiClientSecure.h>//https客户端连接库
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include "base64.h"
#include "ESP8266TimerInterrupt.h"
#include <SPI.h>
#include <SD.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define MAX_LEN 16000
#define USING_TIM_DIV1                true          // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              false            // for longest timer but least accurate
#define TIMER_FREQ_HZ        20000
//设置gpio口
#define key 0
#define ADC A0
#define led LED_BUILTIN//SCL=5,SDA=4
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//常量设置
String token="24.2dd8f242f4f448e4febbc3a81800db2f.2592000.1711328785.282335-45702264",payload,encodedData = "";;
//通过get_token函数获得的token，以及用于保存返回信息的全局变量
String client_id_voice = "w1RXUe0iP9IHz8j3WIwtpnls";
String client_secret_voice = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
String client_id_translate = "kM5AAWhILGSgjt6axCv3iQVw";
String client_secret_translate = "WOM4Y4kGxgPoie4YtDDijRZ5eqpeEcGu";
uint32_t time1=0,time2=0;int num=0,n=0;//caiyangshuju he shijian
/***********标志位设置************/
uint8_t adc_start_flag=0,adc_complete_flag=0;       //adc采样开始标志
/*容器设置*/
volatile uint16_t adc_temp[MAX_LEN];
ESP8266WiFiMulti WiFiMulti;//wifi多种账号数据容器
File ADCFile;
ESP8266Timer ITimer;
/******函数声明******/
String gain_token(String api_key,String seceretkey);   //获取token
String ch_to_ev();      //中文语音to英文文本
String encodeBuffer();
void entospeaker();     //英文文本转音频编码输出
void draw(String text);
void translate_main();
void IRAM_ATTR TimerHandler()
{
  if(adc_start_flag==1)
    {
      adc_temp[num++]=analogRead(A0);
      if(num==16000)
      {
        adc_complete_flag = 1;
      }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(key,INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ADC, INPUT);
    //WiFi.mode(WIFI_STA);
    WiFi.mode(WIFI_OFF);//先关闭WIFI功能避免影响采样数据

    // u8g2.begin();
    // u8g2.enableUTF8Print();
    digitalWrite(LED_BUILTIN,HIGH);

    //Serial.println("Initializing SD card...");
    // while (!SD.begin(D8)) {
    //   Serial.println("Card failed, or not present");
    // }
    // SD.remove("ADC.txt");
    // Serial.println("card initialized.");
    // ADCFile = SD.open("ADC.txt", FILE_WRITE);// | FILE_READ | O_CREAT
    // if (ADCFile) {
    //   Serial.println("成功打开或创建 ADC 文件");
    // } else {
    //   Serial.println("无法打开或创建 ADC 文件");
    // }
    
}

void loop()
{  
  String text = ch_to_ev();
  //gain_token(client_id_translate,client_secret_translate);//
  //draw(text);
  //delay(1000);
}


String gain_token(String api_key,String seceretkey)   //获取token
{
    HTTPClient http_token;//get_token
    WiFiClient client;
    int httpCode;
    Serial.printf("Connected to %s,IP address:",WiFi.SSID());            // NodeMCU将通过串口监视器输出。
    Serial.println(WiFi.localIP());
    //注意，要把下面网址中的your_apikey和your_secretkey替换成自己的API Key和Secret Key
    String curl = "http://aip.baidubce.com/oauth/2.0/token?client_id="+api_key+"&client_secret="+seceretkey+"&grant_type=client_credentials";
    //Serial.println("http://quan.suning.com/getSysTime.do"); 
    Serial.println(curl);      
    http_token.begin(client,curl);
    // http_token.addHeader("Content-Type", "application/json");
    //http_token.setReuse(true);//设置请求头中的keep-alive
    //http_token.setTimeout(10000);
    httpCode = http_token.GET();
    //httpCode=http_token.sendRequest("POST", "");
    Serial.print("the httpcode is");Serial.println(httpCode);
    if(httpCode > 0) {
        if(httpCode == HTTP_CODE_OK) {
            payload = http_token.getString();
            Serial.println(payload);draw(payload);
        }
    }
    else {
        String error =  http_token.errorToString(httpCode);
        Serial.printf("[HTTP] GET... failed, error: %s\n",error.c_str());
        payload = error;draw(payload);delay(2000);
    }
    http_token.end();
    return payload;
}
String ch_to_ev()
{
    while(digitalRead(key))
      ESP.wdtFeed(); 
    timer1_isr_init();
    timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);  //此处选择div模式，timer触发方式（不要改），timer_loop循环，SINGLE单次
    timer1_write(TIMER_FREQ_HZ);    //修改计数比较器，这里为25000个间隔触发中断，变量用uint32_t
    timer1_attachInterrupt(TimerHandler);timer1_disable();
    adc_start_flag=1,num=0,n=0;
    time1=micros();timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
    while(adc_complete_flag==0) {
      ESP.wdtFeed(); 
        //ets_delay_us(10);
    }timer1_disable();adc_start_flag=0,adc_complete_flag=0;
    time2 = micros() - time1;
    //adc_start_flag = 0; // 置零准备用于下次采集判断
    // ADCFile.printf("}");
    // ADCFile.close();
    Serial.printf("\nnum:%d,time2:%d,stime:%fHZ",num,time2,num/(time2/1000000.00f));
    //str_adc_data.concat(base64::encode((uint8_t *)adc_data,sizeof(adc_data)).c_str());
    encodeBuffer();Serial.println("编码数据"); // 输出编码后的数据
    Serial.println(encodedData); // 输出编码后的数据
    Serial.printf("长度是：%d",encodedData.length()); // 输出编码后的数据
      return encodedData;
      // String test = "{\"from\": \"zh\",\"to\": \"en\",\"format\": \"pcm\",\"voice\":\"";//+str_adc_data+"\"}";
      // const char* fingerprint = "97 42 D5 98 27 D6 22 88 CF 59 C3 FF 75 86 8D D5 D3 12 A0 AF";
      // Serial.print("test:");
      // Serial.println(test);
      // int httpsCode;
      // String url = "https://aip.baidubce.com/rpc/2.0/mt/v2/speech-translation?access_token="+token;Serial.print("url:");
      // /*语音翻译url：https://aip.baidubce.com/rpc/2.0/mt/v2/speech-translation
      // 文本翻译：https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1
      // */
      // Serial.println(url);

      // //建立WiFi连接
      // //WiFi.setAutoReconnect(true);//设置断开连接后重连
      // //通过addAp函数存储  WiFi名称       WiFi密码
      // WiFi.begin("JUNMOXIE","ABCD1949");
      // while (WiFi.status() != WL_CONNECTED) {//等待WiFi连接
      // delay(500);
      // Serial.print(".");}
      // Serial.print("\n");
      // Serial.println("Connect Succesed");

      // WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象
      // client.setFingerprint(fingerprint);  // 设置指纹码
      // Serial.print("begin to connect your host\n");
      // if (!client.connect("aip.baidubce.com", 443)) {  // 建立与服务器的连接
      //     Serial.println("Connection failed");
      //     return "connection failed";
      // }

      // String request = "POST " + url + " HTTP/1.1\r\n" +
      //                 "Host: aip.baidubce.com\r\n" +
      //                 "Connection: close\r\n" +
      //                 "Content-Type: application/json\r\n" +
      //                 "Content-Length: " + encodedData.length() + "\r\n" +
      //                 "\r\n"+test+"\"}";

      // client.print(request);  // 发送请求
      // Serial.println(request);
      // delay(1000);  // 等待一段时间以确保服务器发送所有数据

      // while (client.available()) {
      //     String line = client.readStringUntil('\r');  // 读取从服务器返回的数据
      //     Serial.println(line);
      //     int startPos = line.indexOf("\"dst\":\"");
      //     if (startPos != -1) 
      //     {
      //       startPos += 7; // 跳过 "\"dst\":\"" 的长度
      //       int endPos = line.indexOf("\"", startPos);
      //       if (endPos != -1) {
      //         String dst = line.substring(startPos, endPos);draw(dst);
      //         Serial.println("Extracted dst: " + dst);
      //         Serial.printf("draw successed?%s",dst.c_str());
      //         payload = dst;
      //       }
      //       digitalWrite(LED_BUILTIN,1);
      //       }
      // }
      // //delay(10);
      // client.stop();  // 关闭与服务器的连接
      // Serial.printf("Recognition complete\r\n");
  return payload;
}
void entospeaker()
{
}
void draw(String text)
{
  u8g2.setFont(u8g2_font_wqy15_t_gb2312b);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.println("智能翻译");
  u8g2.setCursor(0, 40);
  u8g2.println(text);
  //Serial.printf("get_token:%s\n",token.c_str());
  u8g2.sendBuffer();
}
void translate_main()
{
   /*循环函数*/
    /*⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅ ⬅         
      ⬇ ➡获取token->获取按键状态->1->进入下一次循环⬆
    0->开始录音->按键状态变零->指示灯亮->时长到达最大值/按键松开->指示灯灭->
    发送翻译post请求->收回应将指定数据显示到oled屏幕上->发送翻译的英文文本结果post请求合成录音
    收到回应然后播放该编码转文件。    
    */
}


String encodeBuffer() {
  encodedData = "";

  // 打开 "ADC.txt" 文件进行读取
  File adcFile = SD.open("ADC.txt", FILE_READ);
  if (!adcFile) {
    Serial.println("无法打开 ADC 文件");
    return encodedData;
  }

  // 逐行读取文件内容并进行 Base64 编码
  while (adcFile.available()) {
    String data = adcFile.readStringUntil(',');
    if (data.endsWith("}")) {
      data.remove(data.length() - 1);
    }
    String encodedChar = base64::encode((const unsigned char*)data.c_str(), data.length());
    encodedData += encodedChar;
  }

  // 关闭文件
  adcFile.close();
  return encodedData;
}