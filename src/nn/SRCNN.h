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

#pragma once

#include <array>

#include <dnnl.hpp>

class SRCNN {
private:
	dnnl::engine eng;
	dnnl::stream eng_str;

	std::array<dnnl::memory::desc, 3> src_descs; //Source memory description.
	std::array<dnnl::memory::desc, 3> ker_descs; //Kernels (weights) memory description.
	std::array<dnnl::memory::desc, 3> bias_descs; //Biases memory description.
	std::array<dnnl::memory::desc, 3> dest_descs; //Destination memory description.

	std::array<dnnl::memory::dims, 3> pads_l;
	std::array<dnnl::memory::dims, 3> pads_r;

	//Convolution layer primitives descriptions.
	std::array<dnnl::convolution_forward, 3> convs;

public:
	static SRCNN create(unsigned short img_w, unsigned short img_h,
						std::array<unsigned short, 3> ker, std::array<unsigned short, 4> chn);

	static SRCNN create(std::array<dnnl::memory::dims, 3> src_dims,
						std::array<dnnl::memory::dims, 3> ker_dims,
						std::array<dnnl::memory::dims, 3> bias_dims,
						std::array<dnnl::memory::dims, 3> dest_dims,
						std::array<dnnl::memory::dims, 3> pads_l,
						std::array<dnnl::memory::dims, 3> pads_r);

	SRCNN(dnnl::engine eng, std::array<dnnl::memory::desc, 3> src_descs,
		  std::array<dnnl::memory::desc, 3> ker_descs, std::array<dnnl::memory::desc, 3> bias_descs,
		  std::array<dnnl::memory::desc, 3> dest_descs, std::array<dnnl::memory::dims, 3> pads_l,
		  std::array<dnnl::memory::dims, 3> pads_r,
		  std::array<dnnl::convolution_forward, 3> convs) :

		  eng(eng), eng_str(eng), src_descs(src_descs), ker_descs(ker_descs),
		  bias_descs(bias_descs), dest_descs(dest_descs), pads_l(pads_l),
		  pads_r(pads_r), convs(convs) {}

	std::array<dnnl::memory::desc, 3> get_ker_descs() {
		return ker_descs;
	}

	std::array<dnnl::memory::desc, 3> get_bias_descs() {
		return bias_descs;
	}

	dnnl::memory::desc get_input_desc() {
		return src_descs[0];
	}

	dnnl::memory::desc get_output_desc() {
		return dest_descs[2];
	}

	dnnl::engine get_engine() {
		return eng;
	}

	void execute(dnnl::memory src_mem, std::array<dnnl::memory, 3> ker_mem,
				 std::array<dnnl::memory, 3> bias_mem, dnnl::memory dest_mem);
};
