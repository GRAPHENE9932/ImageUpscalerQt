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

#include "Algorithms.h"

QStringList Algorithms::split(QString str, QChar c) {
	return str.split(c);
}

int Algorithms::char_count(QString str, QChar c) {
	return str.count(c);
}

bool Algorithms::parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
							 std::array<unsigned short, 3>* paddings_out,
							 std::array<unsigned short, 2>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 2> channels = {0, 0};

		//BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 3)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 3; i++) {
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

		for (uint8_t i = 0; i < 2; i++)
			channels[i] = splitted_ch[i].toUShort();
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
							  std::array<unsigned short, 3>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 4> kernels = {0, 0, 0, 0};
		std::array<unsigned short, 4> paddings = {0, 0, 0, 0};
		std::array<unsigned short, 3> channels = {0, 0, 0};

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
			channels[i] = splitted_ch[i].toUShort();
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
									const std::array<unsigned short, 2> channels) {
	//5-1-9 64-32
	return QString("%1-%2-%3 %4-%5").arg(QString::number(kernels[0]),
										 QString::number(kernels[1]),
										 QString::number(kernels[2]),
										 QString::number(channels[0]),
										 QString::number(channels[1]));
}

QString Algorithms::fsrcnn_to_string(const std::array<unsigned short, 4> kernels,
									 const std::array<unsigned short, 3> channels) {
	//3-1-3-4 512-32-64
	return QString("%1-%2-%3-%4 %5-%6-%7").arg(QString::number(kernels[0]),
										       QString::number(kernels[1]),
										       QString::number(kernels[2]),
										       QString::number(kernels[3]),
										       QString::number(channels[0]),
										       QString::number(channels[1]),
										       QString::number(channels[2]));
}

unsigned long long Algorithms::srcnn_operations_amount(std::array<unsigned short, 3> kernels,
													   std::array<unsigned short, 2> channels_short) {
	//Use formula for it
	//Formula: "O= sum from{i=1} to{3} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 )  + {W_{ i+1 }^2 times O_a}".
	//(LibreOffice Math)
	const std::array<unsigned short, 4> channels = {1, channels_short[0], channels_short[1], 1};

	unsigned long long result = 0;
	for (unsigned char i = 0; i < 3; i++) {
		result += (192 * 192) * (long long)channels[i + 1] *
			(2 * (long long)channels[i] * (long long)kernels[i] * (long long)kernels[i] - 1);
		result += (192 * 192);
	}

	return result;
}

unsigned long long Algorithms::fsrcnn_operations_amount(std::array<unsigned short, 4> kernels,
														std::array<unsigned short, 3> channels_short) {
	//Use formula for it
	//Formula: "O = W_5^2 times C_5 times K_4^2( 2C_4-1 )+ color green { sum from{i=1} to{3} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) +
	//W_{ i+1 }^2 times O_a}". (LibreOffice Math)
	const std::array<unsigned short, 5> sizes = {64, 64, 64, 64, 128};
	const std::array<unsigned short, 5> channels = {1, channels_short[0], channels_short[1], channels_short[2], 1};

	unsigned long long result = 0;
	for (unsigned char i = 0; i < 3; i++)
		result += (long long)sizes[i + 1] * (long long)sizes[i + 1] * (long long)channels[i + 1] *
		(2 * (long long)channels[i] * (long long)kernels[i] * (long long)kernels[i] - 1) +
		(long long)sizes[i + 1] * (long long)sizes[i + 1] * 1;
	result += (long long)sizes[4] * (long long)sizes[4] * (long long)channels[4] * (long long)kernels[3] *
	(long long)kernels[3] * (2 * (long long)channels[3] - 1);

	return result;
}

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
