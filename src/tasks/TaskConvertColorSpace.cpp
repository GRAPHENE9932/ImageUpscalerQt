#include <OpenImageIO/imagebufalgo.h>

#include "TaskConvertColorSpace.h"

TaskConvertColorSpace::TaskConvertColorSpace() {
	this->task_kind = TaskKind::convert_color_space;
}

TaskConvertColorSpace::TaskConvertColorSpace(ColorSpaceConversion color_space_conversion) {
	this->task_kind = TaskKind::convert_color_space;
	this->color_space_conversion = color_space_conversion;
}

std::string TaskConvertColorSpace::to_string(unsigned short index) const {
	std::stringstream ss;
	//1: convert from RGB to YCbCr
	ss << index << ": convert from " <<
	COLOR_SPACE_CONVERSION_NAMES[(unsigned char)color_space_conversion];

	return ss.str();
}

std::string TaskConvertColorSpace::to_string() const {
	std::stringstream ss;
	//convert from RGB to YCbCr
	ss << "convert from " <<
	COLOR_SPACE_CONVERSION_NAMES[(unsigned char)color_space_conversion];

	return ss.str();
}

//BEGIN Coefficients
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
//END Coefficients

void add_to_existing(float* object, const float* operand, const size_t size) {
	for (size_t i = 0; i < size; i++)
		object[i] += operand[i];
}

void add_to_existing(float* object, const float operand, const size_t size) {
	for (size_t i = 0; i < size; i++)
		object[i] += operand;
}

std::unique_ptr<float[]> add(const float* o1, const float o2, const size_t size) {
	std::unique_ptr<float[]> result = std::make_unique<float[]>(size);

	for (size_t i = 0; i < size; i++)
		result[i] = o1[i] + o2;

	return result;
}

std::unique_ptr<float[]> multiply(const float* o1, const float o2, const size_t size) {
	std::unique_ptr<float[]> result = std::make_unique<float[]>(size);

	for (size_t i = 0; i < size; i++)
		result[i] = o1[i] * o2;

	return result;
}

OIIO::ImageBuf TaskConvertColorSpace::do_task(OIIO::ImageBuf input) {
	if (input.nchannels() < 3)
		throw std::runtime_error(std::string("Only 3 or more channel images are allowed to conversion. Provided: ")
		+ std::to_string(input.nchannels()));

	//Get pointer to the needed matrix
	const float (*matrix_p)[3][3];
	const float (*offset_after_p)[3] = nullptr;
	const float (*offset_before_p)[3] = nullptr;
	switch (color_space_conversion) {
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

	//Split input to separate channels
	size_t channel_size = input.spec().width * input.spec().height;
	std::array<std::unique_ptr<float[]>, 3> inputs;
	for (unsigned char i = 0; i < 3; i++) {
		OIIO::ROI roi(0, input.spec().width, 0, input.spec().height, 0, 1, i, i + 1);
		inputs[i] = std::make_unique<float[]>(channel_size);
		input.get_pixels(roi, OIIO::TypeDesc::FLOAT, inputs[i].get());
	}

	//Output will be also splitted
	//Init outputs as float arrays
	std::array<std::unique_ptr<float[]>, 3> outputs;
	for (unsigned char i = 0; i < 3; i++)
		outputs[i] = std::make_unique<float[]>(input.spec().width * input.spec().height);

	for (unsigned char c_out = 0; c_out < 3; c_out++) {
		//if (offset_before_p != nullptr)
			//add_to_existing(outputs[c_out].get(), (*offset_before_p)[c_out], channel_size); //Offset before

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
		}

		if (offset_after_p != nullptr)
			add_to_existing(outputs[c_out].get(), (*offset_after_p)[c_out], channel_size); //Offset after
	}

	//Merge single-channel outputs to triple-channel one
	OIIO::ImageBuf output;
	output = input;
	for (unsigned char i = 0; i < 3; i++) {
		OIIO::ROI cur_roi(0, output.spec().width, 0, output.spec().height, 0, 1, i, i + 1);
		output.set_pixels(cur_roi, OIIO::TypeDesc::FLOAT, outputs[i].get());
	}
	//Last channels - without changes

	return output;
}
