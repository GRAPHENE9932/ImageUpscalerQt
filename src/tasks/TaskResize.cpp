/*
 * ImageUpscalerQt - resize task
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sstream>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "TaskResize.hpp"

TaskResize::TaskResize(TaskResizeDesc desc) : desc(desc) {}

OIIO::ImageBuf TaskResize::do_task(OIIO::ImageBuf input, std::function<void()> canceled) {
	// Create ROI.
	OIIO::ROI output_roi = OIIO::ROI(0, desc.size.width(), 0, desc.size.height(), 0, 1, 0, input.nchannels());

	// Resize it.
	OIIO::ImageBuf output;
	switch (desc.interpolation) {
		case Interpolation::bilinear: {
			output = OIIO::ImageBufAlgo::resample(input, true, output_roi);
			break;
		}
		default: {
			output = OIIO::ImageBufAlgo::resize(input,
				INTERPOLATION_OIIO_NAMES[static_cast<unsigned char>(desc.interpolation)],
				0.0f, output_roi);
		}
	}

	// Return result.
	return output;
}

const TaskDesc* TaskResize::get_desc() const {
	return dynamic_cast<const TaskDesc*>(&desc);
}
