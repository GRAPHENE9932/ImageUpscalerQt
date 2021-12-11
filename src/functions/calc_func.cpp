#include "func.h"

unsigned long long func::srcnn_operations_amount(std::array<unsigned short, 3> kernels,
												 std::array<unsigned short, 4> channels,
												 std::array<int, 4> widths,
												 std::array<int, 4> heights) {
	//Use formula for it.
	//O=sum from{i=1} to{3} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}

	unsigned long long result = 0;

	for (unsigned char i = 0; i < 3; i++) {
		result +=
			(unsigned long long)widths[i + 1] *
			(unsigned long long)heights[i + 1] *
			(unsigned long long)channels[i + 1] *
			(
				2ull *
				(unsigned long long)channels[i] *
				(unsigned long long)kernels[i] *
				(unsigned long long)kernels[i] -
				1ull
			) +
			(unsigned long long)widths[i + 1] *
			(unsigned long long)heights[i + 1] *
			1ull;
	}

	return result;
}

unsigned long long func::fsrcnn_operations_amount(std::vector<unsigned short> kernels,
												  std::vector<unsigned short> channels,
												  std::vector<int> widths,
												  std::vector<int> heights) {
	assert(kernels.size() + 1 == channels.size() && channels.size() == widths.size() &&
		   widths.size() == heights.size());

	//Use formula for it
	//O=sum from{i=1} to{n} W_{i+1}^2 times C_{i+1}( 2C_i times K_i^2 - 1 ) + {W_{ i+1 }^2 times O_a}
	//(LibreOffice Math)

	unsigned long long result = 0;

	for (unsigned char i = 0; i < kernels.size() - 1; i++) {
		result +=
			(unsigned long long)widths[i + 1] *
			(unsigned long long)heights[i + 1] *
			(unsigned long long)channels[i + 1] *
			(
				2ull *
				(unsigned long long)channels[i] *
				(unsigned long long)kernels[i] *
				(unsigned long long)kernels[i] -
				1ull
			) +
			(unsigned long long)widths[i + 1] *
			(unsigned long long)heights[i + 1] *
			1ull;
	}

	return result;
}

unsigned long long func::measure_cnn_memory_consumption(std::array<unsigned short, 4> channels,
														std::array<int, 4> widths,
														std::array<int, 4> heights) {
	std::vector<unsigned short> channels_vec(channels.begin(), channels.end());
	std::vector<int> widths_vec(widths.begin(), widths.end());
	std::vector<int> heights_vec(heights.begin(), heights.end());

	return measure_cnn_memory_consumption(channels_vec, widths_vec, heights_vec);
}

///APPROXIMATE minimum memory consumption of tensors that going throught the CNN
///@returns Amount of bytes that will be consumed
unsigned long long func::measure_cnn_memory_consumption(std::vector<unsigned short> channels,
														std::vector<int> widths,
														std::vector<int> heights) {
	assert(channels.size() == widths.size() && widths.size() == heights.size());

	//Iterate throught every convolutional layer to find the point with maximum memory consumption
	//Considering the channels amount, width and height (in short, tensor size)
	// |  1  | --- | 128 | --- |  64 | --- |  1  |
	//                      ^
	//    max consumption point = size 1 + size 2

	unsigned long long max_point = 0;

	for (unsigned char i = 0; i < channels.size() - 1; i++) {
		unsigned long long cur_max_point = (unsigned long long)widths[i] * (unsigned long long)heights[i] *
										   (unsigned long long)channels[i] +
										   (unsigned long long)widths[i + 1] *
										   (unsigned long long)heights[i + 1] *
										   (unsigned long long)channels[i + 1];

		if (cur_max_point > max_point)
			max_point = cur_max_point;
	}

	//Now, we have amount of numbers, but they are in float type, so multiply it to convert it to bytes
	max_point *= sizeof(float);

	//Weirdness of the PyTorch. The real consumption is 4 times greater than calculated just now. So...
	max_point *= 4ull;

	return max_point;
}
