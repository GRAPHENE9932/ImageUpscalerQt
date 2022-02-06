/*
 * ImageUpscalerQt - Fast Super Resolution Convolutional Neural Network header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <vector>

#include <dnnl.hpp>

#include "../tasks/TaskDesc.hpp"

class FSRCNN {
private:
	dnnl::engine eng;
	dnnl::stream eng_str;

	std::vector<dnnl::memory::desc> src_descs; // Source memory description.
	std::vector<dnnl::memory::desc> ker_descs; // Kernels (weights) memory description.
	std::vector<dnnl::memory::desc> bias_descs; // Biases memory description.
	std::vector<dnnl::memory::desc> dest_descs; // Destination memory description.

	std::vector<dnnl::memory::dims> pads_l;
	std::vector<dnnl::memory::dims> pads_r;

	// Convolution layer primitives descriptions.
	std::vector<dnnl::convolution_forward> convs;
	dnnl::deconvolution_forward deconv;

public:
	static FSRCNN create(unsigned short img_w, unsigned short img_h, FSRCNNDesc desc);

	static FSRCNN create(std::vector<dnnl::memory::dims> src_dims, std::vector<dnnl::memory::dims> ker_dims,
						 std::vector<dnnl::memory::dims> bias_dims, std::vector<dnnl::memory::dims> dest_dims,
						 std::vector<dnnl::memory::dims> pads_l, std::vector<dnnl::memory::dims> pads_r);

	FSRCNN(dnnl::engine eng, dnnl::stream eng_str, const std::vector<dnnl::memory::desc>& src_descs,
		  const std::vector<dnnl::memory::desc>& ker_descs, const std::vector<dnnl::memory::desc>& bias_descs,
		  const std::vector<dnnl::memory::desc>& dest_descs, const std::vector<dnnl::memory::dims>& pads_l,
		  const std::vector<dnnl::memory::dims>& pads_r, const std::vector<dnnl::convolution_forward>& convs,
		  dnnl::deconvolution_forward deconv) :

		  eng(eng), eng_str(eng_str), src_descs(src_descs), ker_descs(ker_descs),
		  bias_descs(bias_descs), dest_descs(dest_descs), pads_l(pads_l),
		  pads_r(pads_r), convs(convs), deconv(deconv) {}

	std::vector<dnnl::memory::desc> get_ker_descs() const {
		return ker_descs;
	}

	std::vector<dnnl::memory::desc> get_bias_descs() const {
		return bias_descs;
	}

	dnnl::memory::desc get_input_desc() const {
		return src_descs[0];
	}

	dnnl::memory::desc get_output_desc() const {
		return dest_descs[dest_descs.size() - 1];
	}

	dnnl::engine get_engine() const {
		return eng;
	}

	void execute(dnnl::memory src_mem, std::vector<dnnl::memory> ker_mem,
				 std::vector<dnnl::memory> bias_mem, dnnl::memory dest_mem);
};
