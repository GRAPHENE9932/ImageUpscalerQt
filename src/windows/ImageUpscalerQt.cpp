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

#define VERSION "1.0"

///Point to make the memory consumption label orange
constexpr unsigned long long WARNING_MEMORY = 1ull * 1024ull * 1024ull * 1024ull; //1.0 GiB
///Point to make the memory consumption label red
constexpr unsigned long long CRITICAL_MEMORY = 2ull * 1024ull * 1024ull * 1024ull; //2.0 GiB

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) : QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt) {
    m_ui->setupUi(this);

	//Set window icon
	setWindowIcon(QIcon(":Icon.png"));

	//Add entries to the resize_interpolation_combobox
	for (QString cur_str : INTERPOLATION_NAMES)
		m_ui->resize_interpolation_combobox->addItem(cur_str);

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

void ImageUpscalerQt::prepare_task_resize() {
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

void ImageUpscalerQt::prepare_task_srcnn() {
	m_ui->srcnn_architecture_combobox->clear();

	//Iterate through all resources to find SRCNN's
	QDirIterator iter(":/SRCNN");
	QStringList list;
	while (iter.hasNext()) {
		//Get current path of resource and leave only filename with extension
		list.push_back(iter.next().section('/', -1, -1));
	}

	Algorithms::numerical_sort(list); //Sort strings numerically

	for (QString cur_filename : list) {
		//Leave only filename without the .pt extension
		cur_filename = cur_filename.left(cur_filename.size() - 3);

		//Check if it is SRCNN
		if (Algorithms::parse_srcnn(cur_filename, nullptr, nullptr, nullptr))
			m_ui->srcnn_architecture_combobox->addItem(cur_filename);
	}
}

void ImageUpscalerQt::prepare_task_fsrcnn() {
	m_ui->fsrcnn_architecture_combobox->clear();

	//Iterate through all resources to find SRCNN's
	QDirIterator iter(":/FSRCNN");
	QStringList list;
	while (iter.hasNext()) {
		//Get current path of resource and leave only filename with extension
		list.push_back(iter.next().section('/', -1, -1));
	}

	Algorithms::numerical_sort(list); //Sort strings numerically

	for (QString cur_filename : list) {
		//Leave only filename without the .pt extension
		cur_filename = cur_filename.left(cur_filename.size() - 3);

		//Check if it is FSRCNN
		if (Algorithms::parse_fsrcnn(cur_filename, nullptr, nullptr, nullptr))
			m_ui->fsrcnn_architecture_combobox->addItem(cur_filename);
	}
}

void ImageUpscalerQt::prepare_task_convert_color_space() {
	//Just do nothing . _.
}

void ImageUpscalerQt::task_kind_changed(int index) {
	//Set up the right part of the window
	switch ((TaskKind)index) {
		case TaskKind::resize: {
			prepare_task_resize();
			break;
		}
		case TaskKind::srcnn: {
			prepare_task_srcnn();
			break;
		}
		case TaskKind::fsrcnn: {
			prepare_task_fsrcnn();
			break;
		}
		case TaskKind::convert_color_space: {
			prepare_task_convert_color_space();
			break;
		}
	}

	//Toggle stacked widget
	m_ui->task_settings_widget->setCurrentIndex(index);
}

Task* ImageUpscalerQt::init_task_resize() {
	return new TaskResize(
		m_ui->resize_x->value(),
		m_ui->resize_y->value(),
		(Interpolation)m_ui->resize_interpolation_combobox->currentIndex()
	);
}

Task* ImageUpscalerQt::init_task_srcnn() {
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
	return new TaskSRCNN(kernels, paddings, channels, block_size);
}

Task* ImageUpscalerQt::init_task_fsrcnn() {
	//Get FSRCNN name
	QString name = m_ui->fsrcnn_architecture_combobox->currentText();
	//Parse FSRCNN
	std::vector<unsigned short> kernels;
	std::vector<unsigned short> paddings;
	std::vector<unsigned short> channels;
	Algorithms::parse_fsrcnn(name, &kernels, &paddings, &channels);

	//Get block size
	unsigned int block_size;
	if (m_ui->fsrcnn_block_split_check->isChecked())
		block_size = m_ui->fsrcnn_block_size_spinbox->value();
	else
		block_size = 0; //0 means do not split image

	//Create task
	return new TaskFSRCNN(kernels, paddings, channels, block_size);
}

Task* ImageUpscalerQt::init_task_convert_color_space() {
	return new TaskConvertColorSpace(
		(ColorSpaceConversion)m_ui->color_space_combobox->currentIndex()
	);
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
	}

	//Set maximum block size
	m_ui->srcnn_block_size_spinbox->setMaximum(std::clamp(std::min(end_width(), end_height()), 32, INT_MAX));

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

	std::array<int, 4> widths; //Define this arrays
	std::array<int, 4> heights;
	if (m_ui->srcnn_block_split_check->isChecked()) { //If split by blocks
		int block_size = m_ui->srcnn_block_size_spinbox->value();

		widths = {block_size, block_size, block_size, block_size};
		heights = {block_size, block_size, block_size, block_size};
	}
	else if (!image_spec.undefined()) { //If not
		widths = {end_width(), end_width(), end_width(), end_width()};
		heights = {end_height(), end_height(), end_height(), end_height()};
	}

	//BEGIN Amount of operations
	//Display amount of operations
	if (image_spec.undefined()) { //If the image is not selected yet
		m_ui->srcnn_total_operations_label->setText("Total operations: no image");
	}
	else if (m_ui->srcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		//Compute amount of operations per block
		auto o_per_block = Algorithms::srcnn_operations_amount(kernels, channels, widths, heights);

		//Compute amount of the blocks
		int blocks_width = end_width() / widths[0];
		if (blocks_width * widths[0] < end_width())
			blocks_width++;
		int blocks_height = end_height() / heights[0];
		if (blocks_height * heights[0] < end_height())
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
	if (!image_spec.undefined() || m_ui->srcnn_block_split_check->isChecked()) {
		//Compute memory consumption
		auto mem_consumption = Algorithms::measure_cnn_memory_consumption(channels, widths, heights);

		//Display it
		m_ui->srcnn_memory_consumption_label->setText("Approx. memory consumption: " +
			Algorithms::bytes_amount_to_string(mem_consumption));

		//Set color of the label
		if (mem_consumption > CRITICAL_MEMORY) {
			m_ui->srcnn_memory_consumption_label->setStyleSheet("font-weight:bold;color:red"); //Red
		}
		else if (mem_consumption > WARNING_MEMORY) {
			m_ui->srcnn_memory_consumption_label->setStyleSheet("font-weight:bold;color:orange"); //Orange
		}
		else {
			m_ui->srcnn_memory_consumption_label->setStyleSheet(styleSheet()); //Default
		}
	}
	else {
		m_ui->srcnn_memory_consumption_label->setText("Approx. memory consumption: no image");
		m_ui->srcnn_memory_consumption_label->setStyleSheet(styleSheet());
	}
	//END Memory consumption
}

//BEGIN Slots
void ImageUpscalerQt::add_task_clicked() {
	//Add task
	switch ((TaskKind)m_ui->task_kind_combobox->currentIndex()) {
		case TaskKind::resize: {
			task_queue.push_back(init_task_resize());
			break;
		}
		case TaskKind::srcnn: {
			task_queue.push_back(init_task_srcnn());
			break;
		}
		case TaskKind::fsrcnn: {
			task_queue.push_back(init_task_fsrcnn());

			//Update task info because the new FSRCNN will change the end size of the image
			update_fsrcnn_info();
			break;
		}
		case TaskKind::convert_color_space: {
			task_queue.push_back(init_task_convert_color_space());
		}
	}

	update_list();
}

void ImageUpscalerQt::select_image_clicked() {
	QFileDialog dialog(this, "Open image", "/home",
		"All images(*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;\
		PNG image(*.png);;JPEG image(*.jpg *.jpeg);;JPEG2000 image(*.jp2 *.jpg2);;\
		Bitmap image(*.bmp);;TIFF image (*.tiff *.tif);;Icon(*.ico)");
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
	}

	//Set maximum block size
	m_ui->fsrcnn_block_size_spinbox->setMaximum(std::clamp(std::min(end_width(), end_height()), 32, INT_MAX));

	//Gray out block size spinbox if the split checkbox is unchecked
	m_ui->fsrcnn_block_size_spinbox->setEnabled(m_ui->fsrcnn_block_split_check->isChecked());

	//Parse current architecture
	std::vector<unsigned short> kernels;
	std::vector<unsigned short> channels;
	if (!Algorithms::parse_fsrcnn(m_ui->fsrcnn_architecture_combobox->currentText(),
		&kernels, nullptr, &channels)) {
		//If the current architecture is invalid, just skip it
		return;
	}

	std::vector<int> widths; //Define this arrays
	std::vector<int> heights;
	if (m_ui->fsrcnn_block_split_check->isChecked()) { //If split by blocks
		int block_size = m_ui->fsrcnn_block_size_spinbox->value();

		widths = std::vector<int>(kernels.size() + 1, block_size);
		widths[widths.size() - 1] *= 3; //Last element have to be with size of the result image
		heights = std::vector<int>(kernels.size() + 1, block_size);
		heights[heights.size() - 1] *= 3; //Last element have to be with size of the result image
	}
	else if (!image_spec.undefined()) { //If not
		widths = std::vector<int>(kernels.size() + 1, end_width());
		widths[widths.size() - 1] *= 3; //Last element have to be with size of the result image
		heights = std::vector<int>(kernels.size() + 1, end_width());
		heights[heights.size() - 1] *= 3; //Last element have to be with size of the result image
	}

	//BEGIN Amount of operations
	//Display amount of operations
	if (image_spec.undefined()) { //If the image is not selected yet
		m_ui->fsrcnn_total_operations_label->setText("Total operations: no image");
	}
	else if (m_ui->fsrcnn_block_split_check->isChecked()) { //If we have to split the image into blocks
		auto o_per_block = Algorithms::fsrcnn_operations_amount(kernels, channels, widths, heights);

		//Compute amount of the blocks
		int blocks_width = end_width() / widths[0];
		if (blocks_width * widths[0] < end_width())
			blocks_width++;
		int blocks_height = end_height() / heights[0];
		if (blocks_height * heights[0] < end_height())
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
	if (!image_spec.undefined() || m_ui->fsrcnn_block_split_check->isChecked()) {
		//Compute memory consumption
		auto mem_consumption = Algorithms::measure_cnn_memory_consumption(channels, widths, heights);

		//Display it
		m_ui->fsrcnn_memory_consumption_label->setText("Approx. memory consumption: " +
			Algorithms::bytes_amount_to_string(mem_consumption));

		//Set color of the label
		if (mem_consumption > CRITICAL_MEMORY) {
			m_ui->fsrcnn_memory_consumption_label->setStyleSheet("font-weight:bold;color:red"); //Red
		}
		else if (mem_consumption > WARNING_MEMORY) {
			m_ui->fsrcnn_memory_consumption_label->setStyleSheet("font-weight:bold;color:orange"); //Orange
		}
		else {
			m_ui->fsrcnn_memory_consumption_label->setStyleSheet(styleSheet()); //Default
		}
	}
	else {
		m_ui->fsrcnn_memory_consumption_label->setText("Approx. memory consumption: no image");
		m_ui->fsrcnn_memory_consumption_label->setStyleSheet(styleSheet());
	}
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
	dialog->setModal(true);
	dialog->show();
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

	//1920x1080 -> 5760x3240
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
				after_width *= 3; //FSRCNN increases resolution in 3 times
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
				after_height *= 3; //FSRCNN increases resolution in 3 times
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
