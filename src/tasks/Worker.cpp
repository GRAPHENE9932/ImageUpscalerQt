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

Worker::Worker(std::vector<std::shared_ptr<TaskDesc>> tasks,
			   std::vector<std::pair<QString, QString>> files) {
	init(tasks, files);
}

void Worker::init(std::vector<std::shared_ptr<TaskDesc>> tasks_descs,
				  std::vector<std::pair<QString, QString>> files) {
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
			auto cur_img_buf = OIIO::ImageBuf(files[cur_img].first.toStdString());

			for (cur_task = 0; cur_task < tasks.size(); cur_task++) {
				auto temp_img_buf = cur_img_buf;
				cur_img_buf = tasks[cur_task]->do_task(temp_img_buf);

				if (cancel_requested) {
					canceled();
					return;
				}
			}
			// Write image.
			// TODO: handle errors.
			cur_img_buf.write(files[cur_img].second.toStdString());
		}
	}
	catch (const char* str) {
		// TODO: better cancelation handling.
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

void Worker::cancel() {
	tasks[cur_task]->cancel_requested = true;
	cancel_requested = true;
}
