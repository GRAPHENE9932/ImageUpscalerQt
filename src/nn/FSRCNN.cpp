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

FSRCNNImpl::FSRCNNImpl(std::vector<unsigned short> ker,
					   std::vector<unsigned short> pad,
					   std::vector<unsigned short> channels) :

	conv_0(torch::nn::Conv2dOptions(1, channels[0], ker[0]).padding(pad[0])),

	conv_1(torch::nn::Conv2dOptions(channels[0], channels[1], ker[1]).padding(pad[1])),

	conv_3(torch::nn::Conv2dOptions(channels[ker.size() - 3],
									channels[ker.size() - 2],
									ker[ker.size() - 2]).padding(pad[ker.size() - 2])),

	conv_trans_4(torch::nn::ConvTranspose2dOptions(channels[ker.size() - 2],
												   1, ker[ker.size() - 1]).stride(3)
												   .padding(pad[ker.size() - 1])),
	conv_2(ker.size() - 4, nullptr) {

	register_module("conv_0", conv_0);
	register_module("conv_1", conv_1);
	for (size_t i = 0; i < conv_2.size(); i++) {
		conv_2[i] = Conv2d(torch::nn::Conv2dOptions(
			channels[i + 1], channels[i + 2], ker[i + 2]
		).padding(pad[i + 2]));
		register_module("conv_2_" + std::to_string(i), conv_2[i]);
	}
	register_module("conv_3", conv_3);
	register_module("conv_trans_4", conv_trans_4);
}

torch::Tensor FSRCNNImpl::forward(torch::Tensor x) {

	func::LeakyReLUFuncOptions act_opt;
	act_opt.negative_slope(0.01);

	x = func::leaky_relu(conv_0(x), act_opt); //Input
	x = func::leaky_relu(conv_1(x), act_opt);
	for (size_t i = 0; i < conv_2.size(); i++)
		x = func::leaky_relu(conv_2[i](x), act_opt);
	x = func::leaky_relu(conv_3(x), act_opt);
	x = func::leaky_relu(conv_trans_4(x), act_opt); //Output

	return x;
}
