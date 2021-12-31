/*
 * ImageUpscalerQt - Super Resolution Convolutional Neural Network
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <iostream>

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
			dest_dims[i] = {1, chn[i + 1], img_h, img_w};
			pads_l[i] = {(ker[i] - 1) / 2, (ker[i] - 1) / 2, 0, 0};
			pads_r[i] = {(ker[i] - 1) / 2, (ker[i] - 1) / 2, 0, 0};
		}

		return create(src_dims, ker_dims, bias_dims, dest_dims, pads_l, pads_r);
	}

SRCNN SRCNN::create(std::array<dnnl::memory::dims, 3> src_dims,
					std::array<dnnl::memory::dims, 3> ker_dims,
					std::array<dnnl::memory::dims, 3> bias_dims,
					std::array<dnnl::memory::dims, 3> dest_dims,
					std::array<dnnl::memory::dims, 3> pads_l,
					std::array<dnnl::memory::dims, 3> pads_r) {
	// Initialize needed variables.
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
												 dnnl::memory::format_tag::x);
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

	return SRCNN(eng_a, eng_str_a, src_descs_a, ker_descs_a, bias_descs_a,
				 dest_descs_a, pads_l_a, pads_r_a, convs_a);
}

void SRCNN::execute(dnnl::memory src_mem, std::array<dnnl::memory, 3> ker_mem,
					std::array<dnnl::memory, 3> bias_mem, dnnl::memory dest_mem) {
	dnnl::memory cur_dest;

	for (char i = 0; i < 3; i++) {
		dnnl::memory cur_src;

		if (i == 0)
			cur_src = src_mem;
		else
			cur_src = cur_dest;

		if (i == 3 - 1)
			cur_dest = dest_mem;
		else
			cur_dest = dnnl::memory(dest_descs[i], eng);

		convs[i].execute(eng_str, {
			{DNNL_ARG_SRC, cur_src},
			{DNNL_ARG_WEIGHTS, ker_mem[i]},
			{DNNL_ARG_BIAS, bias_mem[i]},
			{DNNL_ARG_DST, cur_dest}
		});
	};
}
