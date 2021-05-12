#include "imageupscalerqt.h"
#include "ui_imageupscalerqt.h"

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt)
{
    m_ui->setupUi(this);
}

ImageUpscalerQt::~ImageUpscalerQt() = default;
