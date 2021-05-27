#include "windows/ImageUpscalerQt.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

	//Disable context help button globally
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

    ImageUpscalerQt w;
    w.show();

    return app.exec();
}

