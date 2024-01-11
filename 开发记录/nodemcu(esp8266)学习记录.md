# NodeMCU（ESP8266）学习记录

学习记录期间中出现的问题以及一些零零碎碎的知识点

## 一、Arduino环境配置

Arduino的软件以及一些相关资料的链接

链接：https://pan.baidu.com/s/1ei8ROh6_vMD6R5agajmvvQ?pwd=c6j0 
提取码：c6j0

下载好软件后打开点击：文件/首选项修改项目文件夹地址，语言一堆，（你改成中文后就知道该改什么了），有一个地方要注意一下：

开发板管理地址：https://arduino.esp8266.com/stable/package_esp8266com_index.json（git的网络地址实际可能因为国内限制访问不了，可以自行搜索离线资源更改）

然后就是配置开发板管理，库管理，需要根据你的具体需求下载，可以网络搜一下。（esp的芯片arduino是默认不支持的，我上面的链接有离线资源）

## 二、esp8266的一些api

\#include <Arduino.h>//Arduino基本库（串口输出）

#include <ESP8266WiFi.h>//ESP8266基本库用于连接wifi

\#include <ESP8266WiFiMulti.h>//wifi容器保存的一个库

\#include <ESP8266HTTPClient.h>//http客户端链接

\#include <WiFiClientSecure.h>//https客户端连接库

\#include <WiFiClient.h>//tcp连接

\#include <ArduinoJson.h>//Arduino的json格式文件处理

\#include <U8g2lib.h>//处理oled屏幕的显示

\#include "base64.h"//base64编码



\#ifdef U8X8_HAVE_HW_SPI

\#include <SPI.h>

\#endif

\#ifdef U8X8_HAVE_HW_I2C

\#include <Wire.h>

\#endif

//设置gpio口

\#define key 0

\#define ADC A0

\#define led LED_BUILTIN//SCL=5,SDA=4

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display



ESP8266WiFiMulti WiFiMulti;//wifi多种账号数据容器

WiFiClient client;//wifi连接

Ticker ticker;//定时器



void setup()//初始化设置函数

{

​    Serial.begin(115200);//串口比特率设置

​    pinMode(key,INPUT_PULLUP);//引脚模式设置

​    WiFi.mode(WIFI_STA);//wifi模式设置

​    WiFi.setAutoReconnect(true);//设置断开连接后重连

​    //通过addAp函数存储  WiFi名称       WiFi密码

​    WiFi.begin("JUNMOXIE","ABCD1949");//添加wifi信息

​    while (WiFi.status() != WL_CONNECTED) {//等待WiFi连接

​    delay(500);//延时函数

​    Serial.print(".");}/串口输出

​    Serial.println("Connect Succesed");//同上

​    u8g2.begin();//u8g2库的初始化

​    u8g2.enableUTF8Print();//开启u8g2的utf-8格式输出

​    digitalWrite(LED_BUILTIN,HIGH);//数字电平输出

​    //gain_token(client_id_translate,client_secret_translate);

}



void loop()//程序循环函数

{   

  if ((WiFi.status() == WL_CONNECTED)) {

  //gain_token(client_id_translate,client_secret_translate);//

  //translate_main();

  text = ch_to_ev("你好世界");

  }  

  //delay(1000);

}





{

  HTTPClient http_token;//创建 HTTPClient 对象

​    Serial.printf("Connected to %s,IP address:",WiFi.SSID());            // NodeMCU将通过串口监视器输出。

​    Serial.println(WiFi.localIP());//输出wifi接入本地的ip

​    //注意，要把下面网址中的your_apikey和your_secretkey替换成自己的API Key和Secret Key

​    String curl = "http://aip.baidubce.com/oauth/2.0/token?client_id="+api_key+"&client_secret="+seceretkey+"&grant_type=client_credentials";

//组合我的curl

​    http_token.begin(client,curl);建立http连接

​    // http_token.addHeader("Content-Type", "application/json");//设置请求头中的一些参数

​    //http_token.setReuse(true);//设置请求头中的keep-alive

​    //http_token.setTimeout(10000);//设置连接超时时间

​    httpCode = http_token.GET();//get方法获取连接网站回应

​    //httpCode=http_token.sendRequest("POST", "");//发送post请求

​    Serial.print("the httpcode is");Serial.println(httpCode);

​    if(httpCode > 0) {

​        if(httpCode == HTTP_CODE_OK) {

​            payload = http_token.getString();

​            Serial.println(payload);draw(payload);

​            HttpDateFlag = true;

​        }

​    }

​    else {

​        String error =  http_token.errorToString(httpCode);

​        Serial.printf("[HTTP] GET... failed, error: %s\n",error.c_str());

​        payload = error;draw(payload);delay(2000);

​    }

​    http_token.end();

​    return payload;

}

String ch_to_ev(String text)

{

​    int key_status = digitalRead(key);//是一个 Arduino 函数，用于读取数字输入引脚的电平状态

​    if(!key_status)

​    {

​        Serial.println("the key_status is 0");

​        digitalWrite(LED_BUILTIN,0);

​        //token = gain_token(client_id_voice,client_secret_voice);//获取token

​        //Serial.println(token);

​        while(digitalRead(key)==0)

​        {//开始录音

​        digitalWrite(LED_BUILTIN,0);

​        //保存数据

​        }

​        

​    //draw(text);//text

  HTTPClient http_cte;//中文转英文

  Serial.printf("Start recognition\r\n\r\n");

  //adc_start_flag=1; //数据开始采集   

  //timerStart(timer);

  //while(!adc_complete_flag)  //等待数据采集完成{

  //ets_delay_us(10);}

  //timerStop(timer);

  //adc_complete_flag=0;   //采集完成，清标志   

  // memset(data_json,'\0',strlen(data_json));   //将数组清空

  // strcat(data_json,"{\"from\":\"zh\",\"to\":\"en\",\"q\":\"你好世界\"}");

  String test = "{\"from\": \"zh\",\"to\": \"en\",\"q\": \""+text+"\"}";

  const char* fingerprint = "97 42 D5 98 27 D6 22 88 CF 59 C3 FF 75 86 8D D5 D3 12 A0 AF";//

  Serial.print("test:");

  Serial.println(test);

  int httpsCode;

  String url = "https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1?access_token="+token;Serial.print("url:");

  Serial.println(url);



  WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象

  client.setFingerprint(fingerprint);  // 设置指纹码

  Serial.print("begin to connect your host");

  if (!client.connect("aip.baidubce.com", 443)) {  // 建立与服务器的https连接

​      Serial.println("Connection failed");

​      return "connection failed";

  }



  String request = "POST " + url + " HTTP/1.1\r\n" +

​                  "Host: aip.baidubce.com\r\n" +

​                  "Connection: close\r\n" +

​                  "Content-Type: application/json\r\n" +

​                  "Content-Length: " + String(test.length()) + "\r\n" +

​                  "\r\n" +

​                  test;



  client.print(request);  // 发送请求

  delay(1000);  // 等待一段时间以确保服务器发送所有数据



  while (client.available()) {

​      String line = client.readStringUntil('\r');  // 读取从服务器返回的数据

​      //Serial.println(line);

​      int startPos = line.indexOf("\"dst\":\"");查找第一个出现 `"dst":"` 子串的位置，并将该位置存储

​      if (startPos != -1) 

​      {

​        startPos += 7; // 跳过 "\"dst\":\"" 的长度

​        int endPos = line.indexOf("\"", startPos);

​        if (endPos != -1) {

​          String dst = line.substring(startPos, endPos);draw(dst);截取起始位置为 `startPos`，结束位置为 `endPos`

​          

## 三、记录一些问题

Curl（全称为“Client URL”）的名称源自于其主要的用途：通过URL与服务器进行通信。URL（Uniform Resource Locator）是用于定位和访问互联网资源的地址，包括网页、图像、文件等。

Curl最初是由瑞典程序员Daniel Stenberg开发的，他选择了这个名称来表示工具的主要功能，即作为客户端与特定URL进行通信。通过Curl，用户可以使用命令行界面或库来执行各种网络操作，如发送HTTP请求、下载文件、上传数据等。

虽然Curl的名称可能会导致一些误解，让人以为它只能处理URL，但实际上它支持多种网络协议，不仅限于HTTP或HTTPS。无论是FTP、SMTP、POP3、IMAP还是其他协议，Curl都可以通过相应的URL来与服务器进行通信。

因此，尽管Curl的名称可能有点具有迷惑性，但它实际上是一个功能强大、通用的网络工具，可以与各种网络协议进行交互，而不仅仅局限于处理URL。

http/https连接不同，https连接还要检验指纹或者安全证书等。