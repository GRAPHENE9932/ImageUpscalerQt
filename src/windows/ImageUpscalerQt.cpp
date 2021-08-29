/*
 * ImageUpscalerQt main window
 * Copyright (C) 2021  Artem Kliminskyi <artemklim50@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <array>
#include <filesystem>
#include <sstream>

#include <QtDebug>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QFileDialog>

#include "ImageUpscalerQt.h"
#include "ui_ImageUpscalerQt.h"
#include "../Algorithms.h"
#include "../tasks/TaskResize.h"
#include "../tasks/TaskSRCNN.h"
#include "../tasks/TaskFSRCNN.h"
#include "../tasks/TaskConvertColorSpace.h"
#include "TasksWaitingDialog.h"

#define VERSION "1.3"

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) : QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt) {
    m_ui->setupUi(this);

	//Set window icon
	setWindowIcon(QIcon(":Icon.png"));

	//BEGIN Connect signals
	connect(m_ui->add_task_button, SIGNAL(clicked()), this, SLOT(add_task_clicked()));
	connect(m_ui->task_kind_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(task_kind_changed(int)));
	connect(m_ui->select_image_button, SIGNAL(clicked()), this, SLOT(select_image_clicked()));
	connect(m_ui->remove_task_button, SIGNAL(clicked()), this, SLOT(remove_task_clicked()));
	connect(m_ui->clear_tasks_button, SIGNAL(clicked()), this, SLOT(clear_tasks_clicked()));
	connect(m_ui->up_task_button, SIGNAL(clicked()), this, SLOT(move_task_up_clicked()));
	connect(m_ui->down_task_button, SIGNAL(clicked()), this, SLOT(move_task_down_clicked()));
	connect(m_ui->resize_x, SIGNAL(valueChanged(int)), this, SLOT(resize_x_changed(int)));
	connect(m_ui->resize_y, SIGNAL(valueChanged(int)), this, SLOT(resize_y_changed(int)));
	connect(m_ui->keep_image_ratio_radio, SIGNAL(toggled(bool)), this, SLOT(keep_ratio_toggled(bool)));
	connect(m_ui->start_tasks_button, SIGNAL(clicked()), this, SLOT(start_tasks_clicked()));
	connect(m_ui->about_button, SIGNAL(clicked()), this, SLOT(about_clicked()));
	connect(m_ui->srcnn_architecture_combobox, SIGNAL(currentTextChanged(QString)), this, SLOT(srcnn_architecture_changed(QString)));
	connect(m_ui->srcnn_block_size_spinbox, SIGNAL(valueChanged(int)), this, SLOT(srcnn_block_size_changed(int)));
	connect(m_ui->srcnn_block_split_check, SIGNAL(stateChanged(int)), this, SLOT(srcnn_split_checked(int)));
	connect(m_ui->fsrcnn_architecture_combobox, SIGNAL(currentTextChanged(QString)), this, SLOT(fsrcnn_architecture_changed(QString)));
	connect(m_ui->fsrcnn_block_size_spinbox, SIGNAL(valueChanged(int)), this, SLOT(fsrcnn_block_size_changed(int)));
	connect(m_ui->fsrcnn_block_split_check, SIGNAL(stateChanged(int)), this, SLOT(fsrcnn_split_checked(int)));
	//END Connect signals
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

//BEGIN Slots
void ImageUpscalerQt::add_task_clicked() {
	//Add task
	switch ((TaskKind)m_ui->task_kind_combobox->currentIndex()) {
		case TaskKind::resize: {
			TaskResize* cur_task = new TaskResize(
				m_ui->resize_x->value(),
				m_ui->resize_y->value(),
				(Interpolation)m_ui->resize_interpolation_combobox->currentIndex()
			);

			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::srcnn: {
			//Get SRCNN name
			QString name = m_ui->srcnn_architecture_combobox->currentText();
			//Parse SRCNN
			std::array<unsigned short, 3> kernels;
			std::array<unsigned short, 3> paddings;
			std::array<unsigned short, 4> channels;
			Algorithms::parse_srcnn(name, &kernels, &paddings, &channels);

			//Get block size
			unsigned int block_size;
			if (m_ui->srcnn_block_split_check->isChecked())
				block_size = m_ui->srcnn_block_size_spinbox->value();
			else
				block_size = 0; //0 means do not split image

			//Create task
			TaskSRCNN* cur_task = new TaskSRCNN(kernels, paddings, channels, block_size);
			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::fsrcnn: {
			//Get FSRCNN name
			QString name = m_ui->fsrcnn_architecture_combobox->currentText();
			//Parse FSRCNN
			std::array<unsigned short, 4> kernels;
			std::array<unsigned short, 4> paddings;
			std::array<unsigned short, 5> channels;
			Algorithms::parse_fsrcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskFSRCNN* cur_task = new TaskFSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);

			//Update task info because the new FSRCNN will change the end size of the image
			update_fsrcnn_info();
			break;
		}
		case TaskKind::convert_color_space: {
			TaskConvertColorSpace* cur_task = new TaskConvertColorSpace(
				(ColorSpaceConversion)m_ui->color_space_combobox->currentIndex()
			);

			task_queue.push_back(cur_task);
		}
	}

	update_list();
}

void ImageUpscalerQt::task_kind_changed(int index) {
	//Set up the right part of the window
	switch ((TaskKind)index) {
		case TaskKind::resize: {
			//Is image selected and valid?
			if (!image_spec.undefined()) {
				//Set image width and height the same
				m_ui->resize_x->setValue(image_spec.width);
				m_ui->resize_y->setValue(image_spec.height);
				//Enable keep ratio radio button
				m_ui->keep_image_ratio_radio->setEnabled(true);
				m_ui->keep_image_ratio_radio->setChecked(true);
			}
			else {
				//Disable keep ratio radio button
				m_ui->keep_image_ratio_radio->setEnabled(false);
				m_ui->keep_image_ratio_radio->setChecked(false);
			}

		}

		case TaskKind::srcnn: {
			m_ui->srcnn_architecture_combobox->clear();

			//Iterate through all resources to find SRCNN's
			QDirIterator iter(":/SRCNN");
			while (iter.hasNext()) {
				QString cur_filename = iter.next(); //Get current path of resource
				//Leave only filename with extension
				cur_filename = cur_filename.section('/', -1, -1);
				//Leave only filename without th .pt extension
				cur_filename = cur_filename.left(cur_filename.size() - 3);

				//Check if it is SRCNN
				if (Algorithms::parse_srcnn(cur_filename, nullptr, nullptr, nullptr))
					m_ui->srcnn_architecture_combobox->addItem(cur_filename);
			}
			break;
		}

		case TaskKind::fsrcnn: {
			m_ui->fsrcnn_architecture_combobox->clear();

			//Iterate through all resources to find FSRCNN's
			QDirIterator iter(":/FSRCNN", QDirIterator::IteratorFlag::Subdirectories);
			while (iter.hasNext()) {
				QString cur_filename = iter.next(); //Get current path of resource
				//Leave only filename with extension
				cur_filename = cur_filename.section('/', -1, -1);
				//Leave only filename without th .pt extension
				cur_filename = cur_filename.left(cur_filename.size() - 3);

				//Check if it is FSRCNN
				if (Algorithms::parse_fsrcnn(cur_filename, nullptr, nullptr, nullptr))
					m_ui->fsrcnn_architecture_combobox->addItem(cur_filename);
			}
			break;
		}

		case TaskKind::convert_color_space: {
			//Just do nothing . _.
		}
	}

	//Toggle stacked widget
	m_ui->task_settings_widget->setCurrentIndex(index);
}

void ImageUpscalerQt::select_image_clicked() {
	QFileDialog dialog(this, "Open image", "/home",
		"All images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;\
		PNG image (*.png);;JPEG image (*.jpg *.jpeg);;JPEG2000 image (*.jp2 *.jpg2)\
		Bitmap image (*.bmp);;TIFF image (*.tiff *.tif);;Icon (*.ico)");
	dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);

	//If accepted
	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		this->image_filename = dialog.selectedFiles()[0];
		m_ui->image_path_label->setText(dialog.selectedFiles()[0]);

		//Get image spec and check image
		auto input = OIIO::ImageInput::open(image_filename.toStdString());
		if (!input) {
			QMessageBox::critical(this, "Failed to read image",
								  "Failed to read image. The file may be either damaged, not supported or corrupted");
			return;
		}
		image_spec = input->spec();

		//Update task creation menu
		task_kind_changed(m_ui->task_kind_combobox->currentIndex());
		update_list(); //Update list
	}
}

void ImageUpscalerQt::remove_task_clicked() {
	auto selected = m_ui->queue_list->selectionModel()->selectedIndexes();
	for (unsigned short i = 0; i < selected.size(); i++)
		task_queue.erase(task_queue.begin() + selected[i].row());

	update_list();
}

void ImageUpscalerQt::clear_tasks_clicked() {
	task_queue.clear();
	update_list();
}

void ImageUpscalerQt::move_task_up_clicked() {
	auto selected_list = m_ui->queue_list->selectionModel()->selectedIndexes();
	//Display error message if either selected more than 1 task or no one selected.
	if (selected_list.size() != 1) {
		QMessageBox::information(this, "Unable to move tasks",
								 "Unable to move tasks. You should select one task");
		return;
	}

	//Extract the integer
	int index = selected_list[0].row();

	//Just ignore if task is already on the top
	if (index == 0)
		return;

	//Swap
	std::swap(task_queue[index], task_queue[index - 1]);

	//Update and restore selection
	update_list();
	m_ui->queue_list->setCurrentRow(index - 1);
}

void ImageUpscalerQt::move_task_down_clicked() {
	auto selected_list = m_ui->queue_list->selectionModel()->selectedIndexes();
	//Display error message if either selected more than 1 task or no one selected.
	if (selected_list.size() != 1) {
		QMessageBox::information(this, "Unable to move tasks",
								 "Unable to move tasks. You should select one task");
		return;
	}

	//Extract the integer
	int index = selected_list[0].row();

	//Just ignore if task is already on the bottom
	if (index == task_queue.size() - 1)
		return;

	//Swap
	std::swap(task_queue[index], task_queue[index + 1]);

	//Update and restore selection
	update_list();
	m_ui->queue_list->setCurrentRow(index + 1);
}

void ImageUpscalerQt::resize_x_changed(int value) {
	//If keep ratio enabled
	if (m_ui->keep_image_ratio_radio->isChecked()) {
		//Calculate dependent y
		float ratio = (float)image_spec.height / image_spec.width;
		int y = ratio * value;
		//Set it to the spinbox
		m_ui->resize_y->blockSignals(true);
		m_ui->resize_y->setValue(y);
		m_ui->resize_y->blockSignals(false);
	}
}

void ImageUpscalerQt::resize_y_changed(int value) {
	//If keep ratio enabled
	if (m_ui->keep_image_ratio_radio->isChecked()) {
		//Calculate dependent x
		float ratio = (float)image_spec.width / image_spec.height;
		int x = ratio * value;
		//Set it to the spinbox
		m_ui->resize_x->blockSignals(true);
		m_ui->resize_x->setValue(x);
		m_ui->resize_x->blockSignals(false);
	}
}

void ImageUpscalerQt::keep_ratio_toggled(bool checked) {
	if (checked)
		resize_x_changed(m_ui->resize_x->value());
}

//BEGIN SRCNN page events
void ImageUpscalerQt::srcnn_architecture_changed(QString) {
	update_srcnn_info();
}

void ImageUpscalerQt::srcnn_block_size_changed(int) {
	update_srcnn_info();
}

void ImageUpscalerQt::srcnn_split_checked(int) {
	update_srcnn_info();
}

void ImageUpscalerQt::update_srcnn_info() {
	//Handle the block size
	if (!image_spec.undefined()) {
		//If current block size is greater than image size, do not split image into blocks
		if (m_ui->srcnn_block_size_spinbox->value() > end_width() ||
			m_ui->srcnn_block_size_spinbox->value() > end_height()) {
			m_ui->srcnn_block_split_check->setChecked(false);
			m_ui->srcnn_block_size_spinbox->setEnabled(false);
		}
		else {
			m_ui->srcnn_block_split_check->setChecked(true);
			m_ui->srcnn_block_size_spinbox->setEnabled(true);
		}
	}

	//Set maximum block size
	m_ui->srcnn_block_size_spinbox->setMaximum(std::min(end_width(), end_height()));

	//Gray out block size spinbox if the split checkbox is unchecked
	m_ui->srcnn_block_size_spinbox->setEnabled(m_ui->srcnn_block_split_check->isChecked());

	//Parse current architecture
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 4> channels;
	if (!Algorithms::parse_srcnn(m_ui->srcnn_architecture_combobox->currentText(),
		&kernels, nullptr, &channels)) {
		//If the current architecture is invalid, just skip it
		return;
	}
	std::array<int, 4> widths; //This arrays will be defined later
	std::array<int, 4> heights;

	//BEGIN Amount of operations
	//Display amount of operations
	if (image_spec.undefined()) { //If the image is not selected yet
		m_ui->srcnn_total_operations_label->setText("Total operations: no image");
	}
	else if (m_ui->srcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		int block_size = m_ui->srcnn_block_size_spinbox->value();

		//Compute amount of operations per block
		widths = {block_size, block_size, block_size, block_size};
		heights = {block_size, block_size, block_size, block_size};
		auto o_per_block = Algorithms::srcnn_operations_amount(kernels, channels, widths, heights);

		//Compute amount of the blocks
		int blocks_width = end_width() / block_size;
		if (blocks_width * block_size < end_width())
			blocks_width++;
		int blocks_height = end_height() / block_size;
		if (blocks_height * block_size < end_height())
			blocks_height++;
		long long blocks_amount = blocks_height * blocks_width;

		//Set label about it
		m_ui->srcnn_total_operations_label->setText(
			QString("Total operations: %1").arg(
				Algorithms::big_number_to_string(o_per_block * blocks_amount * image_spec.nchannels, ' ')
			)
		);
	}
	else { //If we have not to split the image into blocks
		widths = {end_width(), end_width(), end_width(), end_width()};
		heights = {end_height(), end_height(), end_height(), end_height()};
		auto operations = Algorithms::srcnn_operations_amount(kernels, channels, widths, heights);
		operations *= image_spec.nchannels;

		//Set label about it
		m_ui->srcnn_total_operations_label->setText(
			QString("Total operations: %1").arg(
				Algorithms::big_number_to_string(operations, ' ')
			)
		);
	}
	//END Amount of operations

	//BEGIN Memory consumption
	unsigned int width_per_iter;
	unsigned int height_per_iter;
	//Define this 2 variables above
	if (m_ui->srcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		width_per_iter = m_ui->srcnn_block_size_spinbox->value();
		height_per_iter = m_ui->srcnn_block_size_spinbox->value();
	}
	else if (!image_spec.undefined()) { //If we have not to split the image into blocks
		width_per_iter = end_width();
		height_per_iter = end_height();
	}

	//Compute memory consumption
	//Iterate throught every convolutional layer to find the point with maximum memory consumption
	//Considering the channels amount, width and height (in short, tensor size)
	// |  1  | --- | 128 | --- |  64 | --- |  1  |
	//                      ^
	//    max consumption point = size 1 + size 2
	unsigned long long max_point = 0;
	for (unsigned char i = 0; i < 3; i++) {
		unsigned long long cur_max_point = widths[i] * heights[i] * channels[i] +
										   widths[i + 1] * heights[i + 1] * channels[i + 1];

		if (cur_max_point > max_point)
			max_point = cur_max_point;
	}

	//Now, we have amount of numbers, but they are in float type, so multiply it to convert it to bytes
	max_point *= sizeof(float);

	//Display it
	m_ui->srcnn_memory_consumption_label->setText("Memory consumption: " +
		Algorithms::bytes_amount_to_string(max_point));
	//END Memory consumption
}

//END SRCNN page events

//BEGIN FSRCNN page events
void ImageUpscalerQt::fsrcnn_architecture_changed(QString) {
	update_fsrcnn_info();
}

void ImageUpscalerQt::fsrcnn_block_size_changed(int) {
	update_fsrcnn_info();
}

void ImageUpscalerQt::fsrcnn_split_checked(int) {
	update_fsrcnn_info();
}

void ImageUpscalerQt::update_fsrcnn_info() {
	//Handle the block size
	if (!image_spec.undefined()) {
		//If current block size is greater than image size, do not split image into blocks
		if (m_ui->fsrcnn_block_size_spinbox->value() > end_width() ||
			m_ui->fsrcnn_block_size_spinbox->value() > end_height()) {
			m_ui->fsrcnn_block_split_check->setChecked(false);
			m_ui->fsrcnn_block_size_spinbox->setEnabled(false);
		}
		else {
			m_ui->fsrcnn_block_split_check->setChecked(true);
			m_ui->fsrcnn_block_size_spinbox->setEnabled(true);
		}
	}

	//Set maximum block size
	m_ui->fsrcnn_block_size_spinbox->setMaximum(std::min(end_width(), end_height()));

	//Gray out block size spinbox if the split checkbox is unchecked
	m_ui->fsrcnn_block_size_spinbox->setEnabled(m_ui->fsrcnn_block_split_check->isChecked());

	//Parse current architecture
	std::array<unsigned short, 4> kernels;
	std::array<unsigned short, 5> channels;
	if (!Algorithms::parse_fsrcnn(m_ui->fsrcnn_architecture_combobox->currentText(),
		&kernels, nullptr, &channels)) {
		//If the current architecture is invalid, just skip it
		return;
	}
	std::array<int, 5> widths; //This arrays will be defined later
	std::array<int, 5> heights;

	//BEGIN Amount of operations
	//Display amount of operations
	if (image_spec.undefined()) { //If the image is not selected yet
		m_ui->fsrcnn_total_operations_label->setText("Total operations: no image");
	}
	else if (m_ui->fsrcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		int block_size = m_ui->fsrcnn_block_size_spinbox->value();

		//Compute amount of operations per block
		widths = {block_size, block_size, block_size, block_size * 2};
		heights = {block_size, block_size, block_size, block_size * 2};
		auto o_per_block = Algorithms::fsrcnn_operations_amount(kernels, channels, widths, heights);

		//Compute amount of the blocks
		int blocks_width = end_width() / block_size;
		if (blocks_width * block_size < end_width())
			blocks_width++;
		int blocks_height = end_height() / block_size;
		if (blocks_height * block_size < end_height())
			blocks_height++;
		long long blocks_amount = blocks_height * blocks_width;

		//Set label about it
		m_ui->fsrcnn_total_operations_label->setText(
			QString("Total perations: %1").arg(
				Algorithms::big_number_to_string(o_per_block * blocks_amount * image_spec.nchannels, ' ')
			)
		);
	}
	else { //If we have not to split the image into blocks
		widths = {end_width(), end_width(), end_width(), end_width() * 2};
		heights = {end_height(), end_height(), end_height(), end_height() * 2};
		auto operations = Algorithms::fsrcnn_operations_amount(kernels, channels, widths, heights);
		operations *= image_spec.nchannels;

		//Set label about it
		m_ui->fsrcnn_total_operations_label->setText(
			QString("Total perations: %1").arg(
				Algorithms::big_number_to_string(operations, ' ')
			)
		);
	}
	//END Amount of operations

	//BEGIN Memory consumption
	int width_per_iter;
	int height_per_iter;
	//Define this 2 variables above
	if (m_ui->fsrcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		width_per_iter = m_ui->fsrcnn_block_size_spinbox->value();
		height_per_iter = m_ui->fsrcnn_block_size_spinbox->value();
	}
	else if (!image_spec.undefined()) { //If we have not to split the image into blocks
		width_per_iter = end_width();
		height_per_iter = end_height();
	}

	//Compute memory consumption
	//Iterate throught every convolutional layer to find the point with maximum memory consumption
	//Considering the channels amount, width and height (in short, tensor size)
	// |  1  | --- | 128 | --- |  64 | --- |  1  |
	//                      ^
	//    max consumption point = size 1 + size 2
	unsigned long long max_point = 0;
	for (unsigned char i = 0; i < 4; i++) {
		unsigned long long cur_max_point = widths[i] * heights[i] * channels[i] +
									   widths[i + 1] * heights[i + 1] * channels[i + 1];

		if (cur_max_point > max_point)
			max_point = cur_max_point;
	}

	//Now, we have amount of numbers, but they are in float type, so multiply it to convert it to bytes
	max_point *= sizeof(float);

	//Display it
	m_ui->fsrcnn_memory_consumption_label->setText("Memory consumption: " +
		Algorithms::bytes_amount_to_string(max_point));
	//END Memory consumption
}
//END FSRCNN page events

void ImageUpscalerQt::start_tasks_clicked() {
	//Special cases
	if (image_spec.undefined()) {
		QMessageBox::warning(this, "No image", "No image selected, so you can\'t start the tasks.");
		return;
	}
	if (task_queue.empty()) {
		QMessageBox::warning(this, "No tasks", "Tasks queue is empty, so you can\'t start them.");
		return;
	}

	TasksWaitingDialog* dialog = new TasksWaitingDialog();
	dialog->open();
	dialog->do_tasks(task_queue, image_filename);
}

void ImageUpscalerQt::about_clicked() {
	QMessageBox::about(this, "About the ImageUpscalerQt",
					   QString("ImageUpscalerQt version ") + VERSION + "\n\nImageUpscalerQt - program for image upscaling using the neural networks, but it also have other auxiliary functions.\n\nMade by Artem Kliminskyi in Ukraine, Zhytomyr");
}
//END Slots

void ImageUpscalerQt::update_list() {
	//Update list
	m_ui->queue_list->clear();
	for (unsigned short i = 0; i < task_queue.size(); i++)
		m_ui->queue_list->addItem(task_queue[i]->to_string(i));

	//Status label
	if (task_queue.size() == 0) {
		m_ui->status_label->setText("No tasks in the queue");
		return;
	}
	if (image_spec.undefined()) {
		m_ui->status_label->setText("Image is not selected");
		return;
	}

	//1920x1080 -> 3840x2160
	QString status = QString("%1x%2 -> %3x%4").arg(QString::number(image_spec.width),
												   QString::number(image_spec.height),
												   QString::number(end_width()),
												   QString::number(end_height()));

	m_ui->status_label->setText(status);
}

int ImageUpscalerQt::end_width() {
	//Calculate "before" and "after" resolution
	const int before_width = image_spec.width;
	int after_width = before_width;
	for (unsigned short i = 0; i < task_queue.size(); i++) {
		switch (task_queue[i]->task_kind) {
			case TaskKind::fsrcnn: {
				after_width *= 2; //FSRCNN increases resolution in 2 times
				break;
			}
			case TaskKind::resize: {
				after_width = ((TaskResize*)task_queue[i])->x_size;
				break;
			}
			default:
				break;
		}
	}

	return after_width;
}

int ImageUpscalerQt::end_height() {
	//Calculate "before" and "after" resolution
	const int before_height = image_spec.height;
	int after_height = before_height;
	for (unsigned short i = 0; i < task_queue.size(); i++) {
		switch (task_queue[i]->task_kind) {
			case TaskKind::fsrcnn: {
				after_height *= 2; //FSRCNN increases resolution in 2 times
				break;
			}
			case TaskKind::resize: {
				after_height = ((TaskResize*)task_queue[i])->x_size;
				break;
			}
			default:
				break;
		}
	}

	return after_height;
}
