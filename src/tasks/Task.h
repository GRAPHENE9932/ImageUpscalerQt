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

	virtual std::string to_string(unsigned short index) const = 0;

	virtual float progress() const { return 0; };
	virtual OIIO::ImageBuf do_task(const OIIO::ImageBuf input) = 0;
};
