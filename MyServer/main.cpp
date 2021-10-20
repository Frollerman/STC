#include <QCoreApplication>
#include "MyServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    MyServer server(2323);

    return app.exec();
}
