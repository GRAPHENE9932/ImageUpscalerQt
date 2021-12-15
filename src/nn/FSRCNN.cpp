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

#include <cassert>

#include "FSRCNN.h"

FSRCNN FSRCNN::create(unsigned short img_w, unsigned short img_h,
					  std::vector<unsigned short> ker, std::vector<unsigned short> chn) {
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
		pads_l[i] = {0, 0, cur_pad, cur_pad};
		pads_r[i] = {0, 0, cur_pad, cur_pad};
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
	std::vector<dnnl::memory::desc> src_descs_a;
	std::vector<dnnl::memory::desc> ker_descs_a;
	std::vector<dnnl::memory::desc> bias_descs_a;
	std::vector<dnnl::memory::desc> dest_descs_a;
	std::vector<dnnl::memory::dims> pads_l_a;
	std::vector<dnnl::memory::dims> pads_r_a;
	std::vector<dnnl::convolution_forward> convs_a;
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
												 dnnl::memory::format_tag::nchw);
			dest_descs_a[i] = dnnl::memory::desc(dest_dims[i],
												 dnnl::memory::data_type::f32,
												 dnnl::memory::format_tag::nchw);
			pads_l_a = pads_l;
			pads_r_a = pads_r;

			//Initialize convolutions
			dnnl::post_ops post_ops;
			post_ops.append_eltwise(1.0F, dnnl::algorithm::eltwise_relu, 0.01F, 0.0F);
			dnnl::primitive_attr attr;
			attr.set_post_ops(post_ops);

			if (i == nn_size - 1) { //If it is the last layer (deconvolutional).
				auto deconv_desc = dnnl::deconvolution_forward::desc(dnnl::prop_kind::forward_inference,
								   dnnl::algorithm::deconvolution_direct,
								   src_descs_a[i], ker_descs_a[i], bias_descs_a[i],
								   dest_descs_a[i], {1, 1}, pads_l_a[i], pads_r_a[i]);
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

	return FSRCNN(eng_a, src_descs_a, ker_descs_a, bias_descs_a,
				  bias_descs_a, pads_l_a, pads_r_a, convs_a, deconv_a);
}

void FSRCNN::execute(dnnl::memory src_mem, std::vector<dnnl::memory> ker_mem,
					 std::vector<dnnl::memory> bias_mem, dnnl::memory dest_mem) {
	assert(ker_mem.size() == convs.size() + 1 &&
		   ker_mem.size() == bias_mem.size());
	const auto nn_size = convs.size() + 1;

	dnnl::memory cur_src_mem = src_mem;
	dnnl::memory cur_dest_mem;
	for (char i = 0; i < nn_size; i++) {
		if (i == nn_size - 1)
			cur_dest_mem = dest_mem;
		else
			cur_dest_mem = dnnl::memory(dest_descs[i], eng);

		if (i == nn_size - 1) { //If it is the last layer (deconvolutional).
			deconv.execute(eng_str, {
				{DNNL_ARG_SRC, cur_src_mem},
				{DNNL_ARG_WEIGHTS, ker_mem[i]},
				{DNNL_ARG_BIAS, bias_mem[i]},
				{DNNL_ARG_DST, cur_dest_mem}
			});
		}
		else {
			convs[i].execute(eng_str, {
				{DNNL_ARG_SRC, cur_src_mem},
				{DNNL_ARG_WEIGHTS, ker_mem[i]},
				{DNNL_ARG_BIAS, bias_mem[i]},
				{DNNL_ARG_DST, cur_dest_mem}
			});
		}

		cur_src_mem = cur_dest_mem;
	}
}

/*namespace func = torch::nn::functional;

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
}*/
