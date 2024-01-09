#include <stdio.h>
#include <iostream>
#include <string.h>
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <memory>
#include "h_curl_encode.h"
struct http_result {
    int has_error;
	char *format; // 音频的格式，aue参数设置
    FILE *fp; // 保存结果的文件
};
std::string client_id = "w1RXUe0iP9IHz8j3WIwtpnls";
std::string client_secret = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";

inline size_t onWriteData(void * buffer, size_t size, size_t nmemb, void * userp)
{
    std::string * str = dynamic_cast<std::string *>((std::string *)userp);
    str->append((char *)buffer, size * nmemb);
    return nmemb;
}
inline size_t onWriteData_wav(void *ptr, size_t size, size_t nmemb, struct http_result *result)
{
    std::cout<<"ptr:"<<ptr<<std::endl;
    char filename[11];
    snprintf(filename, 11, "temp.%s", 0 ? "txt" : "wav");
    if (result->fp == NULL) {
        result->fp = fopen(filename, "w+");
        if (result->fp == NULL) {
            //snprintf(g_demo_error_msg, BUFFER_ERROR_SIZE, "%s cannot be opened\n", filename);
            std::cout<<"open error"<<std::endl;
            return 0;
        }
        printf("Data will be write into %s in the current directory\n", filename);
    }
    std::cout<<__LINE__<<std::endl;
    return fwrite(ptr, size, nmemb,result->fp);
}
std::string getFileBase64Content(const char * path,  bool urlencoded=false)
{
    const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    unsigned int bufferSize = 1024;
    unsigned char buffer[bufferSize];
    std::ifstream file_read;
    file_read.open(path, std::ios::binary);
    while (!file_read.eof())
    {
        file_read.read((char *) buffer, bufferSize * sizeof(char));
        int num = file_read.gcount();
        int m = 0;
        while (num--)
        {
            char_array_3[i++] = buffer[m++];
            if(i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                for(i = 0; (i <4) ; i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }
    file_read.close();
    if(i)
    {
        for(j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for(j = 0; (j < i + 1); j++)
        {
            ret += base64_chars[char_array_4[j]];
        }
        while((i++ < 3))
        {
            ret += '=';
        }
    }
    if (urlencoded)
    {
        ret = curl_escape(ret.c_str(), ret.length());
    }
    return ret;
}


std::string getAccessToken()
{
    std::string result;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "https://aip.baidubce.com/oauth/2.0/token");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string data = "grant_type=client_credentials&client_id="+client_id+"&client_secret="+client_secret;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(result.data(), result.data() + result.size(), &obj, &error);
    return obj["access_token"].asString();
}

int main(int argc, char *argv[])
{
    std::string result;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    char str_in[512];memcpy(str_in,argv[1],strlen(argv[1])+1);
    char *textemp = curl_easy_escape(curl, str_in,strlen(str_in)); // 需要释放
	char *tex = curl_easy_escape(curl, textemp, strlen(textemp)); // 需要释放
    free(textemp);
    if(curl){
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, ("https://tsn.baidu.com/text2audio"));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: */*");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // tok 可以通过 getFileBase64Content('access_token') 方法获取
        // 注意getFileBase64Content方法第二个参数，如果Content-Type是application/x-www-form-urlencoded时，urlencoded请设置为True
        char parms[] = "tex=%s&tok=%s&cuid=860697924YB&ctp=1&lan=zh&spd=5&pit=5&vol=5&per=0&aue=6";
        char data[512];
        snprintf(data,sizeof(data),parms,tex,getAccessToken);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        std::cout<<"!!!!!!"<<data<<std::endl;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData_wav);
        res = curl_easy_perform(curl);
        std::cout<<"result is"<<result;
    }
    free(tex);
    curl_easy_cleanup(curl);
    return (int)res;
}
