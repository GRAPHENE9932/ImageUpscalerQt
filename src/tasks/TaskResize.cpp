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

unsigned char* TaskResize::do_task(unsigned char* input, int width, int height, unsigned char ch_n) {
	//Create specs
	OIIO::ROI input_roi = OIIO::ROI(0, width, 0, height, 0, 1, 0, ch_n);
	OIIO::ROI output_roi = OIIO::ROI(0, x_size, 0, y_size, 0, 1, 0, ch_n);
	OIIO::ImageSpec input_spec = OIIO::ImageSpec(input_roi, OIIO::TypeDesc::UINT8);

	//Create buf
	OIIO::ImageBuf input_buf = OIIO::ImageBuf(input_spec, input);

	//Resize it
	OIIO::ImageBuf output_buf;
	switch (interpolation) {
		case Interpolation::none: {
			output_buf = OIIO::ImageBufAlgo::resample(input_buf, false, output_roi);
			break;
		}
		case Interpolation::bilinear: {
			output_buf = OIIO::ImageBufAlgo::resample(input_buf, true, output_roi);
			break;
		}
		default: {
			output_buf = OIIO::ImageBufAlgo::resize(input_buf,
													INTERPOLATION_OIIO_NAMES[(unsigned char)interpolation],
													0.0F, output_roi);
		}
	}

	//Extract pixels from output_buf
	unsigned char* output = new unsigned char[output_roi.width() * output_roi.height() * output_roi.nchannels()];
	output_buf.get_pixels(output_buf.roi(), OIIO::TypeDesc::UINT8, output);

	//Return result
	return output;
}
