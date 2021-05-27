#ifndef IMAGEUPSCALERQT_H
#define IMAGEUPSCALERQT_H

#include <vector>

#include <QMainWindow>
#include <QScopedPointer>
#include <OpenImageIO/imageio.h>

#include "../tasks/Task.h"

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
	OIIO::ImageSpec image_spec;

	void update_list();

private slots:
	void add_task_clicked();
	void task_kind_changed(int index);
	void select_image_clicked();

	void remove_task_clicked();
	void clear_tasks_clicked();
	void move_task_up_clicked();
	void move_task_down_clicked();

	void resize_x_changed(int value);
	void resize_y_changed(int value);
	void keep_ratio_toggled(bool checked);

	void start_tasks_clicked();
};

#endif // IMAGEUPSCALERQT_H
