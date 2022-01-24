/*
 * ImageUpscalerQt - tasks worker header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QStringList>
#include <OpenImageIO/imagebuf.h>

#include "TaskDesc.h"
#include "Task.h"

class Worker {
public:
	/// Resulting images (may be unfinished).
	std::vector<OIIO::ImageBuf> res_images;

	Worker();
	/// Not only construct, but also init().
	Worker(std::vector<std::shared_ptr<TaskDesc>> tasks, QStringList files);

	/// Needed if the worker was constructed with default constructor.
	void init(std::vector<std::shared_ptr<TaskDesc>> tasks, QStringList files);
	QString cur_status() const;
	/// Progress of the current task (from 0 to 1).
	float cur_task_progress() const;
	/// Progress of all tasks and all images (from 0 to 1).
	float overall_progress() const;

	void do_tasks(std::function<void()> success, std::function<void()> canceled,
				  std::function<void(QString)> error);
	void save_images(QString folder_path, std::function<void(QString)> error);
	void cancel();

private:
	QStringList files;
	std::vector<Task*> tasks;
	int cur_task = 0, cur_img = 0;

	bool cancel_requested = false;
};
