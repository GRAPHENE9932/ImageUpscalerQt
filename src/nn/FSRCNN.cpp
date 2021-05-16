#include "FSRCNN.h"

namespace func = torch::nn::functional;

FSRCNNImpl::FSRCNNImpl(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
	std::array<unsigned short, 3> channels) :
	conv_0(torch::nn::Conv2dOptions(1, channels[0], kernels[0]).padding(paddings[0])),
	conv_1(torch::nn::Conv2dOptions(channels[0], channels[1], kernels[1]).padding(paddings[1])),
	conv_2(torch::nn::Conv2dOptions(channels[1], channels[2], kernels[2]).padding(paddings[2])),
	conv_trans_3(torch::nn::ConvTranspose2dOptions(channels[2], 1, kernels[3]).stride(2)
	.padding(paddings[3])) {

	register_module("conv_0", conv_0);
	register_module("conv_1", conv_1);
	register_module("conv_2", conv_2);
	register_module("conv_trans_3", conv_trans_3);
}

torch::Tensor FSRCNNImpl::forward(torch::Tensor x) {
	assert(x.size(2) == 64 && x.size(3) == 64);

	func::LeakyReLUFuncOptions act_opt;
	act_opt.negative_slope(0.15);

	x = conv_0(x); //Input
	x = func::leaky_relu(conv_1(x), act_opt);
	x = func::leaky_relu(conv_2(x), act_opt);
	x = func::leaky_relu(conv_trans_3(x), act_opt); //Output

	return x;
}
