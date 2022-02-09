/*
 * ImageUpscalerQt - Super Resolution Convolutional Neural Network
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <iostream>

#include "SRCNN.hpp"

SRCNN::SRCNN(const unsigned short img_w, const unsigned short img_h, const SRCNNDesc& desc) {
	init_src_descs(desc.channels, img_w, img_h);
	init_ker_descs(desc.channels, desc.kernels);
	init_bias_descs(desc.channels);
	init_dest_descs(desc.channels, img_w, img_h);
	init_pads(desc.kernels);

	eng = dnnl::engine(dnnl::engine::kind::cpu, 0);
	eng_str = dnnl::stream(eng);

	init_conv();
}

void inline SRCNN::init_src_descs(const std::array<unsigned short, 4>& chn,
								  const unsigned short img_w, const unsigned short img_h) {
	for (int i = 0; i < 3; i++) {
		dnnl::memory::dims cur_dims = {1, chn[i], img_h, img_w};
		src_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										  dnnl::memory::format_tag::nchw);
	}
}

void inline SRCNN::init_ker_descs(const std::array<unsigned short, 4>& chn,
								  const std::array<unsigned short, 3>& ker) {
	for (int i = 0; i < 3; i++) {
		dnnl::memory::dims cur_dims = {chn[i + 1], chn[i], ker[i], ker[i]};
		ker_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										  dnnl::memory::format_tag::oihw);
	}
}

void inline SRCNN::init_bias_descs(const std::array<unsigned short, 4>& chn) {
	for (int i = 0; i < 3; i++) {
		dnnl::memory::dims cur_dims = {chn[i + 1]};
		bias_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										   dnnl::memory::format_tag::x);
	}
}

void inline SRCNN::init_dest_descs(const std::array<unsigned short, 4>& chn,
								   const unsigned short img_w, const unsigned short img_h) {
	for (int i = 0; i < 3; i++) {
		dnnl::memory::dims cur_dims = {1, chn[i + 1], img_h, img_w};
		dest_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										   dnnl::memory::format_tag::nchw);
	}
}

void inline SRCNN::init_pads(const std::array<unsigned short, 3>& ker) {
	for (int i = 0; i < 3; i++) {
		pads_l[i] = {(ker[i] - 1) / 2, (ker[i] - 1) / 2, 0, 0};
		pads_r[i] = {(ker[i] - 1) / 2, (ker[i] - 1) / 2, 0, 0};
	}
}

void inline SRCNN::init_conv() {
	dnnl::post_ops post_ops;
	post_ops.append_eltwise(1.0f, dnnl::algorithm::eltwise_relu, 0.15f, 0.0f);
	dnnl::primitive_attr attr;
	attr.set_post_ops(post_ops);
	for (int i = 0; i < 3; i++) {
		auto conv_desc = dnnl::convolution_forward::desc(dnnl::prop_kind::forward_inference,
						 dnnl::algorithm::convolution_auto,
						 src_descs[i], ker_descs[i], bias_descs[i],
						 dest_descs[i], {1, 1}, pads_l[i], pads_r[i]);

		auto conv_prim_desc = dnnl::convolution_forward::primitive_desc(conv_desc, attr, eng);

		convs[i] = dnnl::convolution_forward(conv_prim_desc);
	}
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
