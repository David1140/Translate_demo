#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>//http客户端链接
#include <WiFiClientSecure.h>//https客户端连接库
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include "base64.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
//设置gpio口
#define key 0
#define ADC A0
#define led LED_BUILTIN//SCL=5,SDA=4
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
/***********标志位设置************/
uint8_t adc_start_flag=0;       //adc采样开始标志
uint8_t adc_complete_flag=0;    //adc采样完成标志
bool HttpDateFlag = false;
/*容器设置*/
ESP8266WiFiMulti WiFiMulti;//wifi多种账号数据容器
WiFiClient client;//wifi连接
//hw_timer_t * timer = NULL;
/******函数声明******/
String gain_token(String api_key,String seceretkey);   //获取token
String ch_to_ev(String text);//中文语音to英文文本
void entospeaker();//英文文本转音频编码输出
void draw(String text);
void translate_main();
#define data_len 16000
uint16_t adc_data[data_len];    //16000个数据，8K采样率，即2分钟，录音时间为2分钟，想要实现更长时间的语音识别，就要改这个数组大小
//和下面data_json数组的大小，改大一些。
char data_json[45000];  //用于储存json格式的数据,大一点,JSON编码后数据字节数变成原来的4/3,所以得计算好,避免出现越界
String token="24.597935f6aea202393f00a7b43d15b66b.2592000.1707013148.282335-45702264",payload;
//通过get_token函数获得的token，以及用于保存返回信息的全局变量
String client_id_voice = "w1RXUe0iP9IHz8j3WIwtpnls";
String client_secret_voice = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
String client_id_translate = "kM5AAWhILGSgjt6axCv3iQVw";
String client_secret_translate = "WOM4Y4kGxgPoie4YtDDijRZ5eqpeEcGu";
void setup()
{
    Serial.begin(115200);
    pinMode(key,INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);//设置断开连接后重连
    //通过addAp函数存储  WiFi名称       WiFi密码
    WiFi.begin("JUNMOXIE","ABCD1949");
    while (WiFi.status() != WL_CONNECTED) {//等待WiFi连接
    delay(500);
    Serial.print(".");}
    Serial.println("Connect Succesed");
    u8g2.begin();
    u8g2.enableUTF8Print();
    digitalWrite(LED_BUILTIN,HIGH);
    //gain_token(client_id_translate,client_secret_translate);
    
}

void loop()
{   
  if ((WiFi.status() == WL_CONNECTED)) {
  //gain_token(client_id_translate,client_secret_translate);//
  //translate_main();
  text = ch_to_ev("你好世界");
  }  
  //delay(1000);
}


String gain_token(String api_key,String seceretkey)   //获取token
{
  HTTPClient http_token;//get_token
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
            HttpDateFlag = true;
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
String ch_to_ev(String text)
{
    int key_status = digitalRead(key);
    if(!key_status)
    {
        Serial.println("the key_status is 0");
        digitalWrite(LED_BUILTIN,0);
        //token = gain_token(client_id_voice,client_secret_voice);//获取token
        //Serial.println(token);
        while(digitalRead(key)==0)
        {//开始录音
        digitalWrite(LED_BUILTIN,0);
        //保存数据
        }
        
    //draw(text);//text
  HTTPClient http_cte;//中文转英文
  Serial.printf("Start recognition\r\n\r\n");
  //adc_start_flag=1;	//数据开始采集   
  //timerStart(timer);
  //while(!adc_complete_flag)  //等待数据采集完成{
  //ets_delay_us(10);}
  //timerStop(timer);
  //adc_complete_flag=0;   //采集完成，清标志   
  // memset(data_json,'\0',strlen(data_json));   //将数组清空
  // strcat(data_json,"{\"from\":\"zh\",\"to\":\"en\",\"q\":\"你好世界\"}");
  String test = "{\"from\": \"zh\",\"to\": \"en\",\"q\": \""+text+"\"}";
  const char* fingerprint = "97 42 D5 98 27 D6 22 88 CF 59 C3 FF 75 86 8D D5 D3 12 A0 AF";
  Serial.print("test:");
  Serial.println(test);
  int httpsCode;
  String url = "https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1?access_token="+token;Serial.print("url:");
  Serial.println(url);

  WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象
  client.setFingerprint(fingerprint);  // 设置指纹码
  Serial.print("begin to connect your host");
  if (!client.connect("aip.baidubce.com", 443)) {  // 建立与服务器的连接
      Serial.println("Connection failed");
      return "connection failed";
  }

  String request = "POST " + url + " HTTP/1.1\r\n" +
                  "Host: aip.baidubce.com\r\n" +
                  "Connection: close\r\n" +
                  "Content-Type: application/json\r\n" +
                  "Content-Length: " + String(test.length()) + "\r\n" +
                  "\r\n" +
                  test;

  client.print(request);  // 发送请求
  delay(1000);  // 等待一段时间以确保服务器发送所有数据

  while (client.available()) {
      String line = client.readStringUntil('\r');  // 读取从服务器返回的数据
      //Serial.println(line);
      int startPos = line.indexOf("\"dst\":\"");
      if (startPos != -1) 
      {
        startPos += 7; // 跳过 "\"dst\":\"" 的长度
        int endPos = line.indexOf("\"", startPos);
        if (endPos != -1) {
          String dst = line.substring(startPos, endPos);draw(dst);
          Serial.println("Extracted dst: " + dst);
          Serial.printf("draw successed?%s",dst.c_str());
          payload = dst;
        }
    digitalWrite(LED_BUILTIN,1);
    }
  }
  //delay(10);
  }
  
  client.stop();  // 关闭与服务器的连接
  Serial.printf("Recognition complete\r\n");
  return payload;
}
void entospeaker()
{
    Serial.printf("Start recognition\r\n\r\n");
    digitalWrite(led,HIGH);
    adc_start_flag=1;
    //timerStart(timer);

    // time1=micros();
    while(!adc_complete_flag)  //等待采集完成
    {
      ets_delay_us(10);
    }
    // time2=micros()-time1;
    //timerStop(timer);
    adc_complete_flag=0;        //清标志

    digitalWrite(led,LOW);

    // Serial.printf("time:%d\r\n",time2);  //打印花费时间


    memset(data_json,'\0',strlen(data_json));   //将数组清空
    strcat(data_json,"{");
    strcat(data_json,"\"format\":\"pcm\",");
    strcat(data_json,"\"rate\":8000,");         //采样率    如果采样率改变了，记得修改该值，只有16000、8000两个固定采样率
    strcat(data_json,"\"dev_pid\":1537,");      //中文普通话
    strcat(data_json,"\"channel\":1,");         //单声道
    strcat(data_json,"\"cuid\":\"123456\",");   //识别码    随便打几个字符，但最好唯一
    strcat(data_json,"\"token\":\"XXXXXXXXXXXXXXXXXXXXXXXXXXXX\",");  //token	这里需要修改成自己申请到的token
    strcat(data_json,"\"len\":32000,");         //数据长度  如果传输的数据长度改变了，记得修改该值，该值是ADC采集的数据字节数，不是base64编码后的长度
    strcat(data_json,"\"speech\":\"");
    strcat(data_json,base64::encode((uint8_t *)adc_data,sizeof(adc_data)).c_str());     //base64编码数据
    strcat(data_json,"\"");
    strcat(data_json,"}");
    // Serial.println(data_json);


    int httpCode;
    http_etp.begin(client,"http://vop.baidu.com/server_api");
    http_etp.addHeader("Content-Type","application/json");
    httpCode = http_etp.POST(data_json);

    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
          String payload = http_etp.getString();
          Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http_etp.errorToString(httpCode).c_str());
    }
    http_etp.end();

    while (!digitalRead(key));
    Serial.printf("Recognition complete\r\n");
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