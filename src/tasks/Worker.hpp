/*
 * ImageUpscalerQt - tasks worker header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QStringList>
#include <OpenImageIO/imagebuf.h>

#include "TaskDesc.hpp"
#include "Task.hpp"

class Worker {
public:
	Worker();
	/// Not only construct, but also init().
	Worker(const std::vector<std::shared_ptr<TaskDesc>>& tasks,
		   const std::vector<std::pair<QString, QString>>& files);

	/// Needed if the worker was constructed with default constructor.
	void init(std::vector<std::shared_ptr<TaskDesc>> task_descs,
			  std::vector<std::pair<QString, QString>> files);
	QString cur_status() const;
	int get_cur_task_index() const;
	int get_cur_img_index() const;
	/// Progress of the current task (from 0 to 1).
	float cur_task_progress() const;
	/// Progress of all tasks and all images (from 0 to 1).
	float overall_progress() const;

	void do_tasks(std::function<void()> success, std::function<void()> canceled,
				  std::function<void(QString)> error);
	void cancel();

private:
	/// Vector of pairs "original file - result file".
	std::vector<std::pair<QString, QString>> files;
	std::vector<Task*> tasks;
	int cur_task = 0, cur_img = 0;

	bool cancel_requested = false;
	bool everything_finished = false;
	bool img_writing_now = false;
};
