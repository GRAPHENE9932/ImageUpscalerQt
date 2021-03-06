/*
 * ImageUpscalerQt - FSRCNN task
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sstream>
#include <memory>
#include <cassert>

#include <QDir>
#include <QFile>
#include <OpenImageIO/imagebufalgo.h>

#include "TaskFSRCNN.hpp"
#include "../nn/FSRCNN.hpp"
#include "../functions/func.hpp"

TaskFSRCNN::TaskFSRCNN(const TaskFSRCNNDesc& desc) : desc(desc) {}

float TaskFSRCNN::progress() const {
	return static_cast<float>(blocks_processed) / blocks_amount;
}

OIIO::ImageBuf TaskFSRCNN::do_task(OIIO::ImageBuf input, std::function<void()> canceled) {
	const unsigned char& mul = desc.fsrcnn_desc.size_multiplier;
	const int& margin = desc.margin;

	// Get spec.
	auto spec = input.spec();
	// Whole image size if we don't have to split image into blocks.
	const int block_width = desc.block_size == 0 ? spec.width : desc.block_size;
	const int block_height = desc.block_size == 0 ? spec.height : desc.block_size;

	// Create the output buffer.
	const OIIO::ImageSpec out_spec(spec.width * mul, spec.height * mul, spec.nchannels);
	OIIO::ImageBuf output(out_spec);

	blocks_amount = func::blocks_amount(QSize(spec.width, spec.height),
										QSize(block_width, block_height), margin) * spec.nchannels;
	blocks_processed = 0;

	// Initialize the neural network.
	FSRCNN nn = FSRCNN(block_width - margin * 2, block_height - margin * 2, desc.fsrcnn_desc);
	const std::vector<dnnl::memory::desc> ker_descs = nn.get_ker_descs();
	const std::vector<dnnl::memory::desc> bias_descs = nn.get_bias_descs();
	const dnnl::memory::desc input_desc = nn.get_input_desc();
	const dnnl::memory::desc output_desc = nn.get_output_desc();
	const dnnl::engine eng = nn.get_engine();
	assert(ker_descs.size() == bias_descs.size());

	// Initialize full kernel and bias sizes in bytes.
	std::vector<size_t> full_ker_sizes(ker_descs.size());
	std::vector<size_t> full_bias_sizes(ker_descs.size());
	size_t total_params_size = 0;
	for (char i = 0; i < ker_descs.size(); i++) {
		full_ker_sizes[i] = ker_descs[i].get_size();
		full_bias_sizes[i] = bias_descs[i].get_size();
		total_params_size += full_ker_sizes[i] + full_bias_sizes[i];
	}

	// Load file with parameters from resources.
	QFile file(":/fsrcnn/" + desc.fsrcnn_desc.to_string() + ".bin");
	file.open(QFile::ReadOnly);
	QByteArray file_array = file.read(512 * 1024 * 1024); //Maximum size is 512 MiB
	assert(file_array.size() == total_params_size);

	// Load this array into dnnl::memory.
	std::vector<dnnl::memory> ker_mems(ker_descs.size());
	std::vector<dnnl::memory> bias_mems(bias_descs.size());
	size_t mem_offset = 0;
	for (char i = 0; i < ker_descs.size(); i++) {
		ker_mems[i] = dnnl::memory(ker_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_ker_sizes[i];
		bias_mems[i] = dnnl::memory(bias_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_bias_sizes[i];
	}

	// Use FSRCNN block by block.
	for (int y = 0; y < spec.height; y += block_height + margin * 2) {
		for (int x = 0; x < spec.width; x += block_width + margin * 2) {
			for (int c = 0; c < spec.nchannels; c++) {
				// Create block roi.
				OIIO::ROI block_roi_input(x + margin,
										  x - margin + block_width,
										  y + margin,
										  y - margin + block_height,
										  0, 1, c, c + 1);

				// Get block pixels. Planar, because we are working on single-channel image.
				auto block_pixels = std::make_unique<float[]>(
					(block_width - margin * 2) * (block_height - margin * 2) * 1
				);
				input.get_pixels(block_roi_input, OIIO::TypeDesc::FLOAT, block_pixels.get());

				// Create input memory.
				dnnl::memory input_mem = dnnl::memory(input_desc, eng, block_pixels.get());
				// Create output memory.
				dnnl::memory output_mem = dnnl::memory(output_desc, eng);
				// Get output from the neural network.
				nn.execute(input_mem, ker_mems, bias_mems, output_mem);

				// Set pixels to buf.
				const OIIO::ROI block_roi_net_output((x + margin) * mul, (x - margin + block_width) * mul,
					(y + margin) * mul, (y - margin + block_height) * mul,
					0, 1, c, c + 1);
				if (margin == 0) {
					output.set_pixels(block_roi_net_output, OIIO::TypeDesc::FLOAT, output_mem.get_data_handle());
				}
				else {
					OIIO::ROI block_roi(0, block_roi_net_output.width(),
										0, block_roi_net_output.height(),
										0, 1, 0, 1);
					OIIO::ImageSpec block_spec(block_roi, OIIO::TypeDesc::FLOAT);
					OIIO::ImageBuf block(block_spec, output_mem.get_data_handle());

					OIIO::ROI marginated_block_roi(-margin * mul, (block_width + margin) * mul,
												   -margin * mul, (block_height + margin) * mul,
												   0, 1, 0, 1);
					OIIO::ImageBufAlgo::paste(output, x * mul, y * mul, 0, c, block, marginated_block_roi);
				}

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

const TaskDesc* TaskFSRCNN::get_desc() const {
	return dynamic_cast<const TaskDesc*>(&desc);
}
