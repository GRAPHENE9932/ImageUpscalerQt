/*
 * ImageUpscalerQt - convert color space task
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <OpenImageIO/imagebufalgo.h>

#include "TaskConvertColorSpace.hpp"

TaskConvertColorSpace::TaskConvertColorSpace(TaskConvertColorSpaceDesc desc) : desc(desc) {}

float TaskConvertColorSpace::progress() const {
	return progress_val;
}

inline void rgb_to_ycbcr(float* src, float* dest, size_t size) {
	assert(size % 3 == 0);
	for (size_t i = 0; i < size; i += 3) {
		dest[i + 0] = 0.299f * src[i] + 0.587f * src[i + 1] + 0.114f * src[i + 2];
		dest[i + 1] = 0.5f - 0.168736f * src[i] - 0.331264f * src[i + 1] + 0.5f * src[i + 2];
		dest[i + 2] = 0.5f + 0.5f * src[i] - 0.418688f * src[i + 1] - 0.081312f * src[i + 2];
	}
}

inline void ycbcr_to_rgb(float* src, float* dest, size_t size) {
	assert(size % 3 == 0);
	for (size_t i = 0; i < size; i += 3) {
		dest[i + 0] = src[i] + 1.402f * (src[i + 2] - 0.5f);
		dest[i + 1] = src[i] - 0.344136f * (src[i + 1] - 0.5f) - 0.714136f * (src[i + 2] - 0.5f);
		dest[i + 2] = src[i] + 1.772f * (src[i + 1] - 0.5f);
	}
}

inline void rgb_to_ycocg(float* src, float* dest, size_t size) {
	assert(size % 3 == 0);
	for (size_t i = 0; i < size; i += 3) {
		dest[i + 0] = 0.25f * src[i] + 0.5f * src[i + 1] + 0.25f * src[i + 2];
		dest[i + 1] = 0.5f + 0.5f * src[i] - 0.5f * src[i + 2];
		dest[i + 2] = 0.5f + -0.25f * src[i] + 0.5f * src[i + 1] - 0.25f * src[i + 2];
	}
}

inline void ycocg_to_rgb(float* src, float* dest, size_t size) {
	assert(size % 3 == 0);
	for (size_t i = 0; i < size; i += 3) {
		dest[i + 0] = src[i] + src[i + 1] - src[i + 2];
		dest[i + 1] = src[i] + src[i + 2] - 0.5f;
		dest[i + 2] = src[i] - src[i + 1] - src[i + 2] + 1.0f;
	}
}

OIIO::ImageBuf TaskConvertColorSpace::do_task(OIIO::ImageBuf input, std::function<void()> canceled) {
	if (input.nchannels() < 3)
		throw std::runtime_error(
			QString("Only 3 or more channel images are allowed to conversion. Provided %1").arg(QString::number(input.nchannels())).toStdString());

	OIIO::ROI roi = input.roi();
	roi.chend = 4;

	size_t pix_count = input.spec().width * input.spec().height * 3;

	OIIO::ImageBuf output = input;

	std::unique_ptr<float[]> src_data = std::make_unique<float[]>(pix_count);
	input.get_pixels(roi, OIIO::TypeDesc::FLOAT, src_data.get());

	std::unique_ptr<float[]> dst_data = std::make_unique<float[]>(pix_count);

	switch (desc.color_space_conversion) {
		case ColorSpaceConversion::rgb_to_ycbcr: {
			rgb_to_ycbcr(src_data.get(), dst_data.get(), pix_count);
			break;
		}
		case ColorSpaceConversion::ycbcr_to_rgb: {
			ycbcr_to_rgb(src_data.get(), dst_data.get(), pix_count);
			break;
		}
		case ColorSpaceConversion::rgb_to_ycocg: {
			rgb_to_ycocg(src_data.get(), dst_data.get(), pix_count);
			break;
		}
		case ColorSpaceConversion::ycocg_to_rgb: {
			ycocg_to_rgb(src_data.get(), dst_data.get(), pix_count);
			break;
		}
	}

	output.set_pixels(roi, OIIO::TypeDesc::FLOAT, dst_data.get());
	return output;
}

const TaskDesc* TaskConvertColorSpace::get_desc() const {
	return &desc;
}
