/*
 * ImageUpscalerQt - main window
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <thread>

#include <QFileDialog>
#include <QMessageBox>
#include <OpenImageIO/imageio.h>

#include "ImageUpscalerQt.h"
#include "ui_ImageUpscalerQt.h"
#include "TaskCreationDialog.h"
#include "../functions/func.h"

constexpr const char* VERSION = "1.2";
constexpr const char* FILE_FILTER = "All images(*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;"
									"PNG image(*.png);;JPEG image(*.jpg *.jpeg);;JPEG2000 image(*.jp2 *.jpg2);;"
									"Bitmap image(*.bmp);;TIFF image (*.tiff *.tif);;Icon(*.ico)";
constexpr QSize SIZE_NULL = QSize(0, 0);

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) : QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt) {
    m_ui->setupUi(this);

	// Set window icon.
	setWindowIcon(QIcon(":icon.png"));
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

QSize ImageUpscalerQt::max_image_size() {
	QSize max_size = SIZE_NULL;

	for (int i = 0; i < files.size(); i++) {
		auto img_input = OIIO::ImageInput::open(files[i].toStdString());

		if (!img_input)
			continue;

		auto spec = img_input->spec();
		if (spec.width * spec.height > max_size.width() * max_size.height())
			max_size = QSize(spec.width, spec.height);
	}

	return max_size;
}

void ImageUpscalerQt::update_file_list() {
	// We have to save current (selected) item.
	auto cur_row = m_ui->file_list_widget->currentRow();

	// Set items.
	m_ui->file_list_widget->clear();
	m_ui->file_list_widget->addItems(func::shorten_file_paths(files));

	// Set tooltips.
	for (int i = 0; i < files.size(); i++)
		m_ui->file_list_widget->item(i)->setToolTip(files[i]);

	m_ui->file_list_widget->setCurrentRow(cur_row);

	auto prev_thread = std::thread(&ImageUpscalerQt::update_previews, this);
	prev_thread.detach();
}

void ImageUpscalerQt::update_previews() {
	for (int i = 0; i < files.size(); i++) {
		QIcon icon(files[i]);

		// Check if file is corrupt.
		if (icon.pixmap(QSize(1, 1)).isNull())
			icon = QIcon(":unknown.svg");

		m_ui->file_list_widget->item(i)->setIcon(icon);
	}
}

void ImageUpscalerQt::swap_files(int index_1, int index_2) {
	assert(index_1 != index_2);
	assert(index_1 < files.size());
	assert(index_2 < files.size());

	// It is important for us that index_2 is greater than index_1.
	if (index_1 > index_2)
		std::swap(index_1, index_2);

	// Swap items in the list that in the GUI.
	QListWidgetItem* item_1 = m_ui->file_list_widget->takeItem(index_1);
	QListWidgetItem* item_2 = m_ui->file_list_widget->takeItem(index_2 - 1);
	m_ui->file_list_widget->insertItem(index_1, item_2);
	m_ui->file_list_widget->insertItem(index_2, item_1);

	// Swap items in the file list.
	std::swap(files[index_1], files[index_2]);
}

void ImageUpscalerQt::update_file_buttons() {
	auto cur_row = m_ui->file_list_widget->currentRow();
	auto cur_size = files.size();

	m_ui->file_up_button->setEnabled(cur_size > 1 && cur_row > 0);
	m_ui->file_down_button->setEnabled(cur_size > 1 && cur_row != -1 && cur_row < cur_size - 1);
	m_ui->file_remove_button->setEnabled(cur_row != -1);
	m_ui->file_clear_button->setEnabled(cur_size > 0);
}

// BEGIN Slots

void ImageUpscalerQt::add_files_clicked() {
	// Create a QFileDialog
	QFileDialog dialog(this, tr("Add images and folders"), QString(), FILE_FILTER);
	dialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
	if (dialog.exec()) {
		files += dialog.selectedFiles();

		int duplicates = files.removeDuplicates();

		update_file_list();

		// Warn user about removed duplicates.
		if (duplicates > 0) {
			QString message = QString::number(duplicates) + ' ' + (duplicates == 1 ?
				tr("duplicate was removed from the list.") :
				tr("duplicates were removed from the list."));

			QMessageBox::information(this, tr("Duplicates"), message, QMessageBox::StandardButton::Ok);
		}
	}

	update_file_buttons();
}

void ImageUpscalerQt::move_file_up_clicked() {
	auto cur_row = m_ui->file_list_widget->currentRow();
	if (cur_row == -1 || cur_row == 0 || files.size() < 2)
		return;

	swap_files(cur_row - 1, cur_row);
	m_ui->file_list_widget->setCurrentRow(cur_row - 1);

	update_file_buttons();
}

void ImageUpscalerQt::move_file_down_clicked() {
	auto cur_row = m_ui->file_list_widget->currentRow();
	if (cur_row == -1 || cur_row >= files.size() - 1 || files.size() < 2)
		return;

	swap_files(cur_row, cur_row + 1);
	m_ui->file_list_widget->setCurrentRow(cur_row + 1);

	update_file_buttons();
}

void ImageUpscalerQt::remove_file_clicked() {
	auto cur_row = m_ui->file_list_widget->currentRow();
	if (cur_row == -1 || cur_row > files.size() - 1)
		return;

	// Remove item from GUI.
	delete m_ui->file_list_widget->takeItem(cur_row);

	// Remove item from the list.
	files.removeAt(cur_row);

	update_file_buttons();
}

void ImageUpscalerQt::clear_files_clicked() {
	m_ui->file_list_widget->clear();
	files.clear();

	update_file_buttons();
}

void ImageUpscalerQt::file_selection_changed(int) {
	update_file_buttons();
}

void ImageUpscalerQt::swap_tasks(int index_1, int index_2) {
	assert(index_1 != index_2);
	assert(index_1 < tasks.size());
	assert(index_2 < tasks.size());

	// It is important for us that index_2 is greater than index_1.
	if (index_1 > index_2)
		std::swap(index_1, index_2);

	// Swap items in the list that in the GUI.
	QListWidgetItem* item_1 = m_ui->task_list_widget->takeItem(index_1);
	QListWidgetItem* item_2 = m_ui->task_list_widget->takeItem(index_2 - 1);
	m_ui->task_list_widget->insertItem(index_1, item_2);
	m_ui->task_list_widget->insertItem(index_2, item_1);

	// Swap items in the file list.
	std::swap(tasks[index_1], tasks[index_2]);
}

void ImageUpscalerQt::update_task_buttons() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	auto cur_size = tasks.size();

	m_ui->task_up_button->setEnabled(cur_size > 1 && cur_row > 0);
	m_ui->task_down_button->setEnabled(cur_size > 1 && cur_row != -1 && cur_row < cur_size - 1);
	m_ui->task_remove_button->setEnabled(cur_row != -1);
	m_ui->task_clear_button->setEnabled(cur_size > 0);
}

void ImageUpscalerQt::add_task_clicked() {
	TaskCreationDialog dialog(max_image_size());
	if (dialog.exec()) {
		const auto task_desc = dialog.get_task_desc();
		tasks.push_back(task_desc);

		m_ui->task_list_widget->addItem(task_desc->to_string());
	}
}

void ImageUpscalerQt::move_task_up_clicked() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	if (cur_row == -1 || cur_row == 0 || tasks.size() < 2)
		return;

	swap_tasks(cur_row - 1, cur_row);
	m_ui->task_list_widget->setCurrentRow(cur_row - 1);

	update_task_buttons();
}

void ImageUpscalerQt::move_task_down_clicked() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	if (cur_row == -1 || cur_row >= tasks.size() - 1 || tasks.size() < 2)
		return;

	swap_tasks(cur_row, cur_row + 1);
	m_ui->task_list_widget->setCurrentRow(cur_row + 1);

	update_task_buttons();
}

void ImageUpscalerQt::remove_task_clicked() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	if (cur_row == -1 || cur_row > tasks.size() - 1)
		return;

	// Remove item from GUI.
	delete m_ui->task_list_widget->takeItem(cur_row);

	// Remove item from the list.
	tasks.erase(tasks.begin() + cur_row);

	update_task_buttons();
}

void ImageUpscalerQt::clear_tasks_clicked() {
	m_ui->task_list_widget->clear();
	tasks.clear();

	update_task_buttons();
}

void ImageUpscalerQt::task_selection_changed(int index) {
	update_task_buttons();
}

void ImageUpscalerQt::start_tasks_clicked() {

}

void ImageUpscalerQt::about_clicked() {

}

// END Slots
