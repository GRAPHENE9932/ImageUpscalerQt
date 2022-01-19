/*
 * ImageUpscalerQt - main window header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <vector>

#include <QMainWindow>
#include <QScopedPointer>
#include <QSize>

#include "../tasks/TaskDesc.h"

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
	std::vector<std::shared_ptr<TaskDesc>> task_queue;
	QStringList files;

	/// @returns Size of the biggest (by width*height area) image in list.
	QSize max_image_size();
	/// Completely synchronize ImageUpscalerQt::files with file_list_widget.
	void update_file_list();
	void update_previews();
	/// Swap files in the list.
	void swap_files(int index_1, int index_2);
	/// Update every list manipulation button.
	void update_file_buttons();

private slots:
	void add_files_clicked();
	void move_file_up_clicked();
	void move_file_down_clicked();
	void remove_file_clicked();
	void clear_files_clicked();
	void file_selection_changed(int index);

	void add_task_clicked();
	void move_task_up_clicked();
	void move_task_down_clicked();
	void remove_task_clicked();
	void clear_tasks_clicked();
	void task_selection_changed(int index);

	void start_tasks_clicked();
	void about_clicked();
};
