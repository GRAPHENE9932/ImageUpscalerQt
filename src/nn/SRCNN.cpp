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

SRCNN SRCNN::create(unsigned short img_w, unsigned short img_h,
					std::array<unsigned short, 3> ker, std::array<unsigned short, 4> chn) {
		std::array<dnnl::memory::dims, 3> src_dims;
		std::array<dnnl::memory::dims, 3> ker_dims;
		std::array<dnnl::memory::dims, 3> bias_dims;
		std::array<dnnl::memory::dims, 3> dest_dims;
		std::array<dnnl::memory::dims, 3> pads_l;
		std::array<dnnl::memory::dims, 3> pads_r;

		for (char i = 0; i < 3; i++) {
			src_dims[i] = {1, chn[i], img_h, img_w};
			ker_dims[i] = {chn[i + 1], chn[i], ker[i], ker[i]};
			bias_dims[i] = {chn[i + 1]};
			dest_dims[i] = {1, chn[i + 1], img_h, img_h};
			pads_l[i] = {0, 0, (ker[i] - 1) / 2, (ker[i] - 1) / 2};
			pads_r[i] = {0, 0, (ker[i] - 1) / 2, (ker[i] - 1) / 2};
		}

		return create(src_dims, ker_dims, bias_dims, dest_dims, pads_l, pads_r);
	}

SRCNN SRCNN::create(std::array<dnnl::memory::dims, 3> src_dims,
					std::array<dnnl::memory::dims, 3> ker_dims,
					std::array<dnnl::memory::dims, 3> bias_dims,
					std::array<dnnl::memory::dims, 3> dest_dims,
					std::array<dnnl::memory::dims, 3> pads_l,
					std::array<dnnl::memory::dims, 3> pads_r) {
	//Initialize needed variables
	dnnl::engine eng_a;
	dnnl::stream eng_str_a;
	std::array<dnnl::memory::desc, 3> src_descs_a;
	std::array<dnnl::memory::desc, 3> ker_descs_a;
	std::array<dnnl::memory::desc, 3> bias_descs_a;
	std::array<dnnl::memory::desc, 3> dest_descs_a;
	std::array<dnnl::memory::dims, 3> pads_l_a;
	std::array<dnnl::memory::dims, 3> pads_r_a;
	std::array<dnnl::convolution_forward, 3> convs_a;

	eng_a = dnnl::engine(dnnl::engine::kind::cpu, 0);
	eng_str_a = dnnl::stream(eng_a);

	for (char i = 0; i < 3; i++) {
			src_descs_a[i] = dnnl::memory::desc(src_dims[i],
												dnnl::memory::data_type::f32,
												dnnl::memory::format_tag::nchw);
			ker_descs_a[i] = dnnl::memory::desc(ker_dims[i],
												dnnl::memory::data_type::f32,
												dnnl::memory::format_tag::oihw);
			bias_descs_a[i] = dnnl::memory::desc(bias_dims[i],
												 dnnl::memory::data_type::f32,
												 dnnl::memory::format_tag::nchw);
			dest_descs_a[i] = dnnl::memory::desc(dest_dims[i],
												 dnnl::memory::data_type::f32,
												 dnnl::memory::format_tag::nchw);
			pads_l_a = pads_l;
			pads_r_a = pads_r;

			//Initialize convolutions
			dnnl::post_ops post_ops;
			post_ops.append_eltwise(1.0F, dnnl::algorithm::eltwise_relu, 0.15F, 0.0F);
			dnnl::primitive_attr attr;
			attr.set_post_ops(post_ops);

			auto conv_desc = dnnl::convolution_forward::desc(dnnl::prop_kind::forward_inference,
							 dnnl::algorithm::convolution_auto,
							 src_descs_a[i], ker_descs_a[i], bias_descs_a[i],
							 dest_descs_a[i], {1, 1}, pads_l_a[i], pads_r_a[i]);
			auto conv_prim_desc = dnnl::convolution_forward::primitive_desc(conv_desc, attr, eng_a);
			convs_a[i] = dnnl::convolution_forward(conv_prim_desc);
	}

	return SRCNN(eng_a, src_descs_a, ker_descs_a, bias_descs_a,
				 bias_descs_a, pads_l_a, pads_r_a, convs_a);
}

void SRCNN::execute(dnnl::memory src_mem, std::array<dnnl::memory, 3> ker_mem,
					std::array<dnnl::memory, 3> bias_mem, dnnl::memory dest_mem) {
	dnnl::memory cur_src_mem = src_mem;
	dnnl::memory cur_dest_mem;
	for (char i = 0; i < 3; i++) {
		if (i == 2)
			cur_dest_mem = dest_mem;
		else
			cur_dest_mem = dnnl::memory(dest_descs[i], eng);

		convs[i].execute(eng_str, {
			{DNNL_ARG_SRC, cur_src_mem},
			{DNNL_ARG_WEIGHTS, ker_mem[i]},
			{DNNL_ARG_BIAS, bias_mem[i]},
			{DNNL_ARG_DST, cur_dest_mem}
		});

		cur_src_mem = cur_dest_mem;
	}
}
