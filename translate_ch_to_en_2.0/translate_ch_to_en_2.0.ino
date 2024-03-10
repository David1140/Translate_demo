#include <Arduino.h>
#include "base64.h"
#include <WiFi.h>
#include "HTTPClient.h"
#include <WiFiClientSecure.h>//https客户端连接库
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <esp_task_wdt.h>
#include <SPI.h>
#include <SD.h>
#include <errno.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define MAX_LEN 16000

//设置gpio口
#define key 15
#define ADC 39
#define LED_BUILTIN 2//SCL=5,SDA=4
#define speaker 4
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//常量设置
String token="24.2dd8f242f4f448e4febbc3a81800db2f.2592000.1711328785.282335-45702264",payload = "";
//通过get_token函数获得的token，以及用于保存返回信息的全局变量
// String client_id_voice = "w1RXUe0iP9IHz8j3WIwtpnls";
// String client_secret_voice = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
// String client_id_translate = "kM5AAWhILGSgjt6axCv3iQVw";
// String client_secret_translate = "WOM4Y4kGxgPoie4YtDDijRZ5eqpeEcGu";
uint32_t time1=0,time2=0;int num=0,n=0;//caiyangshuju he shijian
/***********标志位设置************/
uint8_t adc_start_flag=0,adc_complete_flag=0;       //adc采样开始标志
/*容器设置*/
File File1;
volatile uint16_t adc_temp[MAX_LEN];
hw_timer_t * timer = NULL;//定时器
/******函数声明******/
//String gain_token(String api_key,String seceretkey);   //获取token
//String ch_to_ev();                                    //中文语音to英文文本
//void entospeaker();                                   //英文文本转音频编码输出
//void draw(String text);
void setClock();
void ADC_Sampling(volatile uint16_t *adc_temp);
void TO_connect_internet();
void play_the_adc(volatile uint16_t *adc_temp);
//void MyTone(int frequency, int duration);
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR TimerHandler()
{
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  if(adc_start_flag==1)
    {
      if(num==0)
      {time1=micros();}
      adc_temp[num++]=analogRead(ADC);
      if(num>=MAX_LEN)
      {
        adc_complete_flag = 1;
        adc_start_flag=0;
        time2 = micros() - time1;
      }
    }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
    Serial.begin(115200);
    pinMode(key,INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);pinMode(speaker,OUTPUT);
    //pinMode(ADC, INPUT);
    WiFi.mode(WIFI_OFF);//先关闭WIFI功能避免影响采样数据
    // u8g2.begin();
    // u8g2.enableUTF8Print();
    digitalWrite(LED_BUILTIN,0);
    Serial.println("Initializing SD card...");
    while (!SD.begin()) {
      Serial.println("Card failed, or not present");
    }
    Serial.println("card initialized.");
    
}

void loop()
{
  while(digitalRead(key)) {
    esp_task_wdt_reset(); 
      //ets_delay_us(10);
  }
  //ADC_Sampling(adc_temp);
  //play_the_adc(adc_temp);
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
  const char* rootCACertificate PROGMEM= \
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\n"\
  "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\n"\
  "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\n"\
  "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\n"\
  "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"\
  "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\n"\
  "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\n"\
  "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\n"\
  "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\n"\
  "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\n"\
  "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\n"\
  "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\n"\
  "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\n"\
  "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\n"\
  "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\n"\
  "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\n"\
  "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\n"\
  "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\n"\
  "WD9f\n"\
  "-----END CERTIFICATE-----\n";

    WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象
    // 设置时间
    setClock();
    // 设置根证书
    client.setCACert(rootCACertificate);
    client.setTimeout(15000);  
    delay(1000);
    
    Serial.print("begin to connect your host\n");
    if (!client.connect("aip.baidubce.com", 443)) {  // 建立与服务器的连接
        Serial.println("Connection failed");
        Serial.println(client.getWriteError());
        return ;
    }
    Serial.println("connect successed");
    File1 = SD.open("/adc.txt","r");
    if (File1) {
      Serial.println("成功打开 adc.txt 文件:");
      
      client.print("POST ");
      client.print(url);
      client.print(" HTTP/1.1\r\nHost: aip.baidubce.com\r\nConnection: close\r\nAccept: application/json\r\nContent-Type: application/json\r\nContent-Length: ");
      client.print(String(test.length()+83262));
      client.print("\r\n\r\n");
      client.print(test);
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
      return;
    }
    // 关闭文件
    File1.close();
    //Serial.println(request);
    delay(10000);  // 等待一段时间以确保服务器发送所有数据
    if (!client.connected()) {
        // 连接已关闭，退出循环
        Serial.print("client is closed");
        return;
    }
    while (client.available()) {
        String line = client.readStringUntil('\r');  // 读取从服务器返回的数据
        Serial.println(line);
        int startPos = line.indexOf("\"target\":\"");
        if (startPos != -1) 
        {
          startPos += 7; // 跳过 "\"dst\":\"" 的长度
          int endPos = line.indexOf("\"", startPos);
          if (endPos != -1) {
            String dst = line.substring(startPos, endPos);//draw(dst);
            Serial.println("Extracted dst: " + dst);
            Serial.printf("draw successed?%s",dst.c_str());
            payload = dst;
          }
          digitalWrite(LED_BUILTIN,1);
        }
    }
    //delay(10);
    client.stop();  // 关闭与服务器的连接
    Serial.printf("Recognition complete\r\n");
}
// 获取网络时间，该时间信息用于证书认证
void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();//yield() 函数的作用是让出一部分处理时间给其他任务或事件，以保持系统的响应性和平滑运行。
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

void ADC_Sampling(volatile uint16_t *adc_temp)
{
  WiFi.disconnect(true);
  Serial.print("键值状况为：");
  Serial.println(digitalRead(key));
  while(digitalRead(key))
    ;//ESP.wdtFeed(); 
  timer = timerBegin(0, 80, true);    //  80M的时钟 80分频 1M
  timerAlarmWrite(timer, 125, true);  //  1M  计125个数进中断  8K
  timerAttachInterrupt(timer, &TimerHandler, true);
  timerAlarmEnable(timer);
  timerStop(timer);   //先暂停
  adc_start_flag=1,adc_complete_flag=0,num=0;
  Serial.println("open led");digitalWrite(LED_BUILTIN,1);timerStart(timer);
  while(adc_complete_flag==0) {
    esp_task_wdt_reset(); 
      //ets_delay_us(10);
  }
  timerStop(timer);adc_start_flag=0,adc_complete_flag=0;
  Serial.printf("\nnum:%d,time2:%d,stime:%fHZ\n",num,time2,num/(time2/1000000.00f));num=0;
  digitalWrite(LED_BUILTIN,0);//guan灯
  for(int i=0;i<MAX_LEN;i++)
  {
    Serial.printf("%d ",adc_temp[i]*=(0.0625));
  }Serial.println(" ");
}
void TO_connect_internet()
{
  //建立WiFi连接
    //WiFi.setAutoReconnect(true);//设置断开连接后重连
    //通过addAp函数存储  WiFi名称       WiFi密码
    Serial.println("kaishi");
    WiFi.mode(WIFI_STA);// 启用自动重连功能
    WiFi.setAutoReconnect(true);
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
void play_the_adc(volatile uint16_t *adc_temp)
{
  Serial.println("player!");
  for(int i=0;i<MAX_LEN;i++)
  {
    //Serial.println(adc_temp[i]);
    analogWrite(speaker, adc_temp[i]);delayMicroseconds(125); // 延时125微秒
  }
  //analogWrite(speaker, 0);
  Serial.println("end!");
}
