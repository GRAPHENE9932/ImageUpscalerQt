/*
 * ImageUpscalerQt - tasks worker
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "Worker.h"
#include "TaskResize.h"
#include "TaskConvertColorSpace.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"

Worker::Worker() {

}

Worker::Worker(std::vector<std::shared_ptr<TaskDesc>> tasks, QStringList files) {
	init(tasks, files);
}

void Worker::init(std::vector<std::shared_ptr<TaskDesc>> tasks_descs, QStringList files) {
	// Construct tasks from theirs descriptions.
	tasks.resize(tasks_descs.size());
	for (int i = 0; i < tasks_descs.size(); i++) {
		const auto ptr = tasks_descs[i];

		switch (ptr->task_kind()) {
		case TaskKind::resize: {
			tasks[i] = new TaskResize(*dynamic_cast<TaskResizeDesc*>(ptr.get()));
			break;
		}
		case TaskKind::convert_color_space: {
			tasks[i] = new TaskConvertColorSpace(*dynamic_cast<TaskConvertColorSpaceDesc*>(ptr.get()));
			break;
		}
		case TaskKind::srcnn: {
			tasks[i] = new TaskSRCNN(*dynamic_cast<TaskSRCNNDesc*>(ptr.get()));
			break;
		}
		case TaskKind::fsrcnn: {
			tasks[i] = new TaskFSRCNN(*dynamic_cast<TaskFSRCNNDesc*>(ptr.get()));
			break;
		}
		}
	}

	this->files = files;
	this->res_images.resize(files.size());
}

float Worker::cur_task_progress() const {
	if (cur_task >= tasks.size())
		return 1;
	else
		return tasks[cur_task]->progress();
}

float Worker::overall_progress() const {
	float cur_img_progress = ((float)cur_task + cur_task_progress()) / (float)tasks.size();
	return ((float)cur_img + cur_img_progress) / files.size();
}

QString Worker::cur_status() const {
	// Save cur_task and cur_img for this function because
	// other thread can change these values during execution of this function.
	auto cur_task_copy = cur_task;
	auto cur_img_copy = cur_img;
	if (cur_task_copy < tasks.size()) {
		// Prepare text for current task label.
		// Task 1/1: Unknown task.
		if (cur_task_progress() == 0)
			return QString("Image %1/%2, task %3/%4: %5").arg(
				QString::number(cur_img_copy + 1),
				QString::number(files.size()),
				QString::number(cur_task_copy + 1),
				QString::number(tasks.size()),
				tasks[cur_task_copy]->get_desc()->to_string());
		// Task 1/1: Unknown task (100%).
		else
			return QString("Image %1/%2, task %3/%4: %5 (%6%)").arg(
				QString::number(cur_img_copy + 1),
				QString::number(files.size()),
				QString::number(cur_task_copy + 1),
				QString::number(tasks.size()),
				tasks[cur_task_copy]->get_desc()->to_string(),
				QString::number((unsigned short)(cur_task_progress() * 100.0F)));
	}
	else {
		return "Done!";
	}
}

void Worker::do_tasks(std::function<void()> success, std::function<void()> canceled,
					  std::function<void(QString)> error) {
	// Disable "cancel_requested" in all tasks.
	for (int i = 0; i < tasks.size(); i++)
		tasks[i]->cancel_requested = false;

	try {
		for (cur_img = 0; cur_img < files.size(); cur_img++) {
			// Read image.
			res_images[cur_img] = OIIO::ImageBuf(files[cur_img].toStdString());

			for (cur_task = 0; cur_task < tasks.size(); cur_task++) {
				auto temp_img = res_images[cur_img];
				res_images[cur_img] = tasks[cur_task]->do_task(temp_img);

				if (cancel_requested) {
					canceled();
					return;
				}
			}
		}
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
	success(); // If not canceled and no errors occured.
}

void Worker::save_images(QString folder_path, std::function<void(QString)> error) {
	for (int i = 0; i < res_images.size(); i++) {
		// Combine the name of the original file and the folder path.
		QString path = folder_path + '/' + files[i].section('/', -1, -1);
		if (!res_images[i].write(path.toStdString()))
			error(QString::fromStdString(res_images[i].geterror()));
	}
}

void Worker::cancel() {
	tasks[cur_task]->cancel_requested = true;
	cancel_requested = true;
}
