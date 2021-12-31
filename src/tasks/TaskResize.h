/*
 * ImageUpscalerQt - resize task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.h"

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

class TaskResize : public Task {
public:
	unsigned int x_size = 0, y_size = 0;
	Interpolation interpolation = Interpolation::bilinear;

	TaskResize();
	TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation);

	/// "1: resize to 1920x1080 | Bilinear".
	QString to_string(unsigned short index) const override;
	/// "resize to 1920x1080 | Bilinear".
	QString to_string() const override;

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;
};
