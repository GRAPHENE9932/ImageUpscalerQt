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

#include <vector>

#include <dnnl.hpp>

class FSRCNN {
private:
	dnnl::engine eng;
	dnnl::stream eng_str;

	std::vector<dnnl::memory::desc> src_descs; //Source memory description.
	std::vector<dnnl::memory::desc> ker_descs; //Kernels (weights) memory description.
	std::vector<dnnl::memory::desc> bias_descs; //Biases memory description.
	std::vector<dnnl::memory::desc> dest_descs; //Destination memory description.

	std::vector<dnnl::memory::dims> pads_l;
	std::vector<dnnl::memory::dims> pads_r;

	//Convolution layer primitives descriptions.
	std::vector<dnnl::convolution_forward> convs;
	dnnl::deconvolution_forward deconv;

public:
	static FSRCNN create(unsigned short img_w, unsigned short img_h,
						 std::vector<unsigned short> ker, std::vector<unsigned short> chn);

	static FSRCNN create(std::vector<dnnl::memory::dims> src_dims, std::vector<dnnl::memory::dims> ker_dims,
						 std::vector<dnnl::memory::dims> bias_dims, std::vector<dnnl::memory::dims> dest_dims,
						 std::vector<dnnl::memory::dims> pads_l, std::vector<dnnl::memory::dims> pads_r);

	FSRCNN(dnnl::engine eng, std::vector<dnnl::memory::desc> src_descs,
		  std::vector<dnnl::memory::desc> ker_descs, std::vector<dnnl::memory::desc> bias_descs,
		  std::vector<dnnl::memory::desc> dest_descs, std::vector<dnnl::memory::dims> pads_l,
		  std::vector<dnnl::memory::dims> pads_r, std::vector<dnnl::convolution_forward> convs,
		  dnnl::deconvolution_forward deconv) :

		  eng(eng), eng_str(eng), src_descs(src_descs), ker_descs(ker_descs),
		  bias_descs(bias_descs), dest_descs(dest_descs), pads_l(pads_l),
		  pads_r(pads_r), convs(convs), deconv(deconv) {}

	void execute(dnnl::memory src_mem, std::vector<dnnl::memory> ker_mem,
				 std::vector<dnnl::memory> bias_mem, dnnl::memory dest_mem);
};
