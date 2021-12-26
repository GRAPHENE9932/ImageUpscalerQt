/*
 * ImageUpscalerQt tasks worker
 * Copyright (C) 2021  Artem Kliminskyi <artemklim50@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
