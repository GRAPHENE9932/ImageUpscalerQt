#pragma once

#include "TaskResize.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"

enum class TaskKind : unsigned char {
	none = 0,
	resize = 1,
	srcnn = 2,
	fsrcnn = 3
};

struct Task {
	TaskKind task_kind = TaskKind::none;
};
