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

#include "v4l2.h"

V4L2::V4L2()
{
    n = 0;
}

V4L2::~V4L2()
{

}

/** 摄像头初始化,需要传入摄像头的挂载路径和输出的帧率，初始化成功则返回真 */
bool V4L2::V4l_Init(char *camera_path, unsigned int frame)
{
    if((fd=open(camera_path, O_RDWR)) == -1){          /** 读写方式打开摄像头 */
        qDebug()<<"Error opening V4L interface";
        return false;
    }
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1){       /** 检查摄像头的信息 */
        qDebug()<<"Error opening device "<<camera_path<<": unable to query device.";
        return false;
    }else{                                             /** 打印摄像头的信息 */
        qDebug()<<"driver:\t\t"<<QString::fromLatin1((char *)cap.driver);     /** 驱动名 */
        qDebug()<<"card:\t\t"<<QString::fromLatin1((char *)cap.card);         /** Device名 */
        qDebug()<<"bus_info:\t\t"<<QString::fromLatin1((char *)cap.bus_info); /** 在Bus系统中存放位置 */
        qDebug()<<"version:\t\t"<<cap.version;                                /** driver 版本 */
        qDebug()<<"capabilities:\t"<<cap.capabilities;                        /** 能力集,通常为：V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING */
    }

    fmtdesc.index=0;
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;          /** 和struct v4l2_format中的type一致 */
    qDebug()<<"Support format:";
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1){     /** 获取摄像头输出图片所支持的格式 */
        qDebug()<<"\t\t"<<fmtdesc.index+1<<QString::fromLatin1((char *)fmtdesc.description);
        fmtdesc.index++;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;            /** 像素格式 */
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;       /** JPEG格式输出 */
    fmt.fmt.pix.height = Image_High;                   /** 图像尺寸,在这里设置想要输出的图片宽和高 */
    fmt.fmt.pix.width = Image_Width;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;         /** 视频帧传输方式,交错式 */
    if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1){           /** 配置摄像头的采集格式 */
        qDebug()<<"Unable to set format";
        return false;
    }
    if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1){           /** 重新读取结构体，以确认完成设置 */
        qDebug()<<"Unable to get format";
        return false;
    }else{
        qDebug()<<"fmt.type:\t\t"<<fmt.type;            /** 输出像素格式 */
        qDebug()<<"pix.height:\t"<<fmt.fmt.pix.height;/** 输出图像的尺寸 */
        qDebug()<<"pix.width:\t\t"<<fmt.fmt.pix.width;
        qDebug()<<"pix.field:\t\t"<<fmt.fmt.pix.field;  /** 视频帧传输方式 */
    }

    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.denominator = frame;/** 预期的帧率 */
    setfps.parm.capture.timeperframe.numerator = 1;     /** fps=frame/1 */
    if(ioctl(fd, VIDIOC_S_PARM, &setfps)==-1){          /** 设置帧数 */
        qDebug()<<"Unable to set fps";
        return false;
    }
    if(ioctl(fd, VIDIOC_G_PARM, &setfps)==-1){          /** 重新读取结构体，以确认完成设置 */
        qDebug()<<"Unable to get fps";
        return false;
    }else{
        qDebug()<<"fps:\t\t"<<setfps.parm.capture.timeperframe.denominator/setfps.parm.capture.timeperframe.numerator;/** 输出帧率 */
    }

    req.count=Video_count;                              /** 3个缓存区 */
    req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;               /** 与struct v4l2_format中的type一致 */
    req.memory=V4L2_MEMORY_MMAP;                        /** Memory Mapping模式，设置为V4L2_MEMORY_MMAP时count字段才有效 */
    if(ioctl(fd,VIDIOC_REQBUFS,&req)==-1){              /** 向内核申请视频缓存 */
        qDebug()<<"request for buffers error";
        return false;
    }
    for (i=0; i<Video_count; i++)                       /** mmap四个缓冲区 */
    {
        bzero(&buffer[i], sizeof(buffer[i]));
        buffer[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer[i].memory = V4L2_MEMORY_MMAP;
        buffer[i].index = i;
        if (ioctl (fd, VIDIOC_QUERYBUF, &buffer[i]) == -1){ /** 获得缓冲区的参数，mmap时需要这些参数 */
            qDebug()<<"query buffer error";
            return false;
        }
        length[i] = buffer[i].length;                   /** 保存缓存的大小 */
        start[i] = (unsigned char *)mmap(NULL,buffer[i].length,PROT_READ |PROT_WRITE, MAP_SHARED, fd, buffer[i].m.offset);
                                                        /** 将申请的内核缓存映射到用户空间 */
    }
    for (i=0; i<Video_count; i++)
    {
        buffer[i].index = i;
        ioctl(fd, VIDIOC_QBUF, &buffer[i]);             /** 将缓存入队 */
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl (fd, VIDIOC_STREAMON, &type);                 /** 开启I/O流 */

    bzero(&v4lbuf, sizeof(v4lbuf));
    v4lbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuf.memory = V4L2_MEMORY_MMAP;
    qDebug()<<"init ok";

    return true;
}

QImage V4L2::Get_image()                               /** 得到摄像头捕获的图片，返回值为QImage类型的变量 */
{
    v4lbuf.index = n%Video_count;
    ioctl(fd, VIDIOC_DQBUF, &v4lbuf);                  /** 将获得图像数据的缓存出队 */

    QByteArray temp;
    temp.append((const char *)start[n%Video_count],length[n%Video_count]);
    image.loadFromData(temp);                          /** 将图片数据放入image变量中 */

    v4lbuf.index = n%Video_count;
    ioctl(fd, VIDIOC_QBUF, &v4lbuf);                   /** 将已经读取过数据的缓存块重新入队 */
    n++;
    if(n == 3){                                        /** 防止n累加溢出 */
        n = 0;
    }
    return image;                                      /** 返回图片数据 */
}

bool V4L2::Close_Camera()                              /** 关闭摄像头,关闭成功则返回真 */
{
    if(fd != -1){
        ioctl(fd, VIDIOC_STREAMOFF, &type);            /** 结束图像显示 */
        int n = close(fd);                             /** 关闭视频设备 */
        if(n == -1){
            qDebug()<<"close camera failed";
            return false;
        }
    }
    for(i=0; i<Video_count; i++)
    {
        if(start[i] != NULL){                          /** 释放申请的内存 */
            start[i] = NULL;
        }
    }
    return true;
}
