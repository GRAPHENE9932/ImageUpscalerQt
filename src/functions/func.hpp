/*
 * ImageUpscalerQt - functions header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>
#include <vector>

#include <QString>
#include <QStringList>
#include <QSize>

#include "../tasks/TaskDesc.hpp"

namespace func {
	// BEGIN String functions

	/// Find duplicates in the designated QStringList.
	/// @returns Indexes of duplicates.
	std::vector<int> duplicate_indexes(const QStringList& list);

	void numerical_sort(QStringList& list);

	/// Example
	/// orig = "HELLOWORLD", spl_char = '-', sec_size = 3, right_start = 1.
	/// Outputs "HEL-LOW-ORLD".
	QString separate_string_with_char(QString orig, QChar spl_char, int sec_size, int right_start);
	QString big_number_to_string(long long num, QChar separator = ' ');

	QString bytes_amount_to_string(unsigned long long bytes);
	QString pixel_amount_to_string(unsigned long long pixels);
	QString milliseconds_to_string(unsigned long long millis);

	QString shorten_file_path(const QString& orig);

	QString get_image_input_wildcard();
	QString get_image_output_wildcard();
	// END String functions

	// BEGIN Calculation functions
	/// How many blocks (block_size) will fit in the full image (full_size).
	int blocks_amount(const QSize full_size, const QSize block_size, const int block_margin = 0);

	unsigned long long srcnn_operations_amount(SRCNNDesc desc, QSize size);
	unsigned long long fsrcnn_operations_amount(FSRCNNDesc desc, QSize size);

	/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
	/// @returns Amount of bytes that will consumed.
	unsigned long long predict_cnn_memory_consumption(SRCNNDesc desc, QSize size);

	/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
	/// @returns Amount of bytes that will consumed.
	unsigned long long predict_cnn_memory_consumption(FSRCNNDesc desc, QSize size);

	/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
	/// @returns Amount of bytes that will consumed.
	unsigned long long predict_cnn_memory_consumption(std::vector<unsigned short> channels,
													  std::vector<QSize> sizes);

	/// Get free physical memory in bytes.
	unsigned long long free_physical_memory();

	// END Calculation functions
}
