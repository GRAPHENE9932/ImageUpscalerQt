/*
 * ImageUpscalerQt - Super Resolution Convolutional Neural Network header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include <dnnl.hpp>

#include "../tasks/TaskDesc.h"

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

public:
	static SRCNN create(unsigned short img_w, unsigned short img_h, SRCNNDesc desc);

	static SRCNN create(std::array<dnnl::memory::dims, 3> src_dims,
						std::array<dnnl::memory::dims, 3> ker_dims,
						std::array<dnnl::memory::dims, 3> bias_dims,
						std::array<dnnl::memory::dims, 3> dest_dims,
						std::array<dnnl::memory::dims, 3> pads_l,
						std::array<dnnl::memory::dims, 3> pads_r);

	SRCNN(dnnl::engine eng, dnnl::stream eng_str, std::array<dnnl::memory::desc, 3> src_descs,
		  std::array<dnnl::memory::desc, 3> ker_descs, std::array<dnnl::memory::desc, 3> bias_descs,
		  std::array<dnnl::memory::desc, 3> dest_descs, std::array<dnnl::memory::dims, 3> pads_l,
		  std::array<dnnl::memory::dims, 3> pads_r,
		  std::array<dnnl::convolution_forward, 3> convs) :

		  eng(eng), eng_str(eng_str), src_descs(src_descs), ker_descs(ker_descs),
		  bias_descs(bias_descs), dest_descs(dest_descs), pads_l(pads_l),
		  pads_r(pads_r), convs(convs) {}

	std::array<dnnl::memory::desc, 3> get_ker_descs() {
		return ker_descs;
	}

	std::array<dnnl::memory::desc, 3> get_bias_descs() {
		return bias_descs;
	}

	dnnl::memory::desc get_input_desc() {
		return src_descs[0];
	}

	dnnl::memory::desc get_output_desc() {
		return dest_descs[2];
	}

	dnnl::engine get_engine() {
		return eng;
	}

	void execute(dnnl::memory src_mem, std::array<dnnl::memory, 3> ker_mem,
				 std::array<dnnl::memory, 3> bias_mem, dnnl::memory dest_mem);
};
