/*
 * ImageUpscalerQt - functions header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>
#include <vector>

#include <QString>

namespace func {
	// BEGIN String functions
	QStringList split(QString str, QChar c);

	int char_count(QString str, QChar c);

	bool parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
					 std::array<unsigned short, 3>* paddings_out,
					 std::array<unsigned short, 4>* channels_out);

	bool parse_fsrcnn(QString str, std::vector<unsigned short>* kernels_out,
					  std::vector<unsigned short>* paddings_out,
					  std::vector<unsigned short>* channels_out);

	QString srcnn_to_string(const std::array<unsigned short, 3> kernels,
							const std::array<unsigned short, 4> channels);

	QString fsrcnn_to_string(const std::vector<unsigned short> kernels,
							 const std::vector<unsigned short> channels);

	void numerical_sort(QStringList& list);

	QString big_number_to_string(long long num, QChar separator);

	QString bytes_amount_to_string(unsigned long long bytes);
	// END String functions

	// BEGIN Calculation functions
	unsigned long long srcnn_operations_amount(std::array<unsigned short, 3> kernels,
											   std::array<unsigned short, 4> channels,
											   std::array<int, 4> widths,
											   std::array<int, 4> heights);

	unsigned long long fsrcnn_operations_amount(std::vector<unsigned short> kernels,
												std::vector<unsigned short> channels,
												std::vector<int> widths,
												std::vector<int> heights);

	unsigned long long predict_cnn_memory_consumption(std::array<unsigned short, 4> channels,
													  std::array<int, 4> widths,
													  std::array<int, 4> heights);

	/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
	/// @returns Amount of bytes that will consumed.
	unsigned long long predict_cnn_memory_consumption(std::vector<unsigned short> channels,
													  std::vector<int> widths,
													  std::vector<int> heights);
	// END Calculation functions
}
