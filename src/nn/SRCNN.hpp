/*
 * ImageUpscalerQt - Super Resolution Convolutional Neural Network header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include <dnnl.hpp>

#include "../tasks/TaskDesc.hpp"

class SRCNN {
private:
	dnnl::engine eng;
	dnnl::stream eng_str;

	std::array<dnnl::memory::desc, 3> src_descs; // Source memory description.
	std::array<dnnl::memory::desc, 3> ker_descs; // Kernels (weights) memory description.
	std::array<dnnl::memory::desc, 3> bias_descs; // Biases memory description.
	std::array<dnnl::memory::desc, 3> dest_descs; // Destination memory description.

	std::array<dnnl::memory::dims, 3> pads_l;
	std::array<dnnl::memory::dims, 3> pads_r;

	// Convolution layer primitives descriptions.
	std::array<dnnl::convolution_forward, 3> convs;

	void init_src_descs(const std::array<unsigned short, 4>& chn,
						const unsigned short img_w, const unsigned short img_h);
	void init_ker_descs(const std::array<unsigned short, 4>& chn,
						const std::array<unsigned short, 3>& ker);
	void init_bias_descs(const std::array<unsigned short, 4>& chn);
	void init_dest_descs(const std::array<unsigned short, 4>& chn,
						 const unsigned short img_w, const unsigned short img_h);
	void init_pads(const std::array<unsigned short, 3>& ker);
	void init_conv();

public:
	SRCNN(const unsigned short img_w, const unsigned short img_h, const SRCNNDesc& desc);

	std::array<dnnl::memory::desc, 3> get_ker_descs() const {
		return ker_descs;
	}

	std::array<dnnl::memory::desc, 3> get_bias_descs() const {
		return bias_descs;
	}

	dnnl::memory::desc get_input_desc() const {
		return src_descs[0];
	}

	dnnl::memory::desc get_output_desc() const {
		return dest_descs[2];
	}

	dnnl::engine get_engine() const {
		return eng;
	}

	void execute(dnnl::memory src_mem, std::array<dnnl::memory, 3> ker_mem,
				 std::array<dnnl::memory, 3> bias_mem, dnnl::memory dest_mem);
};
