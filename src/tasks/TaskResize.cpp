#include <sstream>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "TaskResize.h"

TaskResize::TaskResize() {
	this->task_kind = TaskKind::resize;
}

TaskResize::TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation) {
	this->task_kind = TaskKind::resize;

	this->x_size = x_size;
	this->y_size = y_size;
	this->interpolation = interpolation;
}

std::string TaskResize::to_string(unsigned short index) {
	std::stringstream ss;
	//1: resize to 1920x1080 | Bilinear
	ss << (index + 1) << ": resize to " << x_size << 'x' << y_size << " | " <<
	INTERPOLATION_NAMES[(unsigned char)interpolation];

	return ss.str();
}

OIIO::ImageBuf TaskResize::do_task(OIIO::ImageBuf input) {
	//Create ROI
	OIIO::ROI output_roi = OIIO::ROI(0, x_size, 0, y_size, 0, 1, 0, input.nchannels());

	//Resize it
	OIIO::ImageBuf output;
	switch (interpolation) {
		case Interpolation::none: {
			output = OIIO::ImageBufAlgo::resample(input, false, output_roi);
			break;
		}
		case Interpolation::bilinear: {
			output = OIIO::ImageBufAlgo::resample(input, true, output_roi);
			break;
		}
		default: {
			output = OIIO::ImageBufAlgo::resize(input, INTERPOLATION_OIIO_NAMES[(unsigned char)interpolation],
												0.0F, output_roi);
		}
	}

	//Return result
	return output;
}
