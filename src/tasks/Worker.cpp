/*
 * ImageUpscalerQt - tasks worker
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "Worker.hpp"
#include "TaskResize.hpp"
#include "TaskConvertColorSpace.hpp"
#include "TaskSRCNN.hpp"
#include "TaskFSRCNN.hpp"

Worker::Worker() {

}

Worker::Worker(const std::vector<std::shared_ptr<TaskDesc>>& tasks,
			   const std::vector<std::pair<QString, QString>>& files) {
	init(tasks, files);
}

void Worker::init(std::vector<std::shared_ptr<TaskDesc>> task_descs,
				  std::vector<std::pair<QString, QString>> files) {
	// Construct tasks from theirs descriptions.
	tasks.resize(task_descs.size());
	for (int i = 0; i < task_descs.size(); i++) {
		const auto ptr = task_descs[i];

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
	return tasks[get_cur_task_index()]->progress();
}

float Worker::overall_progress() const {
	float cur_img_progress = (static_cast<float>(get_cur_task_index()) + cur_task_progress()) /
		static_cast<float>(tasks.size());
	return (static_cast<float>(get_cur_img_index()) + cur_img_progress) / files.size();
}

QString Worker::cur_status() const {
	// Save cur_task and cur_img for this function because
	// other thread can change these values during execution of this function.
	auto cur_task_copy = get_cur_task_index();
	auto cur_img_copy = get_cur_img_index();

	if (cur_task_copy == tasks.size() && cur_img_copy == files.size())
		return "Done!";

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
			QString::number(static_cast<int>(cur_task_progress() * 100.0f)));
}

int Worker::get_cur_task_index() const {
	if (cur_task >= tasks.size())
		return tasks.size() - 1;
	return cur_task;
}

int Worker::get_cur_img_index() const {
	if (cur_img >= files.size())
		return files.size() - 1;
	return cur_img;
}

void Worker::do_tasks(std::function<void()> success, std::function<void()> canceled,
					  std::function<void(QString)> error) {
	// Disable "cancel_requested" in all tasks.
	for (int i = 0; i < tasks.size(); i++)
		tasks[i]->cancel_requested = false;

#ifdef NDEBUG
	try {
#endif
		for (cur_img = 0; cur_img < files.size(); cur_img++) {
			// Read image.
			auto cur_img_buf = OIIO::ImageBuf(files[cur_img].first.toStdString());
			if (cur_img_buf.has_error()) {
				error(QString::fromStdString(
					"Can't read the image. The file may be inaccessible, "
					"in an unsupported format or damaged.\nMessage:\n"
					+ cur_img_buf.geterror()
				));
				return;
			}

			for (cur_task = 0; cur_task < tasks.size(); cur_task++) {
				auto temp_img_buf = cur_img_buf;
				cur_img_buf = tasks[cur_task]->do_task(temp_img_buf, canceled);

				if (cancel_requested) {
					canceled();
					return;
				}
			}
			// Write image.
			cur_img_buf.write(files[cur_img].second.toStdString());
			if (cur_img_buf.has_error()) {
				error(QString::fromStdString(
					"Can't write the image. The path may be non existent or "
					"inaccessible.\nMessage:\n"
					+ cur_img_buf.geterror()
				));
				return;
			}
		}
#ifdef NDEBUG
	}
	catch (const std::runtime_error& e) {
		error(e.what());
		return;
	}
	catch (const std::exception& e) {
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
	tasks[get_cur_task_index()]->cancel_requested = true;
	cancel_requested = true;
}
