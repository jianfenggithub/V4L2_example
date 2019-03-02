#include "widget.h"
#include <QApplication>
#include <QThread>
#include "camerathread.h"

QThread *ImageThread = new QThread;                         /** 获取摄像头数据线程 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    CameraThread *camera_worker = new CameraThread;

    camera_worker->moveToThread(ImageThread);               /** 创建线程 */
    QObject::connect(ImageThread, SIGNAL(started()),
                     camera_worker, SLOT(dowork()));
    QObject::connect(ImageThread, SIGNAL(finished()),
                     camera_worker, SLOT(deleteLater()));   /** 当线程结束时，释放资源 */

    ImageThread->start();                                   /** 启动线程 */

    w.show();

    return a.exec();
}
