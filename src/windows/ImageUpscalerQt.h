/*
 * ImageUpscalerQt - main window header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

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
	QString image_filename;
	OIIO::ImageSpec image_spec;

	/// Prepare GUI for entering parameters (default parameters).
	void prepare_task_resize();
	/// Prepare GUI for entering parameters (default parameters).
	void prepare_task_srcnn();
	/// Update info for SRCNN that in labels.
	void update_srcnn_info();
	/// Prepare GUI for entering parameters (default parameters).
	void prepare_task_fsrcnn();
	/// Update info for FSRCNN that in labels.
	void update_fsrcnn_info();
	/// Prepare GUI for entering parameters (default parameters).
	void prepare_task_convert_color_space();

	/// Collect task parameters from GUI.
	Task* init_task_resize();
	/// Collect task parameters from GUI.
	Task* init_task_srcnn();
	/// Collect task parameters from GUI.
	Task* init_task_fsrcnn();
	/// Collect task parameters from GUI.
	Task* init_task_convert_color_space();

	void update_list();

	int end_width();
	int end_height();

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

	// BEGIN SRCNN page events
	void srcnn_architecture_changed(QString);
	void srcnn_block_size_changed(int);
	void srcnn_split_checked(int);
	// END SRCNN page events

	// BEGIN FSRCNN page events
	void fsrcnn_architecture_changed(QString);
	void fsrcnn_block_size_changed(int);
	void fsrcnn_split_checked(int);
	// END SRCNN page events

	void start_tasks_clicked();

	void about_clicked();
};
