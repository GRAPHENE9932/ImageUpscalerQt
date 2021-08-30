/*
 * ImageUpscalerQt algorithms
 * Copyright (C) 2021  Artem Kliminskyi <artemklim50@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>
#include <string>
#include <vector>

#include <QString>
#include <QStringList>

namespace Algorithms {
	QStringList split(QString str, QChar c);

	int char_count(QString str, QChar c);

	bool parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
					 std::array<unsigned short, 3>* paddings_out,
					 std::array<unsigned short, 4>* channels_out);

	bool parse_fsrcnn(QString str, std::array<unsigned short, 4>* kernels_out,
					  std::array<unsigned short, 4>* paddings_out,
					  std::array<unsigned short, 5>* channels_out);

	QString srcnn_to_string(std::array<unsigned short, 3> kernels,
							std::array<unsigned short, 4> channels);

	QString fsrcnn_to_string(std::array<unsigned short, 4> kernels,
							 std::array<unsigned short, 5> channels);

	unsigned long long srcnn_operations_amount(std::array<unsigned short, 3> kernels,
											   std::array<unsigned short, 4> channels,
											   std::array<int, 4> widths,
											   std::array<int, 4> heights);

	unsigned long long fsrcnn_operations_amount(std::array<unsigned short, 4> kernels,
												std::array<unsigned short, 5> channels,
												std::array<int, 5> widths,
												std::array<int, 5> heights);

	///Compute the minimum memory consumption of tensors that going throught the CNN
	///@returns Amount of bytes that will be consumed
	template <int S>
	unsigned long long cnn_memory_consumption(std::array<unsigned short, S> channels,
											  std::array<int, S> widths,
											  std::array<int, S> heights);

	///Convert "987654321" to "987 654 321" if separator is ' '
	QString big_number_to_string(long long num, QChar separator);

	///Convert "113246208" to "108.0 MiB"
	QString bytes_amount_to_string(unsigned long long bytes);
}
