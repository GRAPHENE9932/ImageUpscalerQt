/*
 * ImageUpscalerQt - task descriptions
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QStringList>
#include <QCoreApplication>

#include "TaskDesc.hpp"

// BEGIN SRCNN

QString SRCNNDesc::to_string() const {
	// 5-1-9 64-32.
	return QString("%1-%2-%3 %4-%5").arg(QString::number(kernels[0]),
										 QString::number(kernels[1]),
										 QString::number(kernels[2]),
										 QString::number(channels[1]),
										 QString::number(channels[2]));
}

QString TaskSRCNNDesc::to_string() const {
	return QCoreApplication::translate("ImageUpscalerQt", "Use SRCNN %1").arg(srcnn_desc.to_string());
}

QSize TaskSRCNNDesc::img_size_after(QSize cur_size) const {
	return cur_size;
}

bool SRCNNDesc::from_string(QString str, SRCNNDesc* srcnn) {
	try {
		QStringList parts = str.split(' ');

		if (parts.size() != 2)
			return false;

		std::array<unsigned short, 3> kernels = {0, 0, 0};
		std::array<unsigned short, 3> paddings = {0, 0, 0};
		std::array<unsigned short, 4> channels = {1, 0, 0, 1};

		// BEGIN Parse first part (kernels)
		QStringList splitted_ker = parts[0].split('-');

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
		QStringList splitted_ch = parts[1].split('-');

		if (splitted_ch.size() != 2)
			return false;

		for (unsigned char i = 0; i < 2; i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		// END

		// Return values.
		if (srcnn != nullptr) {
			srcnn->kernels = kernels;
			srcnn->paddings = paddings;
			srcnn->channels = channels;
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

bool SRCNNDesc::operator<(const SRCNNDesc right) {
	for (unsigned char i = 0; i < 3; i++)
		if (kernels[i] < right.kernels[i])
			return true;
		else if (kernels[i] > right.kernels[i])
			return false;

	for (unsigned char i = 0; i < 3; i++)
		if (channels[i] < right.channels[i])
			return true;
		else if (channels[i] > right.channels[i])
			return false;

	return false;
}

bool SRCNNDesc::operator>(const SRCNNDesc right) {
	return !(*this < right || *this == right);
}

bool SRCNNDesc::operator==(const SRCNNDesc right) {
	return kernels == right.kernels && channels == right.channels;
}

// END SRCNN

// BEGIN FSRCNN

QString FSRCNNDesc::to_string() const {
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

QString TaskFSRCNNDesc::to_string() const {
	return QCoreApplication::translate("ImageUpscalerQt", "Use FSRCNN %1").arg(fsrcnn_desc.to_string());
}

QSize TaskFSRCNNDesc::img_size_after(QSize cur_size) const {
	return cur_size * 3;
}

bool FSRCNNDesc::from_string(QString str, FSRCNNDesc* fsrcnn) {
	try {
		QStringList parts = str.split(' ');

		if (parts.size() != 2)
			return false;

		// BEGIN Parse first part (kernels)
		QStringList splitted_ker = parts[0].split('-');

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
		QStringList splitted_ch = parts[1].split('-');
		// Inititalize channels vector.
		std::vector<unsigned short> channels(splitted_ker.size() + 1, 0);
		channels[0] = 1;
		channels[channels.size() - 1] = 1;

		for (uint8_t i = 0; i < splitted_ch.size(); i++)
			channels[i + 1] = splitted_ch[i].toUShort();
		// END

		// Return values.
		if (fsrcnn != nullptr) {
			fsrcnn->kernels = kernels;
			fsrcnn->paddings = paddings;
			fsrcnn->channels = channels;
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

bool FSRCNNDesc::operator<(const FSRCNNDesc right) {
	if (kernels.size() < right.kernels.size() || channels.size() < right.channels.size())
		return true;

	// After previous statement, we can assume that amount of kernels and channels
	// are equal in both operands.
	for (unsigned char i = 0; i < kernels.size(); i++)
		if (kernels[i] < right.kernels[i])
			return true;
		else if (kernels[i] > right.kernels[i])
			return false;

	for (unsigned char i = 0; i < channels.size(); i++)
		if (channels[i] < right.channels[i])
			return true;
		else if (channels[i] > right.channels[i])
			return false;

	return false;
}

bool FSRCNNDesc::operator>(const FSRCNNDesc right) {
	return !(*this < right || *this == right);
}

bool FSRCNNDesc::operator==(const FSRCNNDesc right) {
	return kernels == right.kernels && channels == right.channels;
}

// END FSRCNN

QString TaskResizeDesc::to_string() const {
	return QString("Resize to %1x%2 | %3").arg(QString::number(size.width()),
											   QString::number(size.height()),
											   INTERPOLATION_NAMES[(unsigned char)interpolation]);
}

QSize TaskResizeDesc::img_size_after(QSize) const {
	return size;
}


QString TaskConvertColorSpaceDesc::to_string() const {
	return QCoreApplication::translate("ImageUpscalerQt", "Convert from %1").arg(
		QCoreApplication::translate("ImageUpscalerQt",
		COLOR_SPACE_CONVERSION_NAMES[(unsigned char)color_space_conversion])
	);
}

QSize TaskConvertColorSpaceDesc::img_size_after(QSize cur_size) const {
	return cur_size;
}
