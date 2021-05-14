#pragma once

#include <string>

enum class TaskKind : unsigned char {
	resize,
	srcnn,
	fsrcnn
};

struct Task {
	TaskKind task_kind;
	virtual std::string to_string(unsigned short index) = 0;
};

#include "TaskResize.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"
