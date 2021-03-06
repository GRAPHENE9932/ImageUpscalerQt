/*
 * ImageUpscalerQt - task descriptions header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>
#include <vector>

#include <QString>
#include <QSize>

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
const char* const INTERPOLATION_NAMES[14] = {
	"B-spline", "Bilinear", "Blackman-Harris", "Box", "Catmull-Rom", "Cubic",
	"Gaussian", "Lanczos3", "Mitchell", "Radial-lanczos3",
	"Rifman", "Sharp-Gaussian", "Simon", "Sinc"
};

/// Interpolation names for the OpenImageIO library.
/// "none" and "bilinear" are not used.
const char* const INTERPOLATION_OIIO_NAMES[14] = {
	"bspline", "bilinear", "blackman-harris", "box", "catmull-rom", "cubic",
	"gaussian", "lanczos3", "mitchell", "radial-lanczos3",
	"rifman", "sharp-gaussian", "simon", "sinc"
};

enum class ColorSpaceConversion : unsigned char {
	rgb_to_ycbcr, ycbcr_to_rgb, rgb_to_ycocg, ycocg_to_rgb
};

/// Color space names for the user.
const char* const COLOR_SPACE_CONVERSION_NAMES[4] = {
	"RGB to YCbCr", "YCbCr to RGB", "RGB to YCoCg", "YCoCg to RGB"
};

struct TaskDesc {
	virtual ~TaskDesc() = default;

	virtual QString to_string() const = 0;

	virtual QSize img_size_after(QSize cur_size) const = 0;

	virtual TaskKind task_kind() const = 0;
};

struct TaskResizeDesc : TaskDesc {
	Interpolation interpolation = Interpolation::bilinear;
	QSize size;

	TaskResizeDesc() = default;

	TaskResizeDesc(Interpolation interpolation, QSize size) :
		interpolation(interpolation), size(size) {}

	~TaskResizeDesc() = default;

	QString to_string() const override;

	QSize img_size_after(QSize cur_size) const override;

	TaskKind task_kind() const override {
		return TaskKind::resize;
	}
};

struct TaskConvertColorSpaceDesc : TaskDesc {
	ColorSpaceConversion color_space_conversion;

	TaskConvertColorSpaceDesc() = default;

	explicit TaskConvertColorSpaceDesc(ColorSpaceConversion color_space_conversion) :
		color_space_conversion(color_space_conversion) {}

	~TaskConvertColorSpaceDesc() = default;

	QString to_string() const override;

	QSize img_size_after(QSize cur_size) const override;

	TaskKind task_kind() const override {
		return TaskKind::convert_color_space;
	}
};

struct SRCNNDesc {
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 4> channels;

	SRCNNDesc() = default;

	SRCNNDesc(std::array<unsigned short, 3> kernels,
			  std::array<unsigned short, 4> channels) :
			  kernels(kernels), channels(channels) {}

	QString to_string() const;
	/// Parse SRCNN. Returns true if parsing is successful.
	/// Returns false if it is impossible to parse.
	/// Pass nullptr as pointer for desc to validate if it is valid SRCNN description string.
	static bool from_string(QString str, SRCNNDesc* desc);

	bool operator< (const SRCNNDesc& right) const;
	bool operator> (const SRCNNDesc& right) const;
	bool operator== (const SRCNNDesc& right) const;
};

struct TaskSRCNNDesc : TaskDesc {
	SRCNNDesc srcnn_desc;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	int block_size;

	TaskSRCNNDesc(const SRCNNDesc& srcnn_desc, unsigned int block_size) :
				  srcnn_desc(srcnn_desc), block_size(block_size) {}

	TaskSRCNNDesc(std::array<unsigned short, 3> kernels,
				  std::array<unsigned short, 4> channels,
				  unsigned int block_size) :
				  srcnn_desc(kernels, channels), block_size(block_size) {}

	~TaskSRCNNDesc() = default;

	QString to_string() const override;

	QSize img_size_after(QSize cur_size) const override;

	TaskKind task_kind() const override {
		return TaskKind::srcnn;
	}
};

struct FSRCNNDesc {
	std::vector<unsigned short> kernels;
	std::vector<unsigned short> channels;
	unsigned char size_multiplier;

	FSRCNNDesc() = default;

	FSRCNNDesc(const std::vector<unsigned short>& kernels,
			   const std::vector<unsigned short>& channels,
			   unsigned char size_multiplier) :
			   kernels(kernels), channels(channels), size_multiplier(size_multiplier) {}

	QString to_string(bool with_multiplier = true) const;
	/// Parse FSRCNN. Returns true if parsing is successful.
	/// Returns false if it is impossible to parse.
	/// Pass nullptr as pointer for desc to validate if it is valid FSRCNN description string.
	static bool from_string(QString str, FSRCNNDesc* desc);

	bool operator< (const FSRCNNDesc& right) const;
	bool operator> (const FSRCNNDesc& right) const;
	bool operator== (const FSRCNNDesc& right) const;
};

struct TaskFSRCNNDesc : TaskDesc {
	FSRCNNDesc fsrcnn_desc;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	unsigned int block_size;
	/// Margin for every block (even if input image must not be splitted).
	/// Positive value creates gaps between the blocks with width of margin * 2.
	/// Negative value crops pixels around the borders in every block.
	/// Usually used to remove artifacts around the borders.
	int margin;

	TaskFSRCNNDesc(const FSRCNNDesc& fsrcnn_desc,
				   unsigned int block_size,
				   int margin = 0) :
				   fsrcnn_desc(fsrcnn_desc),
				   block_size(block_size),
				   margin(margin) {};

	TaskFSRCNNDesc(const std::vector<unsigned short>& kernels,
				   const std::vector<unsigned short>& channels,
				   unsigned char size_multiplier,
				   unsigned int block_size,
				   int margin = 0) :
				   fsrcnn_desc(kernels, channels, size_multiplier),
				   block_size(block_size),
				   margin(margin) {};

	~TaskFSRCNNDesc() = default;

	QString to_string() const override;

	QSize img_size_after(QSize cur_size) const override;

	TaskKind task_kind() const override {
		return TaskKind::fsrcnn;
	}
};
