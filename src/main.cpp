/*
 * ImageUpscalerQt - main function
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "windows/ImageUpscalerQt.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

	// Disable the context help button globally.
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
	// HiDPI icons
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    ImageUpscalerQt w;
    w.show();

    return app.exec();
}

