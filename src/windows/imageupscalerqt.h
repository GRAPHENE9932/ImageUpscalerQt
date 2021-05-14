#ifndef IMAGEUPSCALERQT_H
#define IMAGEUPSCALERQT_H

#include <vector>

#include <QMainWindow>
#include <QScopedPointer>


#include "../task/Task.h"

namespace Ui {
class ImageUpscalerQt;
}

class ImageUpscalerQt : public QMainWindow {
    Q_OBJECT

public:
    explicit ImageUpscalerQt(QWidget *parent = nullptr);
    ~ImageUpscalerQt() override;

private:
    QScopedPointer<Ui::ImageUpscalerQt> m_ui;
	std::vector<Task*> task_queue;
	std::string image_filename;

	void update_list();

private slots:
	void add_task_clicked();
	void task_kind_changed(int index);
	void select_image_clicked();
};

#endif // IMAGEUPSCALERQT_H
