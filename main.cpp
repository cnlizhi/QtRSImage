//Remote Sensing Image Process Program

#include "QRSImageWindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QRSImageWindow RSImageWindow;
    RSImageWindow.show();
    return app.exec();
}
