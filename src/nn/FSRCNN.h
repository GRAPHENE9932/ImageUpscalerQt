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

#pragma once

#include <sstream>
#include <vector>

#include <torch/torch.h>

using namespace torch::nn;

struct FSRCNNImpl : torch::nn::Module {
	Conv2d conv_0, conv_1, conv_3;
	ConvTranspose2d conv_trans_4;
	std::vector<Conv2d> conv_2;

public:
	FSRCNNImpl(std::vector<unsigned short> kernels, std::vector<unsigned short> paddings,
			   std::vector<unsigned short> channels);

	torch::Tensor forward(torch::Tensor x);
};

TORCH_MODULE(FSRCNN);
