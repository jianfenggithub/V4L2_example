/**
 * 功能：使用V4L2采集UVC摄像头数据
 * 日期：2018.3.26
 * 用法：在Ubuntu下的Qt工程中添加V4L2.h和V4L2.cpp文件，定义一个V4L2对象，
 *      调用bool V4L2::V4l_Init(char* camera_path, unsigned int frame) 函数对摄像头进行初始化操作
 *      调用QImage V4L2::Get_image() 函数就可以获得摄像头传来的一张图片
 *      调用bool Close_Camera(void)函数关闭摄像头
 * 注意：需要在V4L2.cpp的54行左右可修改摄像头图片的输出格式，如修改为MJPEG输出
 *      在V4L2.h的宏定义中可以修改输出图片的像素
 *
 * v1.1-2018.9.26:修改注释风格，规范代码
 * v1.2-2019.3.2:优化Get_image函数,减少数据转移次数
 */

#ifndef V4L2_H
#define V4L2_H

#include <fcntl.h>                                  /** 打开摄像头的open函数所在的头文件 */
#include <sys/mman.h>                               /** 将申请的内核缓存映射到用户空间的mmap函数所在头文件 */
#include <linux/videodev2.h>                        /** V4L2相关结构体所在的头文件 */
#include <unistd.h>                                 /** 关闭摄像头的close函数所在的头文件 */
#include <sys/ioctl.h>
#include <QDebug>
#include <QImage>

#define Video_count    3                            /** 缓冲帧数 */
#define Image_Width    1280                         /** 输出图片的像素 */
#define Image_High     720

class V4L2
{
public:
    V4L2();
    ~V4L2();
    bool V4l_Init(char *camera_path, unsigned int frame);/** 摄像头初始化函数,需要传入摄像头的挂载路径和输出的帧率，初始化成功则返回真 */
    QImage Get_image(void);                         /** 获取图片，返回值为QImage类型的变量 */
    bool Close_Camera(void);                        /** 关闭摄像头，关闭成功则返回真 */
    int n;                                          /** 用在Get_image函数，获取图片时，用来控制哪快缓存 */

private:
    int                           i;
    int                           fd;               /** 摄像头句柄 */
    int                           length[Video_count];/** 用来保存申请的缓存的大小 */
    QImage                        image;
    unsigned char *               start[Video_count];/** 用来保存图片数据 */

    struct v4l2_buffer            buffer[Video_count];/** 向内核申请缓存时用到此结构体，每一个struct v4l2_buffer对应内核摄像头驱动中的一个缓存 */
    struct v4l2_format            fmt;               /** 用来设置像素格式 图片输出格式 图像尺寸 扫描方式 */
    struct v4l2_fmtdesc           fmtdesc;           /** 获取摄像头输出图片所支持的格式时需要用到此结构体 */
    struct v4l2_capability        cap;               /** 检查摄像头的信息时需要用到此结构体 */
    struct v4l2_streamparm        setfps;            /** 设置帧率时用到此结构体 */
    struct v4l2_requestbuffers    req;               /** 向内核申请缓存时用到此结构体 */
    struct v4l2_buffer            v4lbuf;            /** 缓存出队 入队时用到此结构体 */
    enum   v4l2_buf_type          type;              /** 开启I/O流时用到此结构体 */
};

#endif // V4L2_H
