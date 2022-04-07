/*
 * ImageUpscalerQt - Fast Super Resolution Convolutional Neural Network
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cassert>

#include "FSRCNN.hpp"

FSRCNN::FSRCNN(unsigned short img_w, unsigned short img_h, const FSRCNNDesc& desc) {
	this->size_multiplier = desc.size_multiplier;

	init_src_descs(desc.channels, img_w, img_h);
	init_ker_descs(desc.kernels, desc.channels);
	init_bias_descs(desc.channels);
	init_dest_descs(desc.channels, img_w, img_h);
	init_pads(desc.kernels);

	eng = dnnl::engine(dnnl::engine::kind::cpu, 0);
	eng_str = dnnl::stream(eng);

	init_conv();
}

void inline FSRCNN::init_src_descs(const std::vector<unsigned short>& chn,
								   const unsigned short img_w, const unsigned short img_h) {
	const size_t& nn_size = chn.size() - 1;

	src_descs.resize(nn_size);

	for (int i = 0; i < nn_size; i++) {
		dnnl::memory::dims cur_dims = {1, chn[i], img_h, img_w};
		src_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										  dnnl::memory::format_tag::nchw);
	}
}

void inline FSRCNN::init_ker_descs(const std::vector<unsigned short>& ker,
								   const std::vector<unsigned short>& chn) {
	const size_t& nn_size = ker.size();

	ker_descs.resize(nn_size);

	for (int i = 0; i < nn_size; i++) {
		dnnl::memory::dims cur_dims = {chn[i + 1], chn[i], ker[i], ker[i]};
		ker_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										  dnnl::memory::format_tag::oihw);
	}
}

void inline FSRCNN::init_bias_descs(const std::vector<unsigned short>& chn) {
	const size_t& nn_size = chn.size() - 1;

	bias_descs.resize(nn_size);

	for (int i = 0; i < nn_size; i++) {
		dnnl::memory::dims cur_dims = {chn[i + 1]};
		bias_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										   dnnl::memory::format_tag::x);
	}
}

void inline FSRCNN::init_dest_descs(const std::vector<unsigned short>& chn,
									const unsigned short img_w, const unsigned short img_h) {
	const size_t& nn_size = chn.size() - 1;
	const unsigned char& mul = size_multiplier;

	dest_descs.resize(nn_size);

	for (int i = 0; i < nn_size; i++) {
		const auto cur_img_h = img_h * (i == nn_size - 1 ? mul : 1);
		const auto cur_img_w = img_w * (i == nn_size - 1 ? mul : 1);
		dnnl::memory::dims cur_dims = {1, chn[i + 1], cur_img_h, cur_img_w};
		dest_descs[i] = dnnl::memory::desc(cur_dims, dnnl::memory::data_type::f32,
										   dnnl::memory::format_tag::nchw);
	}
}

void inline FSRCNN::init_pads(const std::vector<unsigned short>& ker) {
	const size_t& nn_size = ker.size();
	const unsigned char& mul = size_multiplier;

	pads_l.resize(nn_size);
	pads_r.resize(nn_size);

	for (int i = 0; i < nn_size; i++) {
		const auto cur_pad = i == nn_size ? (ker[i] - mul) / 2 : (ker[i] - 1) / 2;
		pads_l[i] = {cur_pad, cur_pad, 0, 0};
		pads_r[i] = {cur_pad, cur_pad, 0, 0};
	}
}

void inline FSRCNN::init_conv() {
	const size_t& nn_size = ker_descs.size();
	const unsigned char& mul = size_multiplier;

	convs.resize(nn_size - 1);

	dnnl::post_ops post_ops;
	post_ops.append_eltwise(1.0f, dnnl::algorithm::eltwise_relu, 0.01f, 0.0f);
	dnnl::primitive_attr attr;
	attr.set_post_ops(post_ops);

	for (int i = 0; i < nn_size - 1; i++) {
		// Initialize convolutions.
		auto conv_desc = dnnl::convolution_forward::desc(dnnl::prop_kind::forward_inference,
							dnnl::algorithm::convolution_auto,
							src_descs[i], ker_descs[i], bias_descs[i],
							dest_descs[i], {1, 1}, pads_l[i], pads_r[i]);

		auto conv_prim_desc = dnnl::convolution_forward::primitive_desc(conv_desc, attr, eng);
		convs[i] = dnnl::convolution_forward(conv_prim_desc);
	}

	// Initialize deconvolution.
	auto deconv_desc = dnnl::deconvolution_forward::desc(dnnl::prop_kind::forward_inference,
					   dnnl::algorithm::deconvolution_direct,
					   src_descs[nn_size - 1], ker_descs[nn_size - 1], bias_descs[nn_size - 1],
					   dest_descs[nn_size - 1], {mul, mul}, pads_l[nn_size - 1], pads_r[nn_size - 1]);

	auto deconv_prim_desc = dnnl::deconvolution_forward::primitive_desc(deconv_desc, attr, eng);
	deconv = dnnl::deconvolution_forward(deconv_prim_desc);
}

void FSRCNN::execute(dnnl::memory src_mem, std::vector<dnnl::memory> ker_mem,
					 std::vector<dnnl::memory> bias_mem, dnnl::memory dest_mem) {
	assert(ker_mem.size() == convs.size() + 1 &&
		   ker_mem.size() == bias_mem.size());

	dnnl::memory cur_dest;

	for (char i = 0; i < ker_mem.size(); i++) {
		dnnl::memory cur_src;

		if (i == 0)
			cur_src = src_mem;
		else
			cur_src = cur_dest;

		if (i == ker_mem.size() - 1) {
			cur_dest = dest_mem;

			deconv.execute(eng_str, {
				{DNNL_ARG_SRC, cur_src},
				{DNNL_ARG_WEIGHTS, ker_mem[i]},
				{DNNL_ARG_BIAS, bias_mem[i]},
				{DNNL_ARG_DST, cur_dest}
			});
		}
		else {
			cur_dest = dnnl::memory(dest_descs[i], eng);

			convs[i].execute(eng_str, {
				{DNNL_ARG_SRC, cur_src},
				{DNNL_ARG_WEIGHTS, ker_mem[i]},
				{DNNL_ARG_BIAS, bias_mem[i]},
				{DNNL_ARG_DST, cur_dest}
			});
		}
	};
}
