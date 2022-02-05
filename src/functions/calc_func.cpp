/*
 * ImageUpscalerQt - calculation functions
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "func.hpp"

int func::blocks_amount(QSize full_size, QSize block_size) {
	int blocks_width = full_size.width() / block_size.width();
	if (blocks_width * block_size.width() < full_size.width())
		blocks_width++;
	int blocks_height = full_size.height() / block_size.height();
	if (blocks_height * block_size.height() < full_size.height())
		blocks_height++;

	return blocks_height * blocks_width;
}

unsigned long long func::srcnn_operations_amount(SRCNNDesc desc,
											     QSize size) {
	// Use formula for it.
	// O=sum from{i=1} to{3} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}
	// (LibreOffice Math).

	unsigned long long result = 0;

	for (unsigned char i = 0; i < 3; i++) {
		result +=
			(unsigned long long)size.width() * size.height() * desc.channels[i + 1] *
			(2ull * desc.channels[i] * desc.kernels[i] * desc.kernels[i] - 1ull) +
			(unsigned long long)size.width() * size.height() * 1ull;
	}

	return result;
}

unsigned long long func::fsrcnn_operations_amount(FSRCNNDesc desc,
												  QSize size) {
	// Use formula for it.
	// O=sum from{i=1} to{n} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}
	// (LibreOffice Math).

	unsigned long long result = 0;

	// Initialize widths and heights.
	unsigned short nn_size = desc.kernels.size();
	std::vector<unsigned long long> widths(nn_size + 1);
	std::vector<unsigned long long> heights(nn_size + 1);
	for (unsigned short i = 0; i < nn_size + 1; i++) {
		widths[i] = i == nn_size ? size.width() * 3 : size.width();
		heights[i] = i == nn_size ? size.height() * 3 : size.height();
	}

	for (unsigned char i = 0; i < nn_size - 1; i++) {
		result +=
			widths[i] * heights[i] * desc.channels[i + 1] *
			(2ull * desc.channels[i] * desc.kernels[i] * desc.kernels[i] - 1ull) +
			(unsigned long long)widths[i + 1] * heights[i + 1] * 1ull;
	}

	return result;
}

/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
/// @returns Amount of bytes that will consumed.
unsigned long long func::predict_cnn_memory_consumption(SRCNNDesc desc,
													    QSize size) {
	std::vector<unsigned short> channels_vec(desc.channels.begin(), desc.channels.end());
	std::vector<QSize> sizes_vec(4, size);

	return predict_cnn_memory_consumption(channels_vec, sizes_vec);
}

/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
/// @returns Amount of bytes that will consumed.
unsigned long long func::predict_cnn_memory_consumption(FSRCNNDesc desc,
													    QSize size) {
	std::vector<unsigned short> channels_vec = desc.channels;
	std::vector<QSize> sizes_vec(desc.kernels.size() + 1, size);
	sizes_vec.back().rwidth() *= 3;
	sizes_vec.back().rheight() *= 3;

	return predict_cnn_memory_consumption(channels_vec, sizes_vec);
}

/// Predict the APPROXIMATE memory consumption of tensors that going throught the CNN.
/// @returns Amount of bytes that will consumed.
unsigned long long func::predict_cnn_memory_consumption(std::vector<unsigned short> channels,
														std::vector<QSize> sizes) {
	assert(channels.size() == sizes.size());

	// Iterate throught every convolutional layer to find the point with maximum memory consumption
	// considering the channels amount, width and height (in short, tensor size).
	// |  1  | --- | 128 | --- |  64 | --- |  1  |
	//                      ^
	//    max consumption point = size 1 + size 2

	unsigned long long max_point = 0;

	for (unsigned char i = 0; i < channels.size() - 1; i++) {
		unsigned long long cur_max_point = (long long)sizes[i].width() * sizes[i].height() * channels[i] +
										   (long long)sizes[i].width() * sizes[i + 1].height() *
										   channels[i + 1];

		if (cur_max_point > max_point)
			max_point = cur_max_point;
	}

	// Now, we have amount of numbers, but they are in float type, so multiply it to convert it to bytes.
	max_point *= sizeof(float);

	// In practice, we have a little greater consumption.
	max_point *= 2ull;

	return max_point;
}

// Windows implementation of free_physical_memory.
#ifdef Q_OS_WIN
#include <windows.h>

unsigned long long func::free_physical_memory() {
	// Use the Windows API for it.
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	return statex.ullAvailPhys;
}

#endif

// Linux implementation of free_physical_memory.
#ifdef Q_OS_LINUX
#include <QFile>

unsigned long long func::free_physical_memory() {
	// Read data from the /proc/meminfo pseudofile.
	QFile file("/proc/meminfo");
	QString meminfo = file.readAll();

	// Parse it.
	// Algorithm explanation:
	// "...\nMemAvailable:    12345 kB\n..." -> mid() ->
	// "    12345 kB" -> chopped() ->
	// "    12345" -> toULongLong() ->
	// 12345 -> *1000 ->
	// 12345000.
	auto start = meminfo.indexOf("MemAvailable:") + sizeof "MemAvailable:";
	return meminfo.mid(
		start,
		meminfo.indexOf('\n', start) - start
	).chopped(sizeof " kB").toULongLong() * 1000;
}

#endif
