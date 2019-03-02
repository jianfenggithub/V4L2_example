#include "widget.h"
#include "ui_widget.h"


QLabel *mylabel = NULL;
extern QThread *ImageThread;    /** 获取摄像头数据线程 */

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    mylabel = ui->label;        /** 获得界面上的label对象, 使得可以在“获取摄像头数据线程”获取的图片数据显示到这个label上 */
}

Widget::~Widget()
{
    delete ui;
}

/** 关闭窗口时候的处理函数 */
void Widget::closeEvent(QCloseEvent *)
{
    ImageThread->requestInterruption(); /** 请求中断 */
    ImageThread->quit();                /** 关闭线程 */
    ImageThread->wait();                /** 同步关闭 */
}
