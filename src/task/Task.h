#pragma once

enum class TaskKind : unsigned char {
	resize,
	srcnn,
	fsrcnn
};

struct Task {
	TaskKind task_kind;
};

#include "TaskResize.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"
