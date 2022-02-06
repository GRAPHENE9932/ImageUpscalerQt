/*
 * ImageUpscalerQt - string functions
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <unordered_map>
#include <cmath>

#include <QStringList>
#include <QCollator>

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

QStringList func::shorten_file_paths(const QStringList& list) {
	auto list_size = list.size();
	// Leave only file name.
	// For the files that have the same names, we will show the folder name they contained in.
	// So we have to find duplicate file names.
	QStringList file_names;
	for (int i = 0; i < list_size; i++)
		file_names += list[i].section('/', -1); // Leave only file name.

	std::vector<int> dups = func::duplicate_indexes(file_names);

	// Use duplicates data now.
	for (int i = 0; i < dups.size(); i++) {
		file_names[dups[i]] = shorten_file_path(list[dups[i]]);
	}

	// Return result.
	return file_names;
}

QString func::shorten_file_path(const QString path) {
	// Some examples of the shortened duplicates:
	// /home/user/image.png -> .../user/image.png
	// /home/image.png -> /home/image.png
	// /image.png -> /image.png
	// C:/image.png -> C:/image.png
	// C:/Users/image.png -> C:/Users/image.png

	int folder_depth = path.count('/');
	QString folder_lvl_2 = path.section('/', -3, -3);
	if (folder_depth > 1) {
		// For Windows, check if the second folder is 'X:', where X - uppercase english letter.
#ifdef Q_OS_WIN
		const QString ENG_ABC = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		if (folder_lvl_2.size() == 2 &&
			ENG_ABC.contains(folder_lvl_2[0]) &&
			folder_lvl_2[1] == ':')
			return path.section('/', -3);
#endif
		// For UNIX, check if the second folder is root.
#ifdef Q_OS_UNIX
		if (folder_lvl_2 == "")
			return path.section('/', -3);
#endif
		// If level 2 folder is not root (or drive letter), just do ".../foo/bar.file".
		return ".../" + path.section('/', -2);
	}
	else if (folder_depth == 1) {
		return path.section('/', -2);
	}
	else {
		// folder_depth == 0 is impossible, but return the whole name (just in case).
		return path;
	}
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
