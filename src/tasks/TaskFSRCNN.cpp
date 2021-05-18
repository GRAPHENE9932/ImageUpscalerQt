#include <sstream>

#include <torch/torch.h>
#include <QDir>

#include "TaskFSRCNN.h"
#include "../nn/FSRCNN.h"
#include "../Algorithms.h"

TaskFSRCNN::TaskFSRCNN() {
	this->task_kind = TaskKind::fsrcnn;
}

TaskFSRCNN::TaskFSRCNN(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
					   std::array<unsigned short, 3> channels) {
	this->task_kind = TaskKind::fsrcnn;

	//Assign arrays
	this->kernels = kernels;

	this->paddings = paddings;

	this->channels = channels;
}

std::string TaskFSRCNN::to_string(unsigned short index) const {
	std::stringstream ss;
	//1: use FSRCNN 3-1-3-4 64-32-32
	ss << (index + 1) << ": use FSRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << '-' << +kernels[3] << ' ' <<
	+channels[0] << '-' << +channels[1] << '-' << +channels[2];

	return ss.str();
}

std::string TaskFSRCNN::to_string() const {
	std::stringstream ss;
	//use FSRCNN 3-1-3-4 64-32-32
	ss << "use FSRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << '-' << +kernels[3] << ' ' <<
	+channels[0] << '-' << +channels[1] << '-' << +channels[2];

	return ss.str();
}

std::string TaskFSRCNN::parameters_path() const {
	std::stringstream ss;
	//"/path/to/program/SRCNN/5-1-9 64-32.pt"
	ss << QDir::currentPath().toStdString() << "/FSRCNN/" <<
	Algorithms::fsrcnn_to_string(kernels, channels) << ".pt";
	return ss.str();
}

float TaskFSRCNN::progress() const {
	return (float)blocks_processed / blocks_amount;
}

OIIO::ImageBuf TaskFSRCNN::do_task(OIIO::ImageBuf input) {
	//Set num threads manually, else torch will use only about half of it
	torch::set_num_threads(std::thread::hardware_concurrency());

	//Initialize neural network
	FSRCNN model(kernels, paddings, channels);
	torch::autograd::variable_list loaded_params;
	torch::load(loaded_params, parameters_path());
	for (uint64_t i = 0; i < loaded_params.size(); i++)
		model->parameters()[i].set_data(loaded_params[i]);

	//Get spec
	auto spec = input.spec();

	//Create output buffer
	OIIO::ROI output_roi(0, spec.width * 2, 0, spec.height * 2, 0, 1, 0, spec.nchannels);
	OIIO::ImageSpec output_spec(output_roi, OIIO::TypeDesc::UINT8);
	OIIO::ImageBuf output(output_spec);

	//Get blocks amount
	int blocks_width = spec.width / 64;
	if (blocks_width * 64 < spec.width)
		blocks_width++;
	int blocks_height = spec.height / 64;
	if (blocks_height * 64 < spec.height)
		blocks_height++;
	blocks_amount = blocks_height * blocks_width * spec.nchannels;
	blocks_processed = 0;

	//Use SRCNN block by block
	for (int y = 0; y < spec.height; y += 64) {
		for (int x = 0; x < spec.width; x += 64) {
			for (int c = 0; c < spec.nchannels; c++) {
				//Create block roi
				OIIO::ROI block_extract_roi(x, x + 64, y, y + 64, 0, 1, c, c + 1);
				//Get block pixels. Will be planar, because single-channel
				auto block_pixels = std::make_unique<float[]>(64 * 64 * 1);
				input.get_pixels(block_extract_roi, OIIO::TypeDesc::FLOAT, block_pixels.get());

				//Create input tensor
				torch::TensorOptions options = torch::TensorOptions(torch::ScalarType::Float);
				torch::Tensor input_tensor = torch::from_blob(block_pixels.get(), {1, 1, 64, 64}, options);

				//Get output from neural network
				torch::Tensor output_tensor = model(input_tensor);

				//Set pixels to buf
				OIIO::ROI block_set_roi(x * 2, (x + 64) * 2, y * 2, (y + 64) * 2, 0, 1, c, c + 1);
				output.set_pixels(block_set_roi, OIIO::TypeDesc::FLOAT, output_tensor.data_ptr<float>());

				blocks_processed++;
			}
		}
	}

	return output;
}
