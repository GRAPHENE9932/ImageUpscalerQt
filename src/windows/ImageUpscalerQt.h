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
	/// Vector of pairs "original file - result file".
	std::vector<std::pair<QString, QString>> files;

	/// Size of the biggest (by width*height area) image in the list.
	QSize max_image_size();
	/// Size of the biggest image in the list after every task.
	QSize max_result_image_size();
	/// Create the output image path automatically from the original path.
	QString auto_output_path(QString orig_path);
	/// Memory consumption of the heaviest neural network.
	unsigned long long max_nn_memory_consumption();
	/// Add files to the list and GUI.
	void add_files(QStringList files);
	void update_previews(int first, int end);
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
