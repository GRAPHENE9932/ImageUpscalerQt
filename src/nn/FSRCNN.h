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
#include <array>

#include <torch/torch.h>

using namespace torch::nn;

struct FSRCNNImpl : torch::nn::Module {
	Conv2d conv_0, conv_1, conv_2;
	ConvTranspose2d conv_trans_3;

public:
	FSRCNNImpl(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
			   std::array<unsigned short, 5> channels);

	torch::Tensor forward(torch::Tensor x);
};

TORCH_MODULE(FSRCNN);
