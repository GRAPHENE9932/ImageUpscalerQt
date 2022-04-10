/*
 * ImageUpscalerQt - string functions
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <unordered_map>
#include <cmath>

#include <QStringList>
#include <QCollator>
#include <QCoreApplication>
#include <QMap>
#include <OpenImageIO/imageio.h>

#include "func.hpp"

std::vector<int> func::duplicate_indexes(const QStringList& list) {
	std::vector<int> indexes;
	auto list_size = list.size();

	// Use an unordered map to find duplicates.
	// Insert if not already present, mark as duplicate if already present
	std::unordered_map<QString, int> map;
	for (int i = 0; i < list_size; i++) {
		if (map.contains(list[i])) {
			indexes.push_back(i);
			indexes.push_back(map[list[i]]);
		}
		else {
			map.insert(std::make_pair(list[i], i));
		}
	}

	return indexes;
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

QString func::separate_string_with_char(QString orig, QChar spl_char, int sec_size, int right_start) {
	QString result;
	for (int i = 0; i < orig.size(); i++) {
		if ((orig.size() - i - right_start) % sec_size == 0 && i < orig.size() - right_start)
			result += spl_char;
		result += orig[i];
	}
	return result;
}

QString func::big_number_to_string(long long num, QChar separator) {
	// Convert the number to string (without sign).
	QString str = QString::number(std::abs(num));

	// Add gaps between every third order.
	str = separate_string_with_char(str, separator, 3, 0);

	// Add the minus if needed.
	if (num < 0)
		str.insert(0, '-');

	return str;
}

QString func::bytes_amount_to_string(unsigned long long bytes) {
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

QString func::pixel_amount_to_string(unsigned long long pixels) {
	if (pixels == 0)
		return "0 MP";

	// If there are less than 1 MP, leave exactly 1 non-zero number after the point.
	if (pixels < 1'000'000ull) {
		int exp = std::floor(std::log10(pixels / 1'000'000.0l));
		return QString::number(pixels / 1'000'000.0l, 'f', -exp) + " MP";
	}
	// Megapixels.
	else if (pixels < 1'000'000'000ull) {
		return QString::number(pixels / 1'000'000.0l, 'f', 1) + " MP";
	}
	// Gigapixels.
	else if (pixels < 1'000'000'000'000ull) {
		return QString::number(pixels / 1'000'000'000.0l, 'f', 1) + " GP";
	}
	// Terapixels.
	else {
		return QString::number(pixels / 1'000'000'000'000.0l, 'f', 1) + " TP";
	}
}

QString func::milliseconds_to_string(unsigned long long millis) {
	auto hours = millis / (1000ull * 60ull * 60ull);
	auto minutes = millis / (1000ull * 60ull) % 60ull;
	auto seconds = millis / 1000ull % 60ull;
	millis %= 1000ull;

	return QString("%1:%2:%3.%4").arg(
		QString::number(hours).rightJustified(2, '0'),
		QString::number(minutes).rightJustified(2, '0'),
		QString::number(seconds).rightJustified(2, '0'),
		QString::number(millis).rightJustified(3, '0')
	);
}

QString func::shorten_file_path(const QString& orig) {
	if (orig.startsWith("/home/") || orig.startsWith("C:/Users/")) {
		const int depth = orig.count('/');
		if (depth >= 3)
			return "~/" + orig.section('/', 3);
	}

	return orig;
}

const QMap<QString, QString> FORMAT_NAMES = {
	{"png", "PNG"},
	{"jpeg", "JPEG"},
	{"webp", "WebP"},
	{"heif", "HEIF/AVIF"},
	{"bmp", "BMP"},
	{"jpeg2000", "JPEG 2000"},
	{"gif", "GIF"},
	{"ico", "ICO"},
	{"tiff", "TIFF"},
	{"cineon", "Cineon"},
	{"dds", "DirecrDraw surface"},
	{"dpx", "DPX"},
	{"fits", "FITS"},
	{"hdr", "RGBE"},
	{"iff", "IFF"},
	{"null", "NULL"},
	{"openexr", "OpenEXR"},
	{"pnm", "PNM"},
	{"psd", "PSD"},
	{"raw", "RAW"},
	{"rla", "RLA"},
	{"sgi", "SGI"},
	{"softimage", "SoftImage"},
	{"targa", "Targa"}
};

QString get_format_name(const QString& format) {
	if (FORMAT_NAMES.contains(format))
		return FORMAT_NAMES[format] + " image";

	return format + " image";
}

QStringList sort_formats(const QStringList& orig_list) {
	QMultiMap<int, QString> map;
	for (const QString& format : orig_list) {
		int priority = 0;

		if (format == "png")
			priority = 9;
		else if (format == "jpeg")
			priority = 8;
		else if (format == "webp")
			priority = 7;
		else if (format == "heif")
			priority = 6;
		else if (format == "bmp")
			priority = 5;
		else if (format == "jpeg2000")
			priority = 4;
		else if (format == "gif")
			priority = 3;
		else if (format == "ico")
			priority = 2;
		else if (format == "tiff")
			priority = 1;

		map.insert(priority, format);
	}
	QStringList str_list = map.values();
	std::reverse(str_list.begin(), str_list.end());
	return str_list;
}

QString input_wildcard;
QString func::get_image_input_wildcard() {
	if (!input_wildcard.isEmpty()) // Return the cached string if it exist.
		return input_wildcard;

	std::map<std::string, std::vector<std::string>> ext_map = OIIO::get_extension_map();
	QStringList input_formats = QString::fromStdString(OIIO::get_string_attribute("input_format_list")).split(',');
	input_formats = sort_formats(input_formats);

	// All extensions part.
	QString result = QCoreApplication::translate("ImageUpscalerQt", "All images(");
	for (QString& format : input_formats) {
		auto extensions = ext_map[format.toStdString()];
		for (std::string& ext : extensions)
			result += "*." + QString::fromStdString(ext) + ' ';
	}
	result[result.size() - 1] = ')'; // Replace the last ' ' with ')'.
	result += ";;";

	// Other formats one by one.
	for (QString& format : input_formats) {
		result += get_format_name(format) + '(';
		auto extensions = ext_map[format.toStdString()];
		for (std::string& ext : extensions)
			result += "*." + QString::fromStdString(ext) + ' ';
		result[result.size() - 1] = ')';
		result += ";;";
	}

	input_wildcard = result;
	return result;
}

QString output_wildcard;
QString func::get_image_output_wildcard() {
	if (!output_wildcard.isEmpty())
		return output_wildcard;

	std::map<std::string, std::vector<std::string>> ext_map = OIIO::get_extension_map();
	QStringList output_formats = QString::fromStdString(OIIO::get_string_attribute("output_format_list")).split(',');
	output_formats = sort_formats(output_formats);

	// Image formats one by one.
	QString result;
	for (QString& format : output_formats) {
		result += get_format_name(format) + '(';
		auto extensions = ext_map[format.toStdString()];
		for (std::string& ext : extensions)
			result += "*." + QString::fromStdString(ext) + ' ';
		result[result.size() - 1] = ')';
		result += ";;";
	}

	output_wildcard = result;
	return result;
}
