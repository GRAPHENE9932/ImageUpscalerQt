/*
 * ImageUpscalerQt - Fast Super Resolution Convolutional Neural Network header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
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

	unsigned char size_multiplier;

	void init_src_descs(const std::vector<unsigned short>& chn,
						const unsigned short img_w, const unsigned short img_h);
	void init_ker_descs(const std::vector<unsigned short>& ker,
						const std::vector<unsigned short>& chn);
	void init_bias_descs(const std::vector<unsigned short>& chn);
	void init_dest_descs(const std::vector<unsigned short>& chn,
						 const unsigned short img_w, const unsigned short img_h);
	void init_pads(const std::vector<unsigned short>& ker);
	void init_conv();

public:
	FSRCNN(unsigned short img_w, unsigned short img_h, const FSRCNNDesc& desc);

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
