# ifndef BMP_H
# define BMP_H

/*
BMP格式
这种格式内的数据分为三到四个部分，依次是：
文件信息头 （14字节）存储着文件类型，文件大小等信息
图片信息头 （40字节）存储着图像的尺寸，颜色索引，位平面数等信息
调色板 （由颜色索引数决定）【可以没有此信息】
位图数据 （由图像尺寸决定）每一个像素的信息在这里存储

一般的bmp图像都是24位，也就是真彩。每8位为一字节，24位也就是使用三字节来存储每一个像素的信息，三个字节对应存放r，g，b三原色的数据，
每个字节的存贮范围都是0-255。那么以此类推，32位图即每像素存储r，g，b，a（Alpha通道，存储透明度）四种数据。8位图就是只有灰度这一种信息，
还有二值图，它只有两种颜色，黑或者白。
*/
// 文件信息头结构体
typedef struct tagBITMAPFILEHEADER
{
    //unsigned short bfType;        // 19778，必须是BM字符串，对应的十六进制为0x4d42,十进制为19778，否则不是bmp格式文件
    unsigned int   bfSize;        // 文件大小 以字节为单位(2-5字节)
    unsigned short bfReserved1;   // 保留，必须设置为0 (6-7字节)
    unsigned short bfReserved2;   // 保留，必须设置为0 (8-9字节)
    unsigned int   bfOffBits;     // 从文件头到像素数据的偏移  (10-13字节)
} BITMAPFILEHEADER;

//图像信息头结构体
typedef struct tagBITMAPINFOHEADER
{
    unsigned int    biSize;          // 此结构体的大小 (14-17字节)
    long            biWidth;         // 图像的宽  (18-21字节)
    long            biHeight;        // 图像的高  (22-25字节)
    unsigned short  biPlanes;        // 表示bmp图片的平面属，显然显示器只有一个平面，所以恒等于1 (26-27字节)
    unsigned short  biBitCount;      // 一像素所占的位数，一般为24   (28-29字节)
    unsigned int    biCompression;   // 说明图象数据压缩的类型，0为不压缩。(30-33字节)
    unsigned int    biSizeImage;     // 像素数据所占大小, 这个值应该等于上面文件头结构中bfSize-bfOffBits (34-37字节)
    long            biXPelsPerMeter; // 说明水平分辨率，用象素/米表示。一般为0 (38-41字节)
    long            biYPelsPerMeter; // 说明垂直分辨率，用象素/米表示。一般为0 (42-45字节)
    unsigned int    biClrUsed;       // 说明位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）。(46-49字节)
    unsigned int    biClrImportant;  // 说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。(50-53字节)
} BITMAPINFOHEADER;

//24位图像素信息结构体,即调色板
typedef struct _PixelInfo {
    unsigned char rgbBlue;   //该颜色的蓝色分量  (值范围为0-255)
    unsigned char rgbGreen;  //该颜色的绿色分量  (值范围为0-255)
    unsigned char rgbRed;    //该颜色的红色分量  (值范围为0-255)
    unsigned char rgbReserved;// 保留，必须为0
} PixelInfo;
// extern BITMAPFILEHEADER fileHeader;
// extern BITMAPINFOHEADER infoHeader;
// void showBmpHead(BITMAPFILEHEADER pBmpHead);
// void showBmpInfoHead(BITMAPINFOHEADER pBmpinfoHead);
#endif