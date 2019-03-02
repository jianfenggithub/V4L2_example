#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QObject>
#include "v4l2.h"
#include <QImage>
#include "widget.h"
#include "ui_widget.h"

class CameraThread : public QObject
{
    Q_OBJECT
public:
    CameraThread();
    ~CameraThread();
    V4L2 v4l;
    QImage currentimage;            /** 保存摄像头一帧图片数据,然后显示到label上 */

signals:

public slots:
    void dowork();                  /** 获取摄像头数据线程 */
};

#endif // CAMERATHREAD_H
