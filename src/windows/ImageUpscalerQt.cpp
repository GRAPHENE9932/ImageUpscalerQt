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

#define VERSION "1.2"

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
	connect(m_ui->fsrcnn_architecture_combobox, SIGNAL(currentTextChanged(QString)), this, SLOT(fsrcnn_architecture_changed(QString)));
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
			std::array<unsigned short, 2> channels;
			Algorithms::parse_srcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskSRCNN* cur_task = new TaskSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::fsrcnn: {
			//Get FSRCNN name
			QString name = m_ui->fsrcnn_architecture_combobox->currentText();
			//Parse FSRCNN
			std::array<unsigned short, 4> kernels;
			std::array<unsigned short, 4> paddings;
			std::array<unsigned short, 3> channels;
			Algorithms::parse_fsrcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskFSRCNN* cur_task = new TaskFSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);
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
			//Get folder that contains .pt files
			QDir nn_storage_path = QDir::currentPath() + "/SRCNN/";

			//Check if this folder exists
			if (!nn_storage_path.exists()) {
				QMessageBox::critical(this, "Missing files", "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList files = nn_storage_path.entryList(QDir::Files);
			QStringList names;
			for (int i = 0; i < files.size(); i++) {
				QString cur_name = QFileInfo(files[i]).baseName();
				QString cur_extension = QFileInfo(files[i]).completeSuffix();
				if (Algorithms::parse_srcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == "pt")
					names.push_back(cur_name);
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}
			//Insert they into combobox
			m_ui->srcnn_architecture_combobox->clear();
			m_ui->srcnn_architecture_combobox->addItems(names);

			break;
		}

		case TaskKind::fsrcnn: {
			//Get folder that contains .pt files
			QDir nn_storage_path = QDir::currentPath() + "/FSRCNN/";

			//Check if this folder exists
			if (!nn_storage_path.exists()) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList files = nn_storage_path.entryList(QDir::Files);
			QStringList names;
			for (int i = 0; i < files.size(); i++) {
				QString cur_name = QFileInfo(files[i]).baseName();
				QString cur_extension = QFileInfo(files[i]).completeSuffix();
				if (Algorithms::parse_fsrcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == "pt")
					names.push_back(cur_name);
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Insert they into combobox
			m_ui->fsrcnn_architecture_combobox->clear();
			m_ui->fsrcnn_architecture_combobox->addItems(names);

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

void ImageUpscalerQt::srcnn_architecture_changed(QString text) {
	//Parse current architecture first
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 2> channels;
	if (!Algorithms::parse_srcnn(text, &kernels, nullptr, &channels))
		return;

	//Calculate amount of operations per block
	long long o_per_block = Algorithms::srcnn_operations_amount(kernels, channels);
	//Set label of it
	m_ui->srcnn_operations_per_block_label->setText(
		QString("Operations per block: %1").arg(Algorithms::big_number_to_string(o_per_block, ' '))
	);

	//Calculate amount of blocks and total amount of operations
	if (!image_spec.undefined()) {
		int blocks_width = image_spec.width / 192;
		if (blocks_width * 192 < image_spec.width)
			blocks_width++;
		int blocks_height = image_spec.height / 192;
		if (blocks_height * 192 < image_spec.height)
			blocks_height++;
		long long blocks = blocks_height * blocks_width * image_spec.nchannels;

		long long o_total = o_per_block * blocks;

		//Set labels of it
		m_ui->srcnn_blocks_label->setText(
			QString("Amount of blocks: %1").arg(Algorithms::big_number_to_string(blocks, ' '))
		);
		m_ui->srcnn_total_operations_label->setText(
			QString("Total operations: %1").arg(Algorithms::big_number_to_string(o_total, ' '))
		);
	}
	else { //If image not selected
		m_ui->srcnn_blocks_label->setText("Amount of blocks: no image");
		m_ui->srcnn_total_operations_label->setText("Total operations: no image");
	}
}

void ImageUpscalerQt::fsrcnn_architecture_changed(QString text) {
	//Parse current architecture first
	std::array<unsigned short, 4> kernels;
	std::array<unsigned short, 3> channels;
	if (!Algorithms::parse_fsrcnn(text, &kernels, nullptr, &channels))
		return;

	//Calculate amount of operations per block
	long long o_per_block = Algorithms::fsrcnn_operations_amount(kernels, channels);
	//Set label of it
	m_ui->fsrcnn_operations_per_block_label->setText(
		QString("Operations per block: %1").arg(Algorithms::big_number_to_string(o_per_block, ' '))
	);

	//Calculate amount of blocks and total amount of operations
	if (!image_spec.undefined()) {
		int blocks_width = image_spec.width / 64;
		if (blocks_width * 64 < image_spec.width)
			blocks_width++;
		int blocks_height = image_spec.height / 64;
		if (blocks_height * 64 < image_spec.height)
			blocks_height++;
		long long blocks = blocks_height * blocks_width * image_spec.nchannels;

		long long o_total = o_per_block * blocks;

		//Set labels of it
		m_ui->fsrcnn_blocks_label->setText(
			QString("Amount of blocks: %1").arg(Algorithms::big_number_to_string(blocks, ' '))
		);
		m_ui->fsrcnn_total_operations_label->setText(
			QString("Total operations: %1").arg(Algorithms::big_number_to_string(o_total, ' '))
		);
	}
	else { //If image not selected
		m_ui->fsrcnn_blocks_label->setText("Amount of blocks: no image");
		m_ui->fsrcnn_total_operations_label->setText("Total operations: no image");
	}
}

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
		m_ui->status_label->setText("No tasks in queue");
		return;
	}
	if (image_spec.undefined()) {
		m_ui->status_label->setText("Image is not selected");
		return;
	}
	//Calculate "before" and "after" resolution
	int before_width = image_spec.width;
	int before_height = image_spec.height;
	int after_width = before_width;
	int after_height = before_height;
	for (unsigned short i = 0; i < task_queue.size(); i++) {
		switch (task_queue[i]->task_kind) {
			case TaskKind::fsrcnn: {
				after_width *= 2; //FSRCNN increases resolution in 2 times
				after_height *= 2;
				break;
			}
			case TaskKind::resize: {
				after_width = ((TaskResize*)task_queue[i])->x_size;
				after_height = ((TaskResize*)task_queue[i])->y_size;
				break;
			}
			default:
				break;
		}
	}
	//1920x1080 -> 3840x2160
	QString status = QString("%1x%2 -> %3x%4").arg(QString::number(before_width),
												   QString::number(before_height),
												   QString::number(after_width),
												   QString::number(after_height));

	m_ui->status_label->setText(status);
}
