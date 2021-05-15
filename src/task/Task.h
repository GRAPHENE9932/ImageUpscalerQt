#pragma once

#include <string>

enum class TaskKind : unsigned char {
	resize,
	srcnn,
	fsrcnn
};

class Task {
public:
	TaskKind task_kind;

	virtual std::string to_string(unsigned short index) = 0;

	virtual float progress() { return 0; };
	virtual unsigned char* do_task(unsigned char* input, int width, int height, unsigned char ch_n) = 0;
};

#include "TaskResize.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"
