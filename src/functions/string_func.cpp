#include <QStringList>
#include <QCollator>

#include "func.h"

QStringList func::split(QString str, QChar c) {
	return str.split(c);
}

int func::char_count(QString str, QChar c) {
	return str.count(c);
}

bool func::parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
					   std::array<unsigned short, 3>* paddings_out,
					   std::array<unsigned short, 4>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 4> channels = {1, 0, 0, 1};

		// BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		if (splitted_ker.size() != 3)
			return false;

		bool bad_params = false;

		for (unsigned char i = 0; i < 3; i++) {
			// Parse name.
			kernels[i] = splitted_ker[i].toUShort();
			paddings[i] = (kernels[i] - 1) / 2; // Formulas simplified from (w - k + 2p) / s + 1.
			bad_params |= kernels[i] % 2 != 1;
		}

		if (bad_params)
			return false;
		// END

		// BEGIN Parse second part (channels)
		QStringList splitted_ch = split(parts[1], '-');

		if (splitted_ch.size() != 2)
			return false;

		for (unsigned char i = 0; i < 2; i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		// END

		// Return values.
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool func::parse_fsrcnn(QString str, std::vector<unsigned short>* kernels_out,
					    std::vector<unsigned short>* paddings_out,
						std::vector<unsigned short>* channels_out) {
	try {
		QStringList parts = split(str, ' ');

		if (parts.size() != 2)
			return false;

		// BEGIN Parse first part (kernels)
		QStringList splitted_ker = split(parts[0], '-');

		// Initialize kernels and paddigns vectors.
		std::vector<unsigned short> kernels(splitted_ker.size(), 0);
		std::vector<unsigned short> paddings(splitted_ker.size(), 0);

		bool bad_params = false;

		for (uint8_t i = 0; i < splitted_ker.size(); i++) {
			kernels[i] = splitted_ker[i].toUShort();
			if (i == splitted_ker.size() - 1) { // Last element (conv transpos).
				paddings[i] = (kernels[i] - 3) / 2;
			}
			else {
				paddings[i] = (kernels[i] - 1) / 2; // Formulas simplified from (w - k + 2p) / s + 1.
				bad_params |= (kernels[i] % 2) != 1;
			}
		}

		if (bad_params)
			return false;

		// END

		// BEGIN Parse second part (channels)
		QStringList splitted_ch = split(parts[1], '-');
		// Inititalize channels vector.
		std::vector<unsigned short> channels(splitted_ker.size() + 1, 0);
		channels[0] = 1;
		channels[channels.size() - 1] = 1;

		for (uint8_t i = 0; i < splitted_ch.size(); i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		// END

		// Return values.
		if (kernels_out != nullptr) *kernels_out = kernels;
		if (paddings_out != nullptr) *paddings_out = paddings;
		if (channels_out != nullptr) *channels_out = channels;
		return true;
	}
	catch (...) {
		return false;
	}
}

QString func::srcnn_to_string(const std::array<unsigned short, 3> kernels,
							  const std::array<unsigned short, 4> channels) {
	// 5-1-9 64-32.
	return QString("%1-%2-%3 %4-%5").arg(QString::number(kernels[0]),
										 QString::number(kernels[1]),
										 QString::number(kernels[2]),
										 QString::number(channels[1]),
										 QString::number(channels[2]));
}

QString func::fsrcnn_to_string(const std::vector<unsigned short> kernels,
							   const std::vector<unsigned short> channels) {
	QString output;

	for (size_t i = 0; i < kernels.size(); i++) {
		output += QString::number(kernels[i]);
		if (i != kernels.size() - 1)
			output += QChar('-');
	}

	output += QChar(' ');

	for (size_t i = 1; i < channels.size() - 1; i++) {
		output += QString::number(channels[i]);
		if (i != channels.size() - 2)
			output += QChar('-');
	}

	return output;
}

void func::numerical_sort(QStringList& list) {
	QCollator collator;
	collator.setNumericMode(true);

	std::sort(
		list.begin(),
		list.end(),
		[&](const QString& str_1, const QString& str_2) {
			return collator.compare(str_1, str_2) < 0;
		}
	);
}

QString func::big_number_to_string(long long num, QChar separator) {
	// Get separate digits.
	QString original = QString::number(num);

	// Do this.
	QString result = "";
	for (int i = 0; i < original.size(); i++) {
		if ((original.size() - i) % 3 == 0 && i != 0 && i != original.size() - 1 && original[i] != '-')
			result += separator;

		result += original[i];
	}

	return result;
}

QString func::bytes_amount_to_string(unsigned long long bytes)
{
	if (bytes < 1024) { // Bytes.
		return QString::number(bytes) + " B";
	}
	else if (bytes < 1024 * 1024) { // Kibibytes.
		return QString::number(bytes / 1024.0, 'f', 1) + " KiB";
	}
	else if (bytes < 1024 * 1024 * 1024) { // Mebibytes.
		return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MiB";
	}
	else { // Gibibytes.
		return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GiB";
	}
}
