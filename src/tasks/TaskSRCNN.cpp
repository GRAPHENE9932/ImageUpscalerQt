/*
 * ImageUpscalerQt - SRCNN task
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sstream>
#include <memory>
#include <cassert>

#include <QDir>
#include <QFile>

#include "TaskSRCNN.h"
#include "../nn/SRCNN.h"
#include "../functions/func.h"

TaskSRCNN::TaskSRCNN() {
	this->task_kind = TaskKind::srcnn;
}

TaskSRCNN::TaskSRCNN(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
					 std::array<unsigned short, 4> ch_n, unsigned int block_size) {
	this->task_kind = TaskKind::srcnn;

	this->kernels = kernels;
	this->paddings = paddings;
	this->channels = ch_n;
	this->block_size = block_size;
}

QString TaskSRCNN::to_string(unsigned short index) const {
	// 1: use SRCNN 5-1-9 64-32.
	return QString("%1: use SRCNN %2").arg(QString::number(index + 1),
										   func::srcnn_to_string(kernels, channels));
}

QString TaskSRCNN::to_string() const {
	// use SRCNN 5-1-9 64-32.
	return QString("use SRCNN %1").arg(func::srcnn_to_string(kernels, channels));
}

float TaskSRCNN::progress() const {
	return (float)blocks_processed / blocks_amount;
}

OIIO::ImageBuf TaskSRCNN::do_task(OIIO::ImageBuf input) {
	// Get spec.
	auto spec = input.spec();
	const int block_width = block_size == 0 ? spec.width : block_size; //Whole image size if we have not to
	const int block_height = block_size == 0 ? spec.height : block_size; //split image into blocks

	// Create an output buffer.
	OIIO::ImageBuf output(spec);

	// Compute the blocks amount.
	int blocks_width = spec.width / block_width;
	if (blocks_width * block_width < spec.width)
		blocks_width++;
	int blocks_height = spec.height / block_height;
	if (blocks_height * block_height < spec.height)
		blocks_height++;
	blocks_amount = blocks_height * blocks_width * spec.nchannels;
	blocks_processed = 0;

	// Initialize the neural network.
	SRCNN nn = SRCNN::create(block_width, block_height, kernels, channels);
	const std::array<dnnl::memory::desc, 3> ker_descs = nn.get_ker_descs();
	const std::array<dnnl::memory::desc, 3> bias_descs = nn.get_bias_descs();
	const dnnl::memory::desc input_desc = nn.get_input_desc();
	const dnnl::memory::desc output_desc = nn.get_output_desc();
	const dnnl::engine eng = nn.get_engine();
	assert(ker_descs.size() == bias_descs.size());

	// Initialize full kernel and bias sizes (in bytes).
	std::array<size_t, 3> full_ker_sizes;
	std::array<size_t, 3> full_bias_sizes;
	size_t total_params_size = 0;
	for (char i = 0; i < 3; i++) {
		full_ker_sizes[i] = ker_descs[i].get_size();
		full_bias_sizes[i] = bias_descs[i].get_size();
		total_params_size += full_ker_sizes[i] + full_bias_sizes[i];
	}

	// Load file with parameters from resources.
	QFile file(":/SRCNN/" + func::srcnn_to_string(kernels, channels) + ".bin");
	file.open(QFile::ReadOnly);
	QByteArray file_array = file.read(512 * 1024 * 1024); // Maximum size is 512 MiB.
	assert(file_array.size() == total_params_size);

	// Load this array into dnnl::memory.
	std::array<dnnl::memory, 3> ker_mems;
	std::array<dnnl::memory, 3> bias_mems;
	size_t mem_offset = 0;
	for (char i = 0; i < 3; i++) {
		ker_mems[i] = dnnl::memory(ker_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_ker_sizes[i];
		bias_mems[i] = dnnl::memory(bias_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_bias_sizes[i];
	}

	// Use SRCNN block by block.
	for (int y = 0; y < spec.height; y += block_height) {
		for (int x = 0; x < spec.width; x += block_width) {
			for (int c = 0; c < spec.nchannels; c++) {
				// Create block roi.
				OIIO::ROI block_extract_roi(x, x + block_width, y, y + block_height, 0, 1, c, c + 1);
				// Get block pixels. Planar, because we are working on single-channel image.
				auto block_pixels = std::make_unique<float[]>(block_width * block_height * 1);
				input.get_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, block_pixels.get());

				// Create input memory.
				dnnl::memory input_mem = dnnl::memory(input_desc, eng, block_pixels.get());

				// Create output memory.
				dnnl::memory output_mem = dnnl::memory(output_desc, eng);

				// Get output from the neural network.
				nn.execute(input_mem, ker_mems, bias_mems, output_mem);

				// Set pixels to buf.
				output.set_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, output_mem.get_data_handle());

				blocks_processed++;

				// Cancel if requested.
				if (cancel_requested)
					throw "canc";
			}
		}
	}

	return output;
}
