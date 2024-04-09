#include <stdio.h>
#include <stdlib.h>
#include "BMP.h"

#define FILE_NAME "1.bmp"
#define NEW_FILE_NAME "2.bmp"
BITMAPFILEHEADER fileHeader;
BITMAPINFOHEADER infoHeader;
PixelInfo RGBQUAD;
void showBmpHead(BITMAPFILEHEADER pBmpHead)
{  //定义显示信息的函数，传入文件头结构体
    printf("BMP文件大小：%d Bytes,%dkb\n", fileHeader.bfSize,fileHeader.bfSize/1024);
    printf("保留字必须为0：%d\n",  fileHeader.bfReserved1);
    printf("保留字必须为0：%d\n",  fileHeader.bfReserved2);
    printf("实际位图数据的偏移字节数: %d\n",  fileHeader.bfOffBits);
}
void showBmpInfoHead(BITMAPINFOHEADER pBmpinfoHead)
{//定义显示信息的函数，传入的是信息头结构体
   printf("位图信息头:\n" );
   printf("信息头的大小:%d\n" ,infoHeader.biSize);
   printf("位图宽度:%d\n" ,infoHeader.biWidth);
   printf("位图高度:%d\n" ,infoHeader.biHeight);
   printf("图像的位面数(位面数是调色板的数量,默认为1个调色板):%d\n" ,infoHeader.biPlanes);
   printf("每个像素的位数:%d\n" ,infoHeader.biBitCount);
   printf("压缩方式:%d\n" ,infoHeader.biCompression);
   printf("图像的大小:%d\n" ,infoHeader.biSizeImage);
   printf("水平方向分辨率:%d\n" ,infoHeader.biXPelsPerMeter);
   printf("垂直方向分辨率:%d\n" ,infoHeader.biYPelsPerMeter);
   printf("使用的颜色数:%d\n" ,infoHeader.biClrUsed);
   printf("重要颜色数:%d\n" ,infoHeader.biClrImportant);
}
int main() {
    FILE *file,*new_file;
    unsigned char *data;
    int width, height;  
    // 打开bmp文件
    file = fopen(FILE_NAME, "rb");
    if (!file) {
        printf("无法打开文件！\n");perror("fopen failed");
        return 1;
    }
    //如果不先读取bifType，根据C语言结构体Sizeof运算规则——整体大于部分之和，从而导致读文件错位
    unsigned short  fileType;
    fread(&fileType,1,sizeof (unsigned short), file);
    if (fileType == 0x4d42)
    {
        printf("文件类型标识正确!" );
        printf("\n文件标识符：0X%04X\n", fileType);
    }
    else
    {
        fclose(file);printf("不是bmp图");return 0;
    }
    fread(&fileHeader, 1, sizeof(BITMAPFILEHEADER), file);
    showBmpHead(fileHeader);
    fread(&infoHeader, 1, sizeof(BITMAPINFOHEADER), file);
    showBmpInfoHead(infoHeader);
    // 提取图像宽度和高度信息
    width = infoHeader.biWidth;
    height = infoHeader.biHeight;
    printf("height:%d,width:%d\n",height,width);
    // 分配内存存储像素数据
    data = (unsigned char*)malloc(width * height * sizeof(unsigned char)/2);
    if (!data) {
        printf("内存分配失败！\n");
        fclose(file);
        return 1;
    }
    int extra_size = fileHeader.bfSize-sizeof(fileType)-sizeof(fileHeader)-sizeof(infoHeader)-width*height/2;
    printf("剩余%d bytes\n",extra_size);
    unsigned char *extra_bytes = (unsigned char*)malloc(extra_size*sizeof(unsigned char));
    fread(extra_bytes, sizeof(unsigned char),extra_size, file);
    // 读取像素数据
    fread(data, sizeof(unsigned char), width * height/2, file);
    
    // 关闭文件
    fclose(file);
    int graph_size = width*height/2;
    int compression_size = graph_size/4;
    int index=0,value=0;
    unsigned char *compression_data = (unsigned char*) malloc(sizeof(unsigned char)*compression_size);
    for(int i=0;i<extra_size;i++)
        printf("<%d>",extra_bytes[i]);
    for(int i=0;i<graph_size;i++)
        printf("%d,",data[i]);
    //压缩
    for(int i=0;i<graph_size;i++)
    {
        unsigned char lbit4 = (data[i]&0x0F)>0?1:0;
        unsigned char hbit4 = (data[i]>>4)>0?1:0;
        value = (value<<2)|(lbit4|(hbit4<<1));
        if((i+1)%4==0)
        {
            compression_data[index++]=value;
        }
    } 
    printf("index:%d\n",index);
    // for(int i=0;i<compression_size;i++)
    // {
    //     printf("0X%02x,",compression_data[i]);
    // } 
    // for(int i=0;i<compression_size;i++)
    // {
    //     for(int j=0;j<4;j++)
    //     {
    //         unsigned char value = (compression_data[i]&(0xC0>>(j*2)))>>(8-(j+1)*2);//
    //         unsigned char lbit4 = (value&0x01)==1?0x0F:0;
    //         unsigned char hbit4 = (value&0x02)==1?0:0x0F;
    //         data[i*4+j] = (hbit4<<4)|(lbit4);

    //     }
    // }
    //创建新的BMP文件
    new_file = fopen(NEW_FILE_NAME, "wb");
    if (!new_file) {
        printf("无法创建新文件！\n");
        perror("fopen failed");
        free(data);
        return 1;
    }
    // 写入BMP文件头
    fwrite(&fileType,1,sizeof (unsigned short), new_file);
    fwrite(&fileHeader, sizeof(unsigned char), sizeof(BITMAPFILEHEADER), new_file);
    fwrite(&infoHeader, sizeof(unsigned char), sizeof(BITMAPINFOHEADER), new_file);
    // 写入二值化后的像素数据
    fwrite(extra_bytes, sizeof(unsigned char), extra_size, new_file);
    fwrite(data, sizeof(unsigned char), width * height/2, new_file);
    
    // 关闭新文件
    fclose(new_file);
    //释放内存
    free(data);free(extra_bytes);free(compression_data);
    printf("二值化后的图像已保存为 %s\n", NEW_FILE_NAME);
    return 0;
}
