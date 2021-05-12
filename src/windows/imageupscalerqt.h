#ifndef IMAGEUPSCALERQT_H
#define IMAGEUPSCALERQT_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class ImageUpscalerQt;
}

class ImageUpscalerQt : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageUpscalerQt(QWidget *parent = nullptr);
    ~ImageUpscalerQt() override;

private:
    QScopedPointer<Ui::ImageUpscalerQt> m_ui;
};

#endif // IMAGEUPSCALERQT_H
