#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
//#include <WiFiClientSecure.h>//https客户端连接库
//#include <WiFiClient.h>
#include <Base64.h>
#include <U8g2lib.h>
#include <esp_task_wdt.h>
#include <SPI.h>
#include <SD.h>
#include <errno.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #ifdef U8X8_HAVE_HW_SPI
// #include <SPI.h>
// #endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
/*ESP32环境下*/
#define OLED_WIDTH 127
#define OLED_HEIGHT 63
//设置gpio口
#define key 15//ESP8266:0 / ESP32:15
#define ADC 39 //ESP32:39 /ESP9266:A0
#define LED_BUILTIN 2//ESP8266 LED_BUILTIN ESP32:2     //SCL=5,SDA=4
#define speaker 13
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // All Boards without Reset of the Display
//常量设置
String token="24.26f61038c317c0684d993b71686651cb.2592000.1713598074.282335-45702264",payload="";
//中文语音识别token,payload为显示在屏幕上的内容用互斥锁保护
const char* adc_temp="/adc_8k.pcm",*pcm_temp="/pcm.txt";
//data_info为识别为中文的json数据体部分
String data_info = "{\"format\":\"pcm\",\"rate\":8000,\"dev_pid\":1537,\"channel\":1,\"cuid\":\"N0N6zV45wOYTQJHQNgbUvYtH38hvE94c\",\"token\":\"24.0d48d0e41557fb5a10b48a6ff92a2f09.2592000.1712891274.282335-44116513\",\"len\":";
/***********标志位设置************/
uint8_t adc_start_flag=0,adc_complete_flag=0;       //adc采样开始标志
SemaphoreHandle_t xMutex;//互斥锁，保护显示在屏幕上的字符串变量
String voiceResult;//语音识别的结果
/*容器设置*/
uint32_t time1=0,time2=0;int n_recording=0;//统计采样数据和时间
/******函数声明******/
//void ADCtoEn();//测试SD卡内数据是否能传输到服务器
int ADC_Sampling(const char* filename);//采样
void TO_connect_internet();//联网
void encodeFileToBase64(const char* inputFile, const char* outputFile);//转码
void play_the_adc(const char* filename, int sampleRate);//播放
void draw(void *arg);//显示
//String gain_token(String api_key,String seceretkey);  //获取token
void task_1(void *arg);
String ch_to_en(String str_voice);//中转英
String Regnization(const char* filename1,const char* filename2); //adc识别
String str_target(String src,String start,String end);//截取字符串                          
//void entospeaker();                                   //英文文本转音频编码输出

void setup()
{
    Serial.begin(115200);//串口设置波特率
    //引脚初始化 按键：上拉输入 led灯、输出功放：输出 adc采样口：输入
    pinMode(key,INPUT_PULLUP);//pinMode(LED_BUILTIN, OUTPUT);
    pinMode(speaker,OUTPUT);
    pinMode(ADC, INPUT);
    //WiFi.mode(WIFI_OFF);//先关闭WIFI功能避免影响采样数据
    TO_connect_internet();
    u8g2.begin();
    u8g2.enableUTF8Print();//开启UTF-8字符打印
    //digitalWrite(LED_BUILTIN,0);//关灯
    Serial.println("Initializing SD card...");
    while (!SD.begin()) {//初始化SD卡
      Serial.println("Card failed, or not present");delay(500);
    }
    Serial.println("card initialized.");
    xMutex = xSemaphoreCreateMutex();
    /* pvTaskCode: 函数指针，指向要运行的任务函数。在这里就是 keyTask 函数。
      pcName: 任务的名称，便于在调试时识别任务。
      usStackDepth: 任务堆栈的大小，以字为单位。任务在执行时会使用堆栈空间来存储临时变量和函数调用信息。
      pvParameters: 传递给任务函数的参数，可以是任意类型的指针，通常用来传递任务需要的数据。
      uxPriority: 任务的优先级，取值范围是 0 到 configMAX_PRIORITIES-1，数值越大，优先级越高。
      pxCreatedTask: 可选参数，用于获取指向新创建任务句柄的指针，如果不需要此句柄，可以传入 NULL。
      xCoreID: 指定任务要运行的核心编号。ESP32 是一个双核处理器，有 0 和 1 两个核心可供选择。 */
      //xTaskCreatePinnedToCore(task_1, "translateTask", 5000, NULL, 2, NULL, 0);
      const char* payloadChar = payload.c_str();
      xTaskCreatePinnedToCore(draw, "DisplayTask", 4096, (void*)payloadChar, 1, NULL, 1);
}

void loop()
{
  //String input = "";
  //for(;;)
  {
    Serial.println("按下按键开始录音");
    while(digitalRead(key)) //等待按键按下
    {esp_task_wdt_reset();}  
    int sampleRate = ADC_Sampling(adc_temp);//采样
    // //play_the_adc(adc_temp,sampleRate);//播放
    encodeFileToBase64(adc_temp,pcm_temp);//编码
    // Serial.println("开始上传给网站");
    // //容器初始化
    String str_voice = Regnization(adc_temp,pcm_temp);//采样数据识别为中文
    /*如果想把串口输入的信息显示到屏幕上把“//@”取消注释，把payload赋值为input*/
    //  if(Serial.available()>0){
    //   char incomingChar = Serial.read(); //读取串口数据
    //    if (incomingChar == '\n') 
    //    { // 如果接收到换行符，说明一行字符串接收完成
    //      Serial.print("Received: ");Serial.println(input); // 将接收到的完整字符串打印出来
    String en_str = ch_to_en(str_voice);
      xSemaphoreTake(xMutex, portMAX_DELAY);
         //翻译，内容同步到显示内容中
      payload =en_str;//input;//
      xSemaphoreGive(xMutex);//input = ""; // 清空缓冲区，准备下一次接收
    //    }
    //    else {input += incomingChar; // 将接收到的字符追加到已有字符串后面
    //   }
    // }
  }
}
void task_1(void *arg)
{
  String input = "";
  for(;;)
  {
    //Serial.println("按下按键开始录音");
    //while(digitalRead(key)) //等待按键按下
    {esp_task_wdt_reset();}  
    //int sampleRate = ADC_Sampling(adc_temp);//采样
    // //play_the_adc(adc_temp,sampleRate);//播放
    //encodeFileToBase64(adc_temp,pcm_temp);//编码
    // Serial.println("开始上传给网站");
    // //容器初始化
    //String str_voice = Regnization(adc_temp,pcm_temp);//采样数据识别为中文
    /*如果想把串口输入的信息显示到屏幕上把“//@”取消注释，把payload赋值为input*/
     if(Serial.available()>0){
      char incomingChar = Serial.read(); //读取串口数据
       if (incomingChar == '\n') 
       { // 如果接收到换行符，说明一行字符串接收完成
         Serial.print("Received: ");Serial.println(input); // 将接收到的完整字符串打印出来
    //String en_str = ch_to_en(str_voice);
      xSemaphoreTake(xMutex, portMAX_DELAY);
         //翻译，内容同步到显示内容中
      payload =input;//en_str;//
      xSemaphoreGive(xMutex);input = ""; // 清空缓冲区，准备下一次接收
       }
       else {input += incomingChar; // 将接收到的字符追加到已有字符串后面
      }
    }
  }
}
String ch_to_en(String str_voice)
{
    if(WiFi.status() != WL_CONNECTED)
    {
      TO_connect_internet();
    }
    if(str_voice=="")
      return "";
    String url = "/rpc/2.0/mt/texttrans/v1?access_token=" + token;
    String test = "{\"from\": \"zh\",\"to\": \"en\",\"q\": \"" + str_voice + "\"}";
    String end_tra = "";
    //TO_connect_internet(); // 连接WiFi
    WiFiClientSecure client;  // 创建一个安全的WiFi客户端对象
    client.setInsecure();
    if (!client.connect("aip.baidubce.com", 443)) {  
        Serial.println("Connection failed");
        return "";
    }

    client.print("POST " + url + " HTTP/1.1\r\n");
    client.print("Host: aip.baidubce.com\r\n");
    client.print("Connection: close\r\n");
    client.print("Accept: application/json\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(test.length());
    client.print("\r\n\r\n");
    client.print(test);

    delay(1000);  

    while (client.connected() || client.available()) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
            // 在这里处理服务器响应，提取所需数据
            String temp = str_target(line,"dst\":\"","\",");
            if(temp!=nullptr)
              end_tra = temp;
        }
    }
    client.stop();
    Serial.println("Request complete");
    return end_tra;
}
String Regnization(const char* filename1,const char* filename2)  
{
    File File1,File2;String end_rec = "";
    File1 = SD.open(filename1,"r");
    int adc_len = File1.size();
    File1.close();
    TO_connect_internet();//连接WiFi
    HTTPClient client;
    data_info+=String(adc_len)+",\"speech\":\"";
    File2 = SD.open(filename2,"r");int fileSize = File2.size();
    if (File2) {
      // 从文件中读取内容
      while (File2.available()) {
        String temp = File2.readString();
        // 将从文件中读取的数据直接写入客户端
        data_info+=temp;
      }
      data_info+="\"}";
    } else {
      // 如果文件打开失败，打印错误原因
      Serial.print("打开文件失败，错误原因：");
      Serial.println(errno);
      return "";
    }
    //Serial.println(data_info);
    client.begin("http://vop.baidu.com/server_api");
    client.addHeader("Content-Type","application/json");
    int httpCode = client.POST(data_info.c_str());
    delay(1000);  // 等待一段时间以确保服务器发送所有数据
    if (!client.connected()) {
        // 连接已关闭，退出循环
        Serial.print("client is closed");
        return "";
    }
    if(httpCode > 0) {
        if(httpCode == HTTP_CODE_OK) {
          String line = client.getString();Serial.println(line);
          String temp = str_target(line,"\"result\":[\"","\"]");
          if(temp!=nullptr)
          {
           end_rec = temp;Serial.printf("结果：%s\n",end_rec.c_str());
          }
        }
    }
    else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", client.errorToString(httpCode).c_str());
    }
    client.end();
    Serial.printf("Recognition complete\r\n");
    return end_rec;
}

int ADC_Sampling(const char* filename)
{
  SD.remove(filename);//移除上一次录入的数据
  File File_test = SD.open(filename,FILE_WRITE);
  if (File_test) {
     Serial.print("打开/创建文件成功");
  } 
  else {
      // 如果文件打开失败，打印错误原因
      Serial.print("打开文件失败，错误原因：");
      Serial.println(errno);
      return 0;
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
  return n_recording/(time2/1000000.00f);
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
void play_the_adc(const char* filename, int sampleRate)
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
    // 计算每个样本的播放延时微秒数
    int delayMicros = 1000000 / sampleRate;   
    while (file.available())
    {
      // 读取两个字节的数据
      uint16_t value = (uint16_t)file.read() | ((uint16_t)file.read()<<8 );
      // 将16位数据转换为8位数据
      uint8_t data = (uint8_t)(value/256);   
      analogWrite(speaker, data);
      delayMicroseconds(delayMicros);
    }  
    analogWrite(speaker, 0);
    // 关闭文件
    file.close();
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
void draw(void *arg)
{
  String previous_payload = "";
    for (;;)
    {
        int str_size = payload.length();
        //Serial.printf("size:%d,%s\n", str_size, payload.c_str());
        if(payload!=previous_payload)
        {previous_payload=payload;Serial.printf("draw:%s\n",previous_payload.c_str());}
        int total_lines = (str_size + 15) / 16; // 计算字符串总共需要多少行来显示
        int total_pages = (total_lines + 3) / 4; // 计算总共需要多少页来显示

        u8g2.setFont(u8g2_font_wqy15_t_gb2312b);
        u8g2.setFontDirection(0);
        u8g2.clearBuffer();

        int start_pos = 0;
        for (int page = 1; page <= total_pages; page++)
        {
          u8g2.clearBuffer(); // 清空屏幕
          if (start_pos >= str_size) // 判断是否已经显示完所有字符
                {
                    delay(500); // 换页延时
                    break;
                }
            for (int line = 0; line < 4; line++)
            {
                u8g2.setCursor(0, 15 + line * 15); // 设置每行起始位置
                String subText = payload.substring(start_pos, start_pos + 16); // 截取需要显示的部分
                u8g2.print(subText);
                //Serial.printf("(%d,%d),payload:%s\n", 0, 15 + line * 15, subText.c_str());
                u8g2.sendBuffer();
                start_pos += 16; // 更新截取位置
            }
            delay(800); // 换页延时
        }
    }
    // while(digitalRead(key)) //等待按键按下
    //   esp_task_wdt_reset();  
}
String str_target(String src,String start,String end)
{
  int startPos = src.indexOf(start);
  if (startPos != -1) 
  {
    //startPos += 11; // 跳过 "\"dst\":\"" 的长度
    int endPos = src.indexOf(end, startPos);
    if (endPos != -1) {
    String dst = src.substring(startPos, endPos);//draw(dst);
    dst = dst.substring(start.length());
    Serial.println("Extracted dst: " + dst);
    //Serial.printf("draw successed?%s",dst.c_str());
    return dst;
    }
  }
  return "";
}