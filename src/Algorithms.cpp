#include <sstream>

#include "Algorithms.h"

QStringList Algorithms::split(QString str, QChar c) {
	return str.split(c);
}

int Algorithms::char_count(QString str, QChar c) {
	return str.count(c);
}

bool Algorithms::parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
							 std::array<unsigned short, 3>* paddings_out,
							 std::array<unsigned short, 2>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 2> channels = {0, 0};

		//BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 3)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 3; i++) {
			//Parse name
			kernels[i] = splitted_ker[i].toUShort();
			paddings[i] = (kernels[i] - 1) / 2; //Formulas simplified from (w - k + 2p) / s + 1
			bad_params |= kernels[i] % 2 != 1;
		}

		if (bad_params)
			return false;

		//END

		//BEGIN Parse second part (channels)
		QStringList splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 2)
			return false;

		for (uint8_t i = 0; i < 2; i++)
			channels[i] = splitted_ch[i].toUShort();
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

bool Algorithms::parse_fsrcnn(QString str, std::array<unsigned short, 4>* kernels_out,
							  std::array<unsigned short, 4>* paddings_out,
							  std::array<unsigned short, 3>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 4> kernels = {0, 0, 0, 0};
		std::array<unsigned short, 4> paddings = {0, 0, 0, 0};
		std::array<unsigned short, 3> channels = {0, 0, 0};

		//BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 4)
			return false;

		bool bad_params = false;

		for (uint8_t i = 0; i < 4; i++) {
			//Parse name
			kernels[i] = splitted_ker[i].toUShort();
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
		QStringList splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 3)
			return false;

		for (uint8_t i = 0; i < 3; i++)
			channels[i] = splitted_ch[i].toUShort();
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

QString Algorithms::srcnn_to_string(const std::array<unsigned short, 3> kernels,
									const std::array<unsigned short, 2> channels) {
	//5-1-9 64-32
	return QString("%1-%2-%3 %4-%5").arg(QString::number(kernels[0]),
										 QString::number(kernels[1]),
										 QString::number(kernels[2]),
										 QString::number(channels[0]),
										 QString::number(channels[1]));
}

QString Algorithms::fsrcnn_to_string(const std::array<unsigned short, 4> kernels,
									 const std::array<unsigned short, 3> channels) {
	//3-1-3-4 512-32-64
	return QString("%1-%2-%3-%4 %5-%6").arg(QString::number(kernels[0]),
										    QString::number(kernels[1]),
										    QString::number(kernels[2]),
										    QString::number(kernels[3]),
										    QString::number(channels[0]),
										    QString::number(channels[1]),
										    QString::number(channels[2]));
}
