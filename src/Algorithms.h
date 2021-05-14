#pragma once

#include <array>
#include <string>
#include <vector>

namespace Algorithms {
	std::vector<std::string> split(std::string str, char c);

	unsigned long long char_count(std::string str, char c);

	bool parse_srcnn(std::string str, std::array<unsigned char, 3>* kernels_out,
					 std::array<unsigned char, 3>* paddings_out, std::array<unsigned char, 2>* channels_out);

	bool parse_fsrcnn(std::string str, std::array<unsigned char, 4>* kernels_out,
					 std::array<unsigned char, 4>* paddings_out, std::array<unsigned char, 3>* channels_out);
}
