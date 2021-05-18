#include <sstream>

#include <torch/torch.h>
#include <QDir>

#include "TaskSRCNN.h"
#include "../nn/SRCNN.h"
#include "../Algorithms.h"

TaskSRCNN::TaskSRCNN() {
	this->task_kind = TaskKind::srcnn;
}

TaskSRCNN::TaskSRCNN(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
					 std::array<unsigned short, 2> ch_n) {
	this->task_kind = TaskKind::srcnn;

	this->kernels = kernels;
	this->paddings = paddings;
	this->channels = ch_n;
}

std::string TaskSRCNN::to_string(unsigned short index) const {
	std::stringstream ss;
	//1: use SRCNN 5-1-9 64-32
	ss << (index + 1) << ": use SRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << ' ' <<
	+channels[0] << '-' << +channels[1];

	return ss.str();
}

std::string TaskSRCNN::to_string() const {
	std::stringstream ss;
	//use SRCNN 5-1-9 64-32
	ss << "use SRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << ' ' <<
	+channels[0] << '-' << +channels[1];

	return ss.str();
}

std::string TaskSRCNN::parameters_path() const {
	std::stringstream ss;
	//"/path/to/program/SRCNN/5-1-9 64-32.pt"
	ss << QDir::currentPath().toStdString() << "/SRCNN/" <<
		Algorithms::srcnn_to_string(kernels, channels) << ".pt";
	return ss.str();
}

float TaskSRCNN::progress() const {
	return (float)blocks_processed / blocks_amount;
}

OIIO::ImageBuf TaskSRCNN::do_task(OIIO::ImageBuf input) {
	//Set num threads manually, else torch will use only about half of it
	torch::set_num_threads(std::thread::hardware_concurrency());

	//Initialize neural network
	SRCNN model(kernels, paddings, channels);
	torch::autograd::variable_list loaded_params;
	torch::load(loaded_params, parameters_path());
	for (uint64_t i = 0; i < loaded_params.size(); i++)
		model->parameters()[i].set_data(loaded_params[i]);

	//Get spec
	auto spec = input.spec();

	//Create output buffer
	OIIO::ImageBuf output(spec);

	//Get blocks amount
	int blocks_width = spec.width / 192;
	if (blocks_width * 192 < spec.width)
		blocks_width++;
	int blocks_height = spec.height / 192;
	if (blocks_height * 192 < spec.height)
		blocks_height++;
	blocks_amount = blocks_height * blocks_width * spec.nchannels;
	blocks_processed = 0;

	//Use SRCNN block by block
	for (int y = 0; y < spec.height; y += 192) {
		for (int x = 0; x < spec.width; x += 192) {
			for (int c = 0; c < spec.nchannels; c++) {
				//Create block roi
				OIIO::ROI block_extract_roi(x, x + 192, y, y + 192, 0, 1, c, c + 1);
				//Get block pixels. Will be planar, because single-channel
				auto block_pixels = std::make_unique<float[]>(192 * 192 * 1);
				input.get_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, block_pixels.get());

				//Create input tensor
				torch::TensorOptions options = torch::TensorOptions(torch::ScalarType::Float);
				torch::Tensor input_tensor = torch::from_blob(block_pixels.get(), {1, 1, 192, 192}, options);

				//Get output from neural network
				torch::Tensor output_tensor = model(input_tensor);

				//Set pixels to buf
				output.set_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, output_tensor.data_ptr<float>());

				blocks_processed++;

				//Cancel if requested
				if (cancel_requested)
					throw "canc";
			}
		}
	}

	return output;
}
