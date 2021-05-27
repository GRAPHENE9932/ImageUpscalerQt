#include "Worker.h"

Worker::Worker() {

}

Worker::Worker(std::vector<Task*> queue, std::string input_filename) {
	init(queue, input_filename);
}

void Worker::init(std::vector<Task*> queue, std::string input_filename) {
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

std::string Worker::cur_status() const {
	//Save cur_task for this function, because multithreading
	auto cur_task_copy = cur_task;
	if (cur_task_copy < tasks_queue.size()) {
		//Prepare text for current task label
		std::stringstream ss;
		//Task 1/1: Unknown task (100%)
		ss << "Task " << cur_task_copy + 1 << '/' << tasks_queue.size() << ": " << tasks_queue[cur_task_copy]->to_string();
		//Add "(xxx%)" if not 0%
		if (cur_task_progress() != 0)
			ss << " (" << (int)(cur_task_progress() * 100.0F) << "%)";

		return ss.str();
	}
	else {
		return "Done!";
	}
}

void Worker::do_tasks(std::function<void()> success, std::function<void()> canceled,
					  std::function<void(std::string)> error) {
	try {
		//Read image
		OIIO::ImageBuf input_buf = OIIO::ImageBuf(input_filename);

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
	success(); //If not canceled and no errors occured
}

void Worker::save_image(std::string filename, std::function<void(std::string)> error) {
	if (!finished_image.write(filename))
		error(cur_image.geterror());
}

void Worker::cancel() {
	cancel_requested = true;
}
