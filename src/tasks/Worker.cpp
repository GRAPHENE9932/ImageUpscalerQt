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

#include "Worker.h"

Worker::Worker() {

}

Worker::Worker(std::vector<Task*> queue, QString input_filename) {
	init(queue, input_filename);
}

void Worker::init(std::vector<Task*> queue, QString input_filename) {
	this->tasks_queue = queue;
	this->input_filename = input_filename;
}

float Worker::cur_task_progress() const {
	if (cur_task >= tasks_queue.size())
		return 1;
	else
		return tasks_queue[cur_task]->progress();
}

float Worker::overall_progress() const {
	float cur_progress = cur_task_progress();
	return ((float)cur_task + cur_progress) / (float)tasks_queue.size();
}

QString Worker::cur_status() const {
	//Save cur_task for this function, because multithreading
	auto cur_task_copy = cur_task;
	if (cur_task_copy < tasks_queue.size()) {
		//Prepare text for current task label
		//Task 1/1: Unknown task (100%)
		if (cur_task_progress() == 0)
			return QString("Task %1/%2: %3").arg(QString::number(cur_task_copy + 1),
												 QString::number(tasks_queue.size()),
												 tasks_queue[cur_task_copy]->to_string());
		//Add "(xxx%)" if not 0%
		else
			return QString("Task %1/%2: %3 (%4%)").arg(QString::number(cur_task_copy + 1),
													   QString::number(tasks_queue.size()),
													   tasks_queue[cur_task_copy]->to_string(),
													   QString::number((unsigned short)(cur_task_progress() * 100.0F)));
	}
	else {
		return "Done!";
	}
}

void Worker::do_tasks(std::function<void()> success, std::function<void()> canceled,
					  std::function<void(QString)> error) {
	//Disable "cancel_requested" in all the tasks
	for (unsigned short i = 0; i < tasks_queue.size(); i++)
		tasks_queue[i]->cancel_requested = false;

	try {
		//Read image
		OIIO::ImageBuf input_buf = OIIO::ImageBuf(input_filename.toStdString());

		for (cur_task = 0; cur_task < tasks_queue.size(); cur_task++) {
			cur_image = tasks_queue[cur_task]->do_task(input_buf);
			input_buf = cur_image;

			if (cancel_requested) {
				canceled();
				return;
			}
		}
		finished_image = input_buf;
	}
	catch (const char* str) {
		if (strcmp(str, "canc") == 0)
			canceled();
		else
			error(str);
		return;
	}
#ifdef NDEBUG
	catch (std::runtime_error e) {
		error(e.what());
		return;
	}
	catch (std::exception e) {
		error(e.what());
		return;
	}
	catch (...) {
		error("Unknown error");
		return;
	}
#endif
	success(); //If not canceled and no errors occured
}

void Worker::save_image(QString filename, std::function<void(QString)> error) {
	if (!finished_image.write(filename.toStdString()))
		error(QString::fromStdString(cur_image.geterror()));
}

void Worker::cancel() {
	tasks_queue[cur_task]->cancel_requested = true;
	cancel_requested = true;
}
