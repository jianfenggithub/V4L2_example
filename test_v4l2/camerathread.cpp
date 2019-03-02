#include "camerathread.h"
#include <QThread>

extern QLabel *mylabel;

CameraThread::CameraThread()
{

}

CameraThread::~CameraThread()
{

}

/** 获取摄像头数据线程 */
void CameraThread::dowork()
{
    v4l.V4l_Init("/dev/video0", 30);                            /** 初始化 */

    while(1){
        if((QThread::currentThread()->isInterruptionRequested())){/** 是否有中断此线程信号 */
            v4l.Close_Camera();                                 /** 关闭摄像头 */
            break;
        }
        currentimage = v4l.Get_image();                         /** 获得一帧图片数据 */
        mylabel->setPixmap(QPixmap::fromImage(currentimage));   /** 显示到label上 */
    }

}
