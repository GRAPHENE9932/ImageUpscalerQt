#pragma once

#include "Task.h"

class Worker {
public:
	///Already finished image
	OIIO::ImageBuf finished_image;

	Worker();
	///Not only construct, but also init()
	Worker(std::vector<Task*> queue, std::string input_filename);

	///Needed if the worker was constructed with default constructor
	void init(std::vector<Task*> queue, std::string input_filename);
	std::string cur_status() const;
	///Progress of current task
	float cur_task_progress() const;
	///Progress of all tasks
	float overall_progress() const;

	void do_tasks(std::function<void()> success, std::function<void()> canceled,
				  std::function<void(std::string)> error);
	void save_image(std::string filename, std::function<void(std::string)> error);
	void cancel();

private:
	///Current image (can be unfinished)
	OIIO::ImageBuf cur_image;

	std::string input_filename;
	std::vector<Task*> tasks_queue;
	unsigned short cur_task = 0;

	bool cancel_requested = false;
};
