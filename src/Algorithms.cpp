#include <sstream>

#include "Algorithms.h"

std::vector<std::string> Algorithms::split(std::string str, char c) {
	size_t pos = 0;
	std::vector<std::string> res;
	std::string token;
	while ((pos = str.find(c)) != std::string::npos) {
		token = str.substr(0, pos);
		res.push_back(token);
		str.erase(0, pos + 1);
	}
	res.push_back(str);

	return res;
}

unsigned long long Algorithms::char_count(std::string str, char c) {
	uint64_t res = 0;

	for (uint64_t i = 0; i < str.size(); i++)
		res += str[i] == c;

	return res;
}

bool Algorithms::parse_srcnn(std::string str, std::array<unsigned short, 3>* kernels_out,
							 std::array<unsigned short, 3>* paddings_out,
							 std::array<unsigned short, 2>* channels_out) {
	try {
		std::vector<std::string> parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 2> channels = {0, 0};

		//BEGIN Parse first part (kernels)
		std::vector<std::string> splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 3)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 3; i++) {
			//Parse name
			kernels[i] = std::stoi(splitted_ker[i]);
			paddings[i] = (kernels[i] - 1) / 2; //Formulas simplified from (w - k + 2p) / s + 1
			bad_params |= kernels[i] % 2 != 1;
		}

		if (bad_params)
			return false;

		//END

		//BEGIN Parse second part (channels)
		std::vector<std::string> splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 2)
			return false;

		for (uint8_t i = 0; i < 2; i++)
			channels[i] = std::stoi(splitted_ch[i]);
		//END

		//Return values
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool Algorithms::parse_fsrcnn(std::string str, std::array<unsigned short, 4>* kernels_out,
							  std::array<unsigned short, 4>* paddings_out,
							  std::array<unsigned short, 3>* channels_out) {
	try {
		std::vector<std::string> parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 4> kernels = {0, 0, 0, 0};
		std::array<unsigned short, 4> paddings = {0, 0, 0, 0};
		std::array<unsigned short, 3> channels = {0, 0, 0};

		//BEGIN Parse first part (kernels)
		std::vector<std::string> splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 4)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 4; i++) {
			//Parse name
			kernels[i] = std::stoi(splitted_ker[i]);
			if (i == splitted_ker.size() - 1) { //If last (last layer - deconvolutional)
				paddings[i] = (kernels[i] - 2) / 2;
				bad_params |= kernels[i] % 2 != 0;
			}
			else {
				paddings[i] = (kernels[i] - 1) / 2; //Formulas simplified from (w - k + 2p) / s + 1
				bad_params |= kernels[i] % 2 != 1;
			}
		}

		if (bad_params)
			return false;

		//END

		//BEGIN Parse second part (channels)
		std::vector<std::string> splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 3)
			return false;

		for (uint8_t i = 0; i < 3; i++)
			channels[i] = std::stoi(splitted_ch[i]);
		//END

		//Return values
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

std::string Algorithms::srcnn_to_string(const std::array<unsigned short, 3> kernels,
										const std::array<unsigned short, 2> channels) {
	std::stringstream ss;
	ss << kernels[0] << '-' << kernels[1] << '-' << kernels[2] << ' ' <<
		channels[0] << '-' << channels[1];

	return ss.str();
}

std::string Algorithms::fsrcnn_to_string(const std::array<unsigned short, 4> kernels,
										 const std::array<unsigned short, 3> channels) {
	std::stringstream ss;
	ss << kernels[0] << '-' << kernels[1] << '-' << kernels[2] << '-' << kernels[3] << ' ' <<
		channels[0] << '-' << channels[1] << channels[2];

	return ss.str();
}
