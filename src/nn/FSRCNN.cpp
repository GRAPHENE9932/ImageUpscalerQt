/*
 * ImageUpscalerQt - Fast Super Resolution Convolutional Neural Network
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cassert>

#include "FSRCNN.hpp"

FSRCNN FSRCNN::create(unsigned short img_w, unsigned short img_h, FSRCNNDesc desc) {
	// Shortcuts
	auto ker = desc.kernels;
	auto chn = desc.channels;

	assert(ker.size() + 1 == chn.size());

	const auto nn_size = ker.size();
	std::vector<dnnl::memory::dims> src_dims(nn_size);
	std::vector<dnnl::memory::dims> ker_dims(nn_size);
	std::vector<dnnl::memory::dims> bias_dims(nn_size);
	std::vector<dnnl::memory::dims> dest_dims(nn_size);
	std::vector<dnnl::memory::dims> pads_l(nn_size);
	std::vector<dnnl::memory::dims> pads_r(nn_size);

	for (char i = 0; i < nn_size; i++) {
		src_dims[i] = {1, chn[i], img_h, img_w};
		ker_dims[i] = {chn[i + 1], chn[i], ker[i], ker[i]};
		bias_dims[i] = {chn[i + 1]};
		dest_dims[i] = {1, chn[i + 1],
			img_h * (i == nn_size - 1 ? 3 : 1), img_w * (i == nn_size - 1 ? 3 : 1)};

		const auto cur_pad = i == nn_size ? (ker[i] - 3) / 2 : (ker[i] - 1) / 2;
		pads_l[i] = {cur_pad, cur_pad, 0, 0};
		pads_r[i] = {cur_pad, cur_pad, 0, 0};
	}

	return create(src_dims, ker_dims, bias_dims, dest_dims, pads_l, pads_r);
}

FSRCNN FSRCNN::create(std::vector<dnnl::memory::dims> src_dims,
					  std::vector<dnnl::memory::dims> ker_dims,
					  std::vector<dnnl::memory::dims> bias_dims,
					  std::vector<dnnl::memory::dims> dest_dims,
					  std::vector<dnnl::memory::dims> pads_l,
					  std::vector<dnnl::memory::dims> pads_r) {
	assert(src_dims.size() == ker_dims.size() &&
		   src_dims.size() == bias_dims.size() &&
		   src_dims.size() == dest_dims.size() &&
		   src_dims.size() == pads_l.size() &&
		   src_dims.size() == pads_r.size());
	const auto nn_size = src_dims.size();

	//Initialize needed variables
	dnnl::engine eng_a;
	dnnl::stream eng_str_a;
	std::vector<dnnl::memory::desc> src_descs_a(nn_size);
	std::vector<dnnl::memory::desc> ker_descs_a(nn_size);
	std::vector<dnnl::memory::desc> bias_descs_a(nn_size);
	std::vector<dnnl::memory::desc> dest_descs_a(nn_size);
	std::vector<dnnl::memory::dims> pads_l_a(nn_size);
	std::vector<dnnl::memory::dims> pads_r_a(nn_size);
	std::vector<dnnl::convolution_forward> convs_a(nn_size - 1);
	dnnl::deconvolution_forward deconv_a;

	eng_a = dnnl::engine(dnnl::engine::kind::cpu, 0);
	eng_str_a = dnnl::stream(eng_a);

	for (char i = 0; i < nn_size; i++) {
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

			// Initialize convolutions.
			dnnl::post_ops post_ops;
			post_ops.append_eltwise(1.0F, dnnl::algorithm::eltwise_relu, 0.01F, 0.0F);
			dnnl::primitive_attr attr;
			attr.set_post_ops(post_ops);

			if (i == nn_size - 1) { // If it is the last layer (deconvolutional).
				auto deconv_desc = dnnl::deconvolution_forward::desc(dnnl::prop_kind::forward_inference,
								   dnnl::algorithm::deconvolution_direct,
								   src_descs_a[i], ker_descs_a[i], bias_descs_a[i],
								   dest_descs_a[i], {3, 3}, pads_l_a[i], pads_r_a[i]);

				auto deconv_prim_desc = dnnl::deconvolution_forward::primitive_desc(deconv_desc, attr, eng_a);

				deconv_a = dnnl::deconvolution_forward(deconv_prim_desc);
			}
			else {
				auto conv_desc = dnnl::convolution_forward::desc(dnnl::prop_kind::forward_inference,
								 dnnl::algorithm::convolution_auto,
								 src_descs_a[i], ker_descs_a[i], bias_descs_a[i],
								 dest_descs_a[i], {1, 1}, pads_l_a[i], pads_r_a[i]);

				auto conv_prim_desc = dnnl::convolution_forward::primitive_desc(conv_desc, attr, eng_a);

				convs_a[i] = dnnl::convolution_forward(conv_prim_desc);
			}
	}

	return FSRCNN(eng_a, eng_str_a, src_descs_a, ker_descs_a, bias_descs_a,
				  dest_descs_a, pads_l_a, pads_r_a, convs_a, deconv_a);
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
