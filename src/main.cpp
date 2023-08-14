#include <Windows.h>
#include <QApplication>

#include "widget.h"

int main(int argc, char *argv[])
{
    // 控制台UTF-8编码输出
    SetConsoleOutputCP(65001);

    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
