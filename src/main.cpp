#include "windows/imageupscalerqt.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ImageUpscalerQt w;
    w.show();

    return app.exec();
}

