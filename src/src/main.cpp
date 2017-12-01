#include <QCoreApplication>
#include "controller.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        return -1;
    }
    QString uin(argv[1]);
    Controller* con = new Controller(uin);

    return a.exec();
}

