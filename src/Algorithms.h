#pragma once

#include <array>
#include <string>
#include <vector>

#include <QString>
#include <QStringList>

namespace Algorithms {
	QStringList split(QString str, QChar c);

	int char_count(QString str, QChar c);

	bool parse_srcnn(QString str, std::array<unsigned short, 3>* kernels_out,
					 std::array<unsigned short, 3>* paddings_out,
					 std::array<unsigned short, 2>* channels_out);

	bool parse_fsrcnn(QString str, std::array<unsigned short, 4>* kernels_out,
					  std::array<unsigned short, 4>* paddings_out,
					  std::array<unsigned short, 3>* channels_out);

	QString srcnn_to_string(std::array<unsigned short, 3> kernels,
								std::array<unsigned short, 2> channels);

	QString fsrcnn_to_string(std::array<unsigned short, 4> kernels,
								 std::array<unsigned short, 3> channels);
}
