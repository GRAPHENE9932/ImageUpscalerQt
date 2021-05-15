#pragma once

#include <string>

#include <OpenImageIO/imagebuf.h>

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
	virtual OIIO::ImageBuf do_task(OIIO::ImageBuf input) = 0;
};

#include "TaskResize.h"
#include "TaskSRCNN.h"
#include "TaskFSRCNN.h"
