#pragma once

#include <sstream>
#include <array>

#include <torch/torch.h>

using namespace torch::nn;

struct FSRCNNImpl : torch::nn::Module {
	Conv2d conv_0, conv_1, conv_2;
	ConvTranspose2d conv_trans_3;

public:
	FSRCNNImpl(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
			   std::array<unsigned short, 3> channels);

	torch::Tensor forward(torch::Tensor x);
};

TORCH_MODULE(FSRCNN);
