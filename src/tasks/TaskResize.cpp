/*
 * ImageUpscalerQt - resize task
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sstream>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "TaskResize.h"

TaskResize::TaskResize(TaskResizeDesc desc) : desc(desc) {}

QString TaskResize::to_string(unsigned short index) const {
	// 1: resize to 1920x1080 | Bilinear.
	return QString("%1: resize to %2x%3 | %4").arg(QString::number(index + 1),
												   QString::number(desc.x_size),
												   QString::number(desc.y_size),
												   INTERPOLATION_NAMES[(unsigned char)desc.interpolation]);
}

QString TaskResize::to_string() const {
	// resize to 1920x1080 | Bilinear.
	return QString("resize to %1x%2 | %3").arg(QString::number(desc.x_size),
											   QString::number(desc.y_size),
											   INTERPOLATION_NAMES[(unsigned char)desc.interpolation]);
}

OIIO::ImageBuf TaskResize::do_task(OIIO::ImageBuf input) {
	// Create ROI.
	OIIO::ROI output_roi = OIIO::ROI(0, desc.x_size, 0, desc.y_size, 0, 1, 0, input.nchannels());

	// Resize it.
	OIIO::ImageBuf output;
	switch (desc.interpolation) {
		case Interpolation::bilinear: {
			output = OIIO::ImageBufAlgo::resample(input, true, output_roi);
			break;
		}
		default: {
			output = OIIO::ImageBufAlgo::resize(input,
				INTERPOLATION_OIIO_NAMES[(unsigned char)desc.interpolation].toStdString(),
				0.0F, output_roi);
		}
	}

	// Return result.
	return output;
}
