#include <QApplication>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "widget.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // 控制台UTF-8编码输出
    SetConsoleOutputCP(65001);
#endif

    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
