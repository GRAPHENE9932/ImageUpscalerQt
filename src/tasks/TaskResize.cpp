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

QString TaskResize::to_string(unsigned short index) const {
	//1: resize to 1920x1080 | Bilinear
	return QString("%1: resize to %2x%3 | %4").arg(QString(index + 1), QString(x_size), QString(y_size),
												   INTERPOLATION_NAMES[(unsigned char)interpolation]);
}

QString TaskResize::to_string() const {
	//resize to 1920x1080 | Bilinear
	return QString("resize to %1x%2 | %3").arg(QString(x_size), QString(y_size),
											   INTERPOLATION_NAMES[(unsigned char)interpolation]);
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
			output = OIIO::ImageBufAlgo::resize(input,
				INTERPOLATION_OIIO_NAMES[(unsigned char)interpolation].toStdString(),
				0.0F, output_roi);
		}
	}

	//Return result
	return output;
}
