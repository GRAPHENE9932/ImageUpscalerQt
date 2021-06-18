/*
 * ImageUpscalerQt SRC Neural network
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

#include "SRCNN.h"

SRCNNImpl::SRCNNImpl(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
		  std::array<unsigned short, 2> channels) :
	conv_0(torch::nn::Conv2dOptions(1, channels[0], kernels[0]).padding(paddings[0])),
	conv_1(torch::nn::Conv2dOptions(channels[0], channels[1], kernels[1]).padding(paddings[1])),
	conv_2(torch::nn::Conv2dOptions(channels[1], 1, kernels[2]).padding(paddings[2])) {

	register_module("conv_0", conv_0);
	register_module("conv_1", conv_1);
	register_module("conv_2", conv_2);
}

torch::Tensor SRCNNImpl::forward(torch::Tensor x) {
	assert(x.size(1) == 1 && x.size(2) == 192 && x.size(3) == 192);

	func::LeakyReLUFuncOptions act_opt;
	act_opt.negative_slope(0.15);

	x = conv_0(x), act_opt; //Input
	x = func::leaky_relu(conv_1(x), act_opt);
	x = conv_2(x); //Output

	return x;
}
