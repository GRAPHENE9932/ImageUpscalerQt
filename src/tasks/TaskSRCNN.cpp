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

#include "TaskSRCNN.hpp"
#include "../nn/SRCNN.hpp"
#include "../functions/func.hpp"

TaskSRCNN::TaskSRCNN(const TaskSRCNNDesc& desc) : desc(desc) {}

float TaskSRCNN::progress() const {
	return static_cast<float>(blocks_processed) / blocks_amount;
}

OIIO::ImageBuf TaskSRCNN::do_task(OIIO::ImageBuf input, std::function<void()> canceled) {
	// Get spec.
	auto spec = input.spec();
	// Whole image size if we have not to split image into blocks.
	const int block_width = desc.block_size == 0 ? spec.width : desc.block_size;
	const int block_height = desc.block_size == 0 ? spec.height : desc.block_size;

	// Create an output buffer.
	OIIO::ImageBuf output(spec);

	blocks_amount = func::blocks_amount(QSize(spec.width, spec.height),
										QSize(block_width, block_height)) * spec.nchannels;
	blocks_processed = 0;

	// Initialize the neural network.
	SRCNN nn = SRCNN::create(block_width, block_height, desc.srcnn_desc);
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
	QFile file(":/SRCNN/" + desc.srcnn_desc.to_string() + ".bin");
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
				if (cancel_requested) {
					canceled();
					return input;
				}
			}
		}
	}

	return output;
}

const TaskDesc* TaskSRCNN::get_desc() const {
	return dynamic_cast<const TaskDesc*>(&desc);
}
