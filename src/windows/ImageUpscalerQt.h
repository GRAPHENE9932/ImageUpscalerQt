/*
 * ImageUpscalerQt - main window header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
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

	std::vector<std::shared_ptr<TaskDesc>> tasks;
	QStringList files;

	/// @returns Size of the biggest (by width*height area) image in list.
	QSize max_image_size();
	/// @return Memory consumption of the heaviest neural network.
	unsigned long long max_nn_memory_consumption();
	/// Completely synchronize ImageUpscalerQt::files with file_list_widget.
	void update_file_list();
	void update_previews();
	/// Swap files in the list and in the GUI.
	void swap_files(int index_1, int index_2);
	/// Update every file list manipulation button.
	void update_file_buttons();

	/// Swap tasks in the list and in the GUI.
	void swap_tasks(int index_1, int index_2);
	/// Update every task list manipulation button.
	void update_task_buttons();

	/// Total amount of pixels in all files.
	unsigned long long total_pixels();
	void update_info_text();

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

	void about_program_triggered();
	void about_qt_triggered();

	void start_tasks_clicked();
};
