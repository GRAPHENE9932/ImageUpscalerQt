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

// BEGIN Coefficients
const float ycbcr_offset[3] {0, 0.5F, 0.5F};
const float ycbcr_offset_neg[3] {-ycbcr_offset[0], -ycbcr_offset[1], -ycbcr_offset[2]};
const float rgb_to_ycbcr_coef[3][3] {
	{0.299F, 0.587F, 0.114F},
	{-0.168736F, -0.331264F, 0.5F},
	{0.5F, -0.418688F, -0.081312F}
};

const float ycbcr_to_rgb_coef[3][3] {
	{1.0F, 0, 1.402F},
	{1.0F, -0.344136F, -0.714136F},
	{1.0F, 1.772F, 0}
};

const float ycocg_offset[3] {0, 0.5F, 0.5F};
const float ycocg_offset_neg[3] {-ycocg_offset[0], -ycocg_offset[1], -ycocg_offset[2]};
const float rgb_to_ycocg_coef[3][3] {
	{0.25F, 0.5F, 0.25F},
	{0.5F, 0, -0.5F},
	{-0.25F, 0.5F, -0.25F}
};

const float ycocg_to_rgb_coef[3][3] {
	{1, 1, -1},
	{1, 0, 1},
	{1, -1, -1}
};
// END Coefficients

// BEGIN Algebra
void add_to_existing_single_thread(float* object, const float* operand, const size_t size) {
	for (size_t i = 0; i < size; i++)
		object[i] += operand[i];
}

void add_to_existing(float* object, const float* operand, const size_t size) {
	// Calculate threads amount.
	unsigned short threads_num = std::thread::hardware_concurrency();
	if (threads_num > size)
		threads_num = size;

	// Calculate batch size.
	size_t batch_size = size / threads_num;

	// Start every thread.
	std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(threads_num);
	for (unsigned short i = 0; i < threads_num; i++) {
		threads[i] = std::thread(
			static_cast<void(*)(float*, const float*, const size_t)>(add_to_existing_single_thread),
			&object[i * batch_size],
			&operand[i * batch_size],
			batch_size
		);
	}

	/*
	   +--------------------------------------------------------------+
	   |   BATCH 0  |   BATCH 1   |   BATCH 2   |   BATCH 3   |remains|
	   |                            size                              |
	   +--------------------------------------------------------------+
	*/

	// Do remains by self.
	for (size_t i = batch_size * threads_num; i < size; i++)
		object[i] += operand[i];

	// Join all threads.
	for (unsigned short i = 0; i < threads_num; i++)
		threads[i].join();
}

void add_to_existing_single_thread(float* object, const float operand, const size_t size) {
	for (size_t i = 0; i < size; i++)
		object[i] += operand;
}

void add_to_existing(float* object, const float operand, const size_t size) {
	// Calculate threads amount.
	unsigned short threads_num = std::thread::hardware_concurrency();
	if (threads_num > size)
		threads_num = size;

	// Calculate batch size.
	size_t batch_size = size / threads_num;

	// Start every thread.
	std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(threads_num);
	for (unsigned short i = 0; i < threads_num; i++) {
		threads[i] = std::thread(
			static_cast<void(*)(float*, const float, const size_t)>(add_to_existing_single_thread),
			&object[i * batch_size],
			operand,
			batch_size
		);
	}

	/*
	   + ---------------*---------------------------------------------+
	   |   BATCH 0  |   BATCH 1   |   BATCH 2   |   BATCH 3   |remains|
	   |                            size                              |
	   +--------------------------------------------------------------+
	*/

	// Do remains by self.
	for (size_t i = batch_size * threads_num; i < size; i++)
		object[i] += operand;

	// Join all threads.
	for (unsigned short i = 0; i < threads_num; i++)
		threads[i].join();
}

void add_single_thread(float* dest, const float* o1, const float o2, const size_t size) {
	for (size_t i = 0; i < size; i++)
		dest[i] = o1[i] + o2;
}

std::unique_ptr<float[]> add(const float* o1, const float o2, const size_t size) {
	// Initialize result.
	std::unique_ptr<float[]> result = std::make_unique<float[]>(size);

	// Calculate threads amount.
	unsigned short threads_num = std::thread::hardware_concurrency();
	if (threads_num > size)
		threads_num = size;

	// Calculate batch size.
	size_t batch_size = size / threads_num;

	// Start every thread.
	std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(threads_num);
	for (unsigned short i = 0; i < threads_num; i++) {
		threads[i] = std::thread(
			static_cast<void(*)(float*, const float*, const float, const size_t)>(add_single_thread),
			&result.get()[i * batch_size],
			&o1[i * batch_size],
			o2,
			batch_size
		);
	}

	/*
	   +--------------------------------------------------------------+
	   |   BATCH 0  |   BATCH 1   |   BATCH 2   |   BATCH 3   |remains|
	   |                            size                              |
	   +--------------------------------------------------------------+
	*/

	// Do remains by self.
	for (size_t i = batch_size * threads_num; i < size; i++)
		result[i] = o1[i] + o2;

	// Join all threads.
	for (unsigned short i = 0; i < threads_num; i++)
		threads[i].join();

	// Return result.
	return result;
}

void multiply_single_thread(float* dest, const float* o1, float o2, size_t size) {
	for (size_t i = 0; i < size; i++)
		dest[i] = o1[i] * o2;
}

std::unique_ptr<float[]> multiply(const float* o1, const float o2, const size_t size) {
	// Initialize result.
	std::unique_ptr<float[]> result = std::make_unique<float[]>(size);

	// Calculate threads amount.
	unsigned short threads_num = std::thread::hardware_concurrency();
	if (threads_num > size)
		threads_num = size;

	// Calculate batch size.
	size_t batch_size = size / threads_num;

	// Start every thread.
	std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(threads_num);
	for (unsigned short i = 0; i < threads_num; i++) {
		threads[i] = std::thread(
			static_cast<void(*)(float*, const float*, const float, const size_t)>(multiply_single_thread),
			&result.get()[i * batch_size],
			&o1[i * batch_size],
			o2,
			batch_size
		);
	}

	/*
	   + -------------------------------------------------------------+
	   |   BATCH 0  |   BATCH 1   |   BATCH 2   |   BATCH 3   |remains|
	   |                            size                              |
	   +--------------------------------------------------------------+
	*/

	// Do remains by self.
	for (size_t i = batch_size * threads_num; i < size; i++)
		result[i] = o1[i] * o2;

	// Join all threads.
	for (unsigned short i = 0; i < threads_num; i++)
		threads[i].join();

	// Return result.
	return result;
}

//END Algebra

OIIO::ImageBuf TaskConvertColorSpace::do_task(OIIO::ImageBuf input, std::function<void()> canceled) {
	if (input.nchannels() < 3)
		throw std::runtime_error(
			QString("Only 3 or more channel images are allowed to conversion. Provided %1").arg(QString::number(input.nchannels())).toStdString());

	// Get pointer to the needed matrix.
	const float (*matrix_p)[3][3];
	const float (*offset_after_p)[3] = nullptr;
	const float (*offset_before_p)[3] = nullptr;
	switch (desc.color_space_conversion) {
		case ColorSpaceConversion::rgb_to_ycbcr: {
			matrix_p = &rgb_to_ycbcr_coef;
			offset_after_p = &ycbcr_offset;
			break;
		}
		case ColorSpaceConversion::ycbcr_to_rgb: {
			matrix_p = &ycbcr_to_rgb_coef;
			offset_before_p = &ycbcr_offset_neg;
			break;
		}
		case ColorSpaceConversion::rgb_to_ycocg: {
			matrix_p = &rgb_to_ycocg_coef;
			offset_after_p = &ycocg_offset;
			break;
		}
		case ColorSpaceConversion::ycocg_to_rgb: {
			matrix_p = &ycocg_to_rgb_coef;
			offset_before_p = &ycocg_offset_neg;
			break;
		}
	}

	// Split input to separate channels.
	size_t channel_size = input.spec().width * input.spec().height;
	std::array<std::unique_ptr<float[]>, 3> inputs;
	for (unsigned char i = 0; i < 3; i++) {
		OIIO::ROI roi(0, input.spec().width, 0, input.spec().height, 0, 1, i, i + 1);
		inputs[i] = std::make_unique<float[]>(channel_size);
		input.get_pixels(roi, OIIO::TypeDesc::FLOAT, inputs[i].get());
	}

	progress_val = 0.1F;

	// Output will be also splitted.
	// Init outputs as float arrays.
	std::array<std::unique_ptr<float[]>, 3> outputs;
	for (unsigned char i = 0; i < 3; i++)
		outputs[i] = std::make_unique<float[]>(input.spec().width * input.spec().height);

	for (unsigned char c_out = 0; c_out < 3; c_out++) {
		for (unsigned char c_in = 0; c_in < 3; c_in++) {
			float cur_val = (*matrix_p)[c_out][c_in];

			std::unique_ptr<float[]> modified_input;
			if (offset_before_p != nullptr) {
				modified_input = add(inputs[c_in].get(), (*offset_before_p)[c_in], channel_size);
			}
			else {
				modified_input = std::make_unique<float[]>(channel_size);
				memcpy(modified_input.get(), inputs[c_in].get(), channel_size * sizeof(float));
			}

			add_to_existing(outputs[c_out].get(),
							multiply(modified_input.get(), cur_val, channel_size).get(),
							channel_size);

			// Cancel if requested.
			if (cancel_requested) {
				canceled();
				return input;
			}
		}

		if (offset_after_p != nullptr)
			add_to_existing(outputs[c_out].get(), (*offset_after_p)[c_out], channel_size); // Offset after.

		progress_val = 0.1F + 0.9F / (3 - c_out);
	}

	// Merge single-channel outputs to triple-channel one.
	OIIO::ImageBuf output;
	output = input;
	for (unsigned char i = 0; i < 3; i++) {
		OIIO::ROI cur_roi(0, output.spec().width, 0, output.spec().height, 0, 1, i, i + 1);
		output.set_pixels(cur_roi, OIIO::TypeDesc::FLOAT, outputs[i].get());
	}
	// Other channels - without changes.
	progress_val = 1.0F;

	return output;
}

const TaskDesc* TaskConvertColorSpace::get_desc() const {
	return dynamic_cast<const TaskDesc*>(&desc);
}
