#pragma once

#include <array>
#include <string>
#include <vector>

namespace Algorithms {
	std::vector<std::string> split(std::string str, char c);

	unsigned long long char_count(std::string str, char c);

	bool parse_srcnn(std::string str, std::array<unsigned short, 3>* kernels_out,
					 std::array<unsigned short, 3>* paddings_out,
					 std::array<unsigned short, 2>* channels_out);

	bool parse_fsrcnn(std::string str, std::array<unsigned short, 4>* kernels_out,
					  std::array<unsigned short, 4>* paddings_out,
					  std::array<unsigned short, 3>* channels_out);

	std::string srcnn_to_string(std::array<unsigned short, 3> kernels,
								std::array<unsigned short, 2> channels);

	std::string fsrcnn_to_string(std::array<unsigned short, 4> kernels,
								 std::array<unsigned short, 3> channels);
}
