/*
 * ImageUpscalerQt - task descriptions
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>
#include <vector>

enum class TaskKind : unsigned char {
	resize,
	convert_color_space,
	srcnn,
	fsrcnn
};

enum class Interpolation : unsigned char {
	b_spline, bilinear, blackman_harris, box, catmull_rom, cubic,
	gaussian, lanczos3, mitchell, radial_lanczos3,
	rifman, sharp_gaussian, simon, sinc
};

/// Interpolation names for the user.
const QString INTERPOLATION_NAMES[14] = {
	"B-spline", "Bilinear", "Blackman-Harris", "Box", "Catmull-Rom", "Cubic",
	"Gaussian", "Lanczos3", "Mitchell", "Radial-lanczos3",
	"Rifman", "Sharp-Gaussian", "Simon", "Sinc"
};

/// Interpolation names for the OpenImageIO library.
/// "none" and "bilinear" are not used.
const QString INTERPOLATION_OIIO_NAMES[14] = {
	"bspline", "bilinear", "blackman-harris", "box", "catmull-rom", "cubic",
	"gaussian", "lanczos3", "mitchell", "radial-lanczos3",
	"rifman", "sharp-gaussian", "simon", "sinc"
};

enum class ColorSpaceConversion : unsigned char {
	rgb_to_ycbcr, ycbcr_to_rgb, rgb_to_ycocg, ycocg_to_rgb
};

/// Color space names for the user.
const QString COLOR_SPACE_CONVERSION_NAMES[4] = {
	"RGB to YCbCr", "YCbCr to RGB", "RGB to YCoCg", "YCoCg to RGB"
};

struct TaskResizeDesc {
	Interpolation interpolation = Interpolation::bilinear;
	unsigned int x_size = 0, y_size = 0;

	TaskResizeDesc(Interpolation interpolation, unsigned int x_size, unsigned int y_size) :
		interpolation(interpolation), x_size(x_size), y_size(y_size) {}
};

struct TaskConvertColorSpaceDesc {
	ColorSpaceConversion color_space_conversion;

	TaskConvertColorSpaceDesc(ColorSpaceConversion color_space_conversion) :
		color_space_conversion(color_space_conversion) {}
};

struct TaskSRCNNDesc {
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 3> paddings;
	std::array<unsigned short, 4> channels;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	unsigned int block_size;

	TaskSRCNNDesc(std::array<unsigned short, 3> kernels,
				  std::array<unsigned short, 3> paddings,
				  std::array<unsigned short, 4> channels,
				  unsigned int block_size) :
				  kernels(kernels), paddings(paddings), channels(channels), block_size(block_size) {}
};

struct TaskFSRCNNDesc {
	std::vector<unsigned short> kernels;
	std::vector<unsigned short> paddings;
	std::vector<unsigned short> channels;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	unsigned int block_size;

	TaskFSRCNNDesc(std::vector<unsigned short> kernels,
				   std::vector<unsigned short> paddings,
				   std::vector<unsigned short> channels,
				   unsigned int block_size) :
				   kernels(kernels), paddings(paddings), channels(channels), block_size(block_size) {}
};
