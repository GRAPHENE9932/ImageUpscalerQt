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

#include "../tasks/TaskDesc.h"

namespace func {
	// BEGIN String functions

	/// Find duplicates in the designated QStringList.
	/// @returns Indexes of duplicates.
	std::vector<int> duplicate_indexes(const QStringList& list);

	QStringList shorten_file_paths(const QStringList& list);

	/// Shorten path and leave only one level of the path (exception is X:),
	/// where X is uppercase letter of English alphabet.
	QString shorten_file_path(const QString path);

	void numerical_sort(QStringList& list);

	QString big_number_to_string(long long num, QChar separator = ' ');

	QString bytes_amount_to_string(unsigned long long bytes);
	// END String functions

	// BEGIN Calculation functions
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
