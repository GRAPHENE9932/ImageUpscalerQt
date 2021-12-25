/*
 * ImageUpscalerQt FSRCNN task
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
#include <memory>
#include <cassert>

#include <QDir>
#include <QFile>

#include "TaskFSRCNN.h"
#include "../nn/FSRCNN.h"
#include "../functions/func.h"

TaskFSRCNN::TaskFSRCNN() {
	this->task_kind = TaskKind::fsrcnn;
}

TaskFSRCNN::TaskFSRCNN(std::vector<unsigned short> kernels, std::vector<unsigned short> paddings,
					   std::vector<unsigned short> channels, unsigned int block_size) {
	this->task_kind = TaskKind::fsrcnn;

	//Assign arrays
	this->kernels = kernels;

	this->paddings = paddings;

	this->channels = channels;

	this->block_size = block_size;
}

QString TaskFSRCNN::to_string(unsigned short index) const {
	//1: use FSRCNN 3-1-3-4 512-32-64
	return QString("%1: use FSRCNN %2").arg(QString::number(index + 1),
										    func::fsrcnn_to_string(kernels, channels));
}

QString TaskFSRCNN::to_string() const {
	//use FSRCNN 3-1-3-4 512-32-64
	return QString("use FSRCNN %1").arg(func::fsrcnn_to_string(kernels, channels));
}

float TaskFSRCNN::progress() const {
	return (float)blocks_processed / blocks_amount;
}

OIIO::ImageBuf TaskFSRCNN::do_task(OIIO::ImageBuf input) {
	//Get spec
	auto spec = input.spec();
	const int block_width = block_size == 0 ? spec.width : block_size; //Whole image size if we have not to
	const int block_height = block_size == 0 ? spec.height : block_size; //split image into blocks

	//Create output buffer
	const OIIO::ImageSpec out_spec(spec.width * 3, spec.height * 3, spec.nchannels);
	OIIO::ImageBuf output(out_spec);

	//Compute blocks amount
	int blocks_width = spec.width / block_width;
	if (blocks_width * block_width < spec.width)
		blocks_width++;
	int blocks_height = spec.height / block_height;
	if (blocks_height * block_height < spec.height)
		blocks_height++;
	blocks_amount = blocks_height * blocks_width * spec.nchannels;
	blocks_processed = 0;

	//Initialize the neural network
	FSRCNN nn = FSRCNN::create(block_width, block_height, kernels, channels);
	const std::vector<dnnl::memory::desc> ker_descs = nn.get_ker_descs();
	const std::vector<dnnl::memory::desc> bias_descs = nn.get_bias_descs();
	const dnnl::memory::desc input_desc = nn.get_input_desc();
	const dnnl::memory::desc output_desc = nn.get_output_desc();
	const dnnl::engine eng = nn.get_engine();
	assert(ker_descs.size() == bias_descs.size());

	//Initialize full kernel and bias sizes in bytes
	std::vector<size_t> full_ker_sizes(ker_descs.size());
	std::vector<size_t> full_bias_sizes(ker_descs.size());
	size_t total_params_size = 0;
	for (char i = 0; i < ker_descs.size(); i++) {
		full_ker_sizes[i] = ker_descs[i].get_size();
		full_bias_sizes[i] = bias_descs[i].get_size();
		total_params_size += full_ker_sizes[i] + full_bias_sizes[i];
	}

	//Load file with parameters from resources
	QFile file(":/FSRCNN/" + func::fsrcnn_to_string(kernels, channels) + ".bin");
	file.open(QFile::ReadOnly);
	QByteArray file_array = file.read(512 * 1024 * 1024); //Maximum size is 512 MiB
	assert(file_array.size() == total_params_size);

	//Load this array into dnnl::memory
	std::vector<dnnl::memory> ker_mems(ker_descs.size());
	std::vector<dnnl::memory> bias_mems(bias_descs.size());
	size_t mem_offset = 0;
	for (char i = 0; i < ker_descs.size(); i++) {
		ker_mems[i] = dnnl::memory(ker_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_ker_sizes[i];
		bias_mems[i] = dnnl::memory(bias_descs[i], eng, file_array.data() + mem_offset);
		mem_offset += full_bias_sizes[i];
	}

	//Use SRCNN block by block
	for (int y = 0; y < spec.height; y += block_height) {
		for (int x = 0; x < spec.width; x += block_width) {
			for (int c = 0; c < spec.nchannels; c++) {
				//Create block roi
				OIIO::ROI block_extract_roi_in(x, x + block_width,
											   y, y + block_height,
											   0, 1, c, c + 1);
				OIIO::ROI block_extract_roi_out(x * 3, (x + block_width) * 3,
												y * 3, (y + block_height) * 3,
												0, 1, c, c + 1);
				//Get block pixels. Planar, because we are working on single-channel image.
				auto block_pixels = std::make_unique<float[]>(block_width * block_height * 1);
				input.get_pixels(block_extract_roi_in, OIIO::TypeDesc::FLOAT, block_pixels.get());

				//Create input memory
				dnnl::memory input_mem = dnnl::memory(input_desc, eng, block_pixels.get());

				//Create output memory
				dnnl::memory output_mem = dnnl::memory(output_desc, eng);

				//Get output from the neural network
				nn.execute(input_mem, ker_mems, bias_mems, output_mem);

				//Set pixels to buf
				output.set_pixels(block_extract_roi_out, OIIO::TypeDesc::FLOAT, output_mem.get_data_handle());

				blocks_processed++;

				//Cancel if requested
				if (cancel_requested)
					throw "canc";
			}
		}
	}

	return output;
}
