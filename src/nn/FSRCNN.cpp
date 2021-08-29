/*
 * ImageUpscalerQt FSRC Neural network
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

#include "FSRCNN.h"

namespace func = torch::nn::functional;

FSRCNNImpl::FSRCNNImpl(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
	std::array<unsigned short, 5> channels) :
	conv_0(torch::nn::Conv2dOptions(channels[0], channels[1], kernels[0]).padding(paddings[0])),
	conv_1(torch::nn::Conv2dOptions(channels[1], channels[2], kernels[1]).padding(paddings[1])),
	conv_2(torch::nn::Conv2dOptions(channels[2], channels[3], kernels[2]).padding(paddings[2])),
	conv_trans_3(torch::nn::ConvTranspose2dOptions(channels[3], channels[4], kernels[3]).stride(2)
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

	x = func::leaky_relu(conv_0(x), act_opt); //Input
	x = func::leaky_relu(conv_1(x), act_opt);
	x = func::leaky_relu(conv_2(x), act_opt);
	x = conv_trans_3(x); //Output

	return x;
}
