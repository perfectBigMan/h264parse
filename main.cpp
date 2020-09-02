#include "widget.h"
#include <QApplication>

#include <iostream>
using namespace std;
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font = a.font();
    font.setPointSize(8);//字体大小
    font.setFamily("Consolas");//微软雅黑字体
    a.setFont(font);

    Widget w;
    w.show();

//    av_register_all();
//    cout << "version is:" << avcodec_version() << endl;
//    cout << avcodec_configuration() << endl;

    return a.exec();
}
