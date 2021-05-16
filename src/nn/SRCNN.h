#pragma once

#include <array>
#include <sstream>

#include <torch/torch.h>

namespace func = torch::nn::functional;

struct SRCNNImpl : torch::nn::Module {
	torch::nn::Conv2d conv_0, conv_1, conv_2;

public:
	SRCNNImpl(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
			  std::array<unsigned short, 2> channels);

	torch::Tensor forward(torch::Tensor x);
};

TORCH_MODULE(SRCNN);
