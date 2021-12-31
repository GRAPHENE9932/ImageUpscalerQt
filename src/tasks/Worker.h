/*
 * ImageUpscalerQt - tasks worker header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.h"

class Worker {
public:
	/// Already finished image.
	OIIO::ImageBuf finished_image;

	Worker();
	/// Not only construct, but also init().
	Worker(std::vector<Task*> queue, QString input_filename);

	/// Needed if the worker was constructed with default constructor.
	void init(std::vector<Task*> queue, QString input_filename);
	QString cur_status() const;
	/// Progress of current task.
	float cur_task_progress() const;
	/// Progress of all tasks.
	float overall_progress() const;

	void do_tasks(std::function<void()> success, std::function<void()> canceled,
				  std::function<void(QString)> error);
	void save_image(QString filename, std::function<void(QString)> error);
	void cancel();

private:
	/// Current image (can be unfinished).
	OIIO::ImageBuf cur_image;

	QString input_filename;
	std::vector<Task*> tasks_queue;
	unsigned short cur_task = 0;

	bool cancel_requested = false;
};
