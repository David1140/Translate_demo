#include<stdio.h>
#include<string.h>
#include <iostream>
#include <string>
std::string client_id_voice = "w1RXUe0iP9IHz8j3WIwtpnls";
std::string client_secret_voice = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
void test0()
{
    char* api_key = "w1RXUe0iP9IHz8j3WIwtpnls";
    char* seceretkey = "O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
    char fortoken_temp[500];
    snprintf(fortoken_temp,153,"%s%s%s%s","https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=",api_key,"&client_secret=",seceretkey);
    printf("STRsize::%d,%s",strlen(fortoken_temp),fortoken_temp);
}
int main()
{   
    std::string str ="https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=w1RXUe0iP9IHz8j3WIwtpnls&client_secret=O7SmzOcKynFSGkoPWcSBGDkOjqZupaLl";
    std::string fortoken_temp = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=";
    fortoken_temp+=client_id_voice+"&client_secret="+client_secret_voice;
    //std::string fortoken_temp = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id="+client_id_voice+"&client_secret="+client_secret_voice;
    printf("%s\v",fortoken_temp.c_str());
    printf("%s",str.c_str());
}