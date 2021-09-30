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

#include <torch/torch.h>
#include <QDir>
#include <QFile>

#include "TaskFSRCNN.h"
#include "../nn/FSRCNN.h"
#include "../Algorithms.h"

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
										    Algorithms::fsrcnn_to_string(kernels, channels));
}

QString TaskFSRCNN::to_string() const {
	//use FSRCNN 3-1-3-4 512-32-64
	return QString("use FSRCNN %1").arg(Algorithms::fsrcnn_to_string(kernels, channels));
}

float TaskFSRCNN::progress() const {
	return (float)blocks_processed / blocks_amount;
}

OIIO::ImageBuf TaskFSRCNN::do_task(OIIO::ImageBuf input) {
	//Set num threads manually, else torch will use only about half of it
	torch::set_num_threads(std::thread::hardware_concurrency());

	//Initialize the neural network
	FSRCNN model(kernels, paddings, channels);

	//Load archive with parameters from resources
	QFile file(":/FSRCNN/" + Algorithms::fsrcnn_to_string(kernels, channels) + ".pt");
	file.open(QFile::ReadOnly);
	QByteArray archive_array = file.read(536870912); //Maximum size is 512 MB

	//Transfer loaded data to variable_list
	torch::autograd::variable_list loaded_params;
	torch::load(loaded_params, archive_array.data(), archive_array.size(), torch::kCPU);
	for (uint64_t i = 0; i < loaded_params.size(); i++) //Transfer variable list to model parameters
		model->parameters()[i].set_data(loaded_params[i]);

	//Get spec
	auto spec = input.spec();
	const int block_width = block_size == 0 ? spec.width : block_size; //Whole image size if we have not to
	const int block_height = block_size == 0 ? spec.height : block_size; //split image into blocks

	//Create output buffer
	OIIO::ROI output_roi(0, spec.width * 3, 0, spec.height * 3, 0, 1, 0, spec.nchannels);
	OIIO::ImageSpec output_spec(output_roi, OIIO::TypeDesc::UINT8);
	OIIO::ImageBuf output(output_spec);

	//Get blocks amount
	int blocks_width = spec.width / block_width;
	if (blocks_width * block_width < spec.width)
		blocks_width++;
	int blocks_height = spec.height / block_height;
	if (blocks_height * block_height < spec.height)
		blocks_height++;
	blocks_amount = blocks_height * blocks_width * spec.nchannels;
	blocks_processed = 0;

	//Use FSRCNN block by block
	for (int y = 0; y < spec.height; y += block_height) {
		for (int x = 0; x < spec.width; x += block_width) {
			for (int c = 0; c < spec.nchannels; c++) {
				//Create block roi
				OIIO::ROI block_extract_roi(x, x + block_width, y, y + block_height, 0, 1, c, c + 1);
				//Get block pixels. Will be planar, because single-channel
				auto block_pixels = std::make_unique<float[]>(block_width * block_height * 1);
				input.get_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, block_pixels.get());

				//Create input tensor
				torch::TensorOptions options = torch::TensorOptions(torch::ScalarType::Float);
				//WARNING: potential error because of block_height and block_width order
				torch::Tensor input_tensor = torch::from_blob(block_pixels.get(),
															  {1, 1, block_height, block_width}, options);

				//Get output from neural network
				torch::Tensor output_tensor = model(input_tensor);

				//Set pixels to buf
				OIIO::ROI block_set_roi(x * 3, (x + block_width) * 3,
										y * 3, (y + block_height) * 3,
										0, 1,
										c, c + 1);
				output.set_pixels(block_set_roi, OIIO::TypeDesc::FLOAT, output_tensor.data_ptr<float>());

				blocks_processed++;

				//Cancel if requested
				if (cancel_requested)
					throw "canc";
			}
		}
	}

	return output;
}
