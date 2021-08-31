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

#include <sstream>
#include <cassert>

#include "Algorithms.h"

QStringList Algorithms::split(QString str, QChar c) {
	return str.split(c);
}

int Algorithms::char_count(QString str, QChar c) {
	return str.count(c);
}

bool Algorithms::parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
							 std::array<unsigned short, 3>* paddings_out,
							 std::array<unsigned short, 4>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 4> channels = {1, 0, 0, 1};

		//BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 3)
			return false;

		bool bad_params = false;

		for (unsigned char i = 0; i < 3; i++) {
			//Parse name
			kernels[i] = splitted_ker[i].toUShort();
			paddings[i] = (kernels[i] - 1) / 2; //Formulas simplified from (w - k + 2p) / s + 1
			bad_params |= kernels[i] % 2 != 1;
		}

		if (bad_params)
			return false;
		//END

		//BEGIN Parse second part (channels)
		QStringList splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 2)
			return false;

		for (unsigned char i = 0; i < 2; i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		//END

		//Return values
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool Algorithms::parse_fsrcnn(QString str, std::array<unsigned short, 4>* kernels_out,
							  std::array<unsigned short, 4>* paddings_out,
							  std::array<unsigned short, 5>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 4> kernels = {0, 0, 0, 0};
		std::array<unsigned short, 4> paddings = {0, 0, 0, 0};
		std::array<unsigned short, 5> channels = {1, 0, 0, 0, 1};

		//BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 4)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 4; i++) {
			//Parse name
			kernels[i] = splitted_ker[i].toUShort();
			if (i == splitted_ker.size() - 1) { //If last (last layer - deconvolutional)
				paddings[i] = (kernels[i] - 2) / 2;
				bad_params |= kernels[i] % 2 != 0;
			}
			else {
				paddings[i] = (kernels[i] - 1) / 2; //Formulas simplified from (w - k + 2p) / s + 1
				bad_params |= kernels[i] % 2 != 1;
			}
		}

		if (bad_params)
			return false;

		//END

		//BEGIN Parse second part (channels)
		QStringList splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 3)
			return false;

		for (uint8_t i = 0; i < 3; i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		//END

		//Return values
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

QString Algorithms::srcnn_to_string(const std::array<unsigned short, 3> kernels,
									const std::array<unsigned short, 4> channels) {
	//5-1-9 64-32
	return QString("%1-%2-%3 %4-%5").arg(QString::number(kernels[0]),
										 QString::number(kernels[1]),
										 QString::number(kernels[2]),
										 QString::number(channels[1]),
										 QString::number(channels[2]));
}

QString Algorithms::fsrcnn_to_string(const std::array<unsigned short, 4> kernels,
									 const std::array<unsigned short, 5> channels) {
	//3-1-3-4 512-32-64
	return QString("%1-%2-%3-%4 %5-%6-%7").arg(QString::number(kernels[0]),
										       QString::number(kernels[1]),
										       QString::number(kernels[2]),
										       QString::number(kernels[3]),
										       QString::number(channels[1]),
										       QString::number(channels[2]),
										       QString::number(channels[3]));
}

unsigned long long Algorithms::srcnn_operations_amount(std::array<unsigned short, 3> kernels,
													   std::array<unsigned short, 4> channels,
													   std::array<int, 4> widths,
													   std::array<int, 4> heights) {
	//Use formula for it.
	//O=sum from{i=1} to{3} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}

	unsigned long long result = 0;

	for (unsigned char i = 0; i < 3; i++) {
		result += (unsigned long long)widths[i + 1] * (unsigned long long)heights[i + 1] *
			(unsigned long long)channels[i + 1] *
			(2 * (unsigned long long)channels[i] * (unsigned long long)kernels[i] * (unsigned long long)kernels[i] - 1) +
			(unsigned long long)widths[i + 1] * (unsigned long long)heights[i + 1] *
			1;
	}

	return result;
}

unsigned long long Algorithms::fsrcnn_operations_amount(std::array<unsigned short, 4> kernels,
														std::array<unsigned short, 5> channels,
														std::array<int, 5> widths,
														std::array<int, 5> heights) {
	//Use formula for it
	//O=sum from{i=1} to{5} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}

	unsigned long long result = 0;

	for (unsigned char i = 0; i < 5; i++) {
		result += (unsigned long long)widths[i + 1] * (unsigned long long)heights[i + 1] *
			(unsigned long long)channels[i + 1] *
			(2 * (unsigned long long)channels[i] * (unsigned long long)kernels[i] * (unsigned long long)kernels[i] - 1) +
			(unsigned long long)widths[i + 1] * (unsigned long long)heights[i + 1] *
			1;
	}

	return result;
}

template <int S>
unsigned long long Algorithms::measure_cnn_memory_consumption(std::array<unsigned short, S> channels,
													  std::array<int, S> widths,
													  std::array<int, S> heights) {
	//Iterate throught every convolutional layer to find the point with maximum memory consumption
	//Considering the channels amount, width and height (in short, tensor size)
	// |  1  | --- | 128 | --- |  64 | --- |  1  |
	//                      ^
	//    max consumption point = size 1 + size 2

	unsigned long long max_point = 0;

	for (unsigned char i = 0; i < S - 1; i++) {
		unsigned long long cur_max_point = widths[i] * heights[i] * channels[i] +
										   widths[i + 1] * heights[i + 1] * channels[i + 1];

		if (cur_max_point > max_point)
			max_point = cur_max_point;
	}

	//Now, we have amount of numbers, but they are in float type, so multiply it to convert it to bytes
	max_point *= sizeof(float);

	//Weirdness of the PyTorch. The real consumption is 4 times greater than calculated just now. So...
	max_point *= 4;

	return max_point;
}

template unsigned long long Algorithms::measure_cnn_memory_consumption<4>(std::array<unsigned short,4>,
																  std::array<int,4>,
																  std::array<int,4>);
template unsigned long long Algorithms::measure_cnn_memory_consumption<5>(std::array<unsigned short,5>,
																  std::array<int,5>,
																  std::array<int,5>);

QString Algorithms::big_number_to_string(long long num, QChar separator) {
	//Get separate digits
	QString original = QString::number(num);

	//Do this
	QString result = "";
	for (int i = 0; i < original.size(); i++) {
		if ((original.size() - i) % 3 == 0 && i != 0 && i != original.size() - 1 && original[i] != '-')
			result += separator;

		result += original[i];
	}

	return result;
}

QString Algorithms::bytes_amount_to_string(unsigned long long bytes)
{
	if (bytes < 1024) { //Bytes
		return QString::number(bytes) + " B";
	}
	else if (bytes < 1024 * 1024) { //Kilobytes
		return QString::number(bytes / 1024.0, 'f', 1) + " KiB";
	}
	else if (bytes < 1024 * 1024 * 1024) { //Megabytes
		return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MiB";
	}
	else { //Gigabytes
		return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GiB";
	}
}
