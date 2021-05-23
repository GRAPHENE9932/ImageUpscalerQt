#pragma once

#include <string>

#include <OpenImageIO/imagebuf.h>

enum class TaskKind : unsigned char {
	resize,
	srcnn,
	fsrcnn,
	convert_color_space
};

class Task {
public:
	TaskKind task_kind;
	bool cancel_requested = false;

	virtual std::string to_string(unsigned short index) const = 0;
	virtual std::string to_string() const = 0;

	virtual float progress() const { return 0; };
	virtual OIIO::ImageBuf do_task(const OIIO::ImageBuf input) = 0;
};
