/*
 * ImageUpscalerQt - main window
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <thread>

#include <QFileDialog>
#include <QMessageBox>
#include <OpenImageIO/imageio.h>

#include "ImageUpscalerQt.hpp"
#include "ui_ImageUpscalerQt.h"
#include "TaskCreationDialog.hpp"
#include "TasksWaitingDialog.hpp"
#include "../functions/func.hpp"

constexpr const char* VERSION = "2.0";
constexpr const char* FILE_FILTER = "All images(*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;"
									"PNG image(*.png);;JPEG image(*.jpg *.jpeg);;"
									"JPEG2000 image(*.jp2 *.jpg2);;"
									"Bitmap image(*.bmp);;TIFF image (*.tiff *.tif);;Icon(*.ico)";
constexpr const char* ABOUT_TEXT = "ImageUpscalerQt is a program for image upscaling "
								   "using neural networks, but it also have other auxiliary functions.\n\n"
								   "Made by Artem Kliminskyi in Ukraine, Zhytomyr.";
constexpr QSize SIZE_NULL = QSize(0, 0);

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) : QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt) {
    m_ui->setupUi(this);

	// Set window icon.
	setWindowIcon(QIcon(":icon.png"));

	// Make the columns of the file list table be equal.
	m_ui->file_list_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
	m_ui->file_list_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);

	// Update info text.
	update_info_text();
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

QSize ImageUpscalerQt::max_image_size() {
	QSize max_size = SIZE_NULL;

	for (int i = 0; i < files.size(); i++) {
		auto img_input = OIIO::ImageInput::open(files[i].first.toStdString());

		if (!img_input)
			continue;

		auto spec = img_input->spec();
		if (spec.width * spec.height > max_size.width() * max_size.height())
			max_size = QSize(spec.width, spec.height);
	}

	return max_size;
}

QSize ImageUpscalerQt::max_result_image_size() {
	QSize cur_max_img_size = max_image_size();

	for (int i = 0; i < tasks.size(); i++)
		cur_max_img_size = tasks[i]->img_size_after(cur_max_img_size);

	return cur_max_img_size;
}

QString ImageUpscalerQt::auto_output_path(QString orig_path) {
	QString extension = orig_path.section('.', -1, -1); // Extract the extension.
	orig_path.chop(extension.size() + 1); // Remove extension from the path.

	if (orig_path[orig_path.size() - 1].isDigit())
		orig_path += '_';

	// Try to name the file like "/bla/bla/orig1.png".
	// But if this file already exists, try "/bla/bla/orig2.png" and so on...
	for (int i = 1; i < INT_MAX; i++) {
		QString cur_path = orig_path + QString::number(i) + '.' + extension;
		if (!QFile::exists(cur_path))
			return cur_path;
	}

	// We have reached INT_MAX? Impossible. Throw an exception!
	throw std::runtime_error("Can't select the output folder automatically.");
}

unsigned long long ImageUpscalerQt::max_nn_memory_consumption() {
	QSize cur_max_img_size = max_image_size();
	unsigned long long cur_max_mem = 0;
	for (int i = 0; i < tasks.size(); i++) {
		// Compute current image size after every task.
		cur_max_img_size = tasks[i]->img_size_after(cur_max_img_size);

		if (tasks[i]->task_kind() == TaskKind::srcnn) {
			TaskSRCNNDesc* desc = static_cast<TaskSRCNNDesc*>(tasks[i].get());
			QSize cur_block_size = desc->block_size == 0 ?
				cur_max_img_size :
				QSize(desc->block_size, desc->block_size);

			unsigned long long cur_mem =
				func::predict_cnn_memory_consumption(desc->srcnn_desc, cur_block_size);
			if (cur_mem > cur_max_mem)
				cur_max_mem = cur_mem;
		}
		else if (tasks[i].get()->task_kind() == TaskKind::fsrcnn) {
			TaskFSRCNNDesc* desc = static_cast<TaskFSRCNNDesc*>(tasks[i].get());
			QSize cur_block_size = desc->block_size == 0 ?
				cur_max_img_size :
				QSize(desc->block_size, desc->block_size);

			unsigned long long cur_mem =
				func::predict_cnn_memory_consumption(desc->fsrcnn_desc, cur_block_size);
			if (cur_mem > cur_max_mem)
				cur_max_mem = cur_mem;
		}
	}
	return cur_max_mem;
}

void ImageUpscalerQt::add_files(QStringList files) {
	int duplicates = files.removeDuplicates();
	int start_index = this->files.size();
	int end_index = start_index + files.size();

	for (int i = 0; i < files.size(); i++) {
		// Add to the file list.
		QString input_file = files[i];
		QString output_file = auto_output_path(files[i]);
		this->files.push_back(std::make_pair(input_file, output_file));

		// Add to the GUI.
		int cur_row = m_ui->file_list_table->rowCount();
		m_ui->file_list_table->insertRow(cur_row);
		QTableWidgetItem* item_0 = new QTableWidgetItem(input_file);
		QTableWidgetItem* item_1 = new QTableWidgetItem(output_file);
		m_ui->file_list_table->setItem(cur_row, 0, item_0);
		m_ui->file_list_table->setItem(cur_row, 1, item_1);
	}
	// Update previews.
	std::thread prev_thr(&ImageUpscalerQt::update_previews, this, start_index, end_index);
	prev_thr.detach();

	// Warn user about duplicates.
	if (duplicates > 0) {
		QString message = QString::number(duplicates) + ' ' + (duplicates == 1 ?
			tr("duplicate was removed from the list.") :
			tr("duplicates were removed from the list."));

		QMessageBox::information(this, tr("Duplicates"), message, QMessageBox::StandardButton::Ok);
	}
}

void ImageUpscalerQt::reselect_input_file(int row) {
	QFileDialog dialog(this, tr("Select input file"), QString(), FILE_FILTER);
	dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
	if (dialog.exec()) {
		// If the output path is the same as one generated automatically,
		// the generate it automatically from the new path.
		bool auto_output = files[row].second == auto_output_path(files[row].first);

		auto selected_files = dialog.selectedFiles();
		files[row].first = selected_files.first();

		if (auto_output) {
			files[row].second = auto_output_path(files[row].first);
			m_ui->file_list_table->item(row, 1)->setText(files[row].second);
		}
		m_ui->file_list_table->item(row, 0)->setText(files[row].first);

		std::thread thread(&ImageUpscalerQt::update_previews, this, row, row + 1);
		thread.detach();
	}
}

void ImageUpscalerQt::reselect_output_file(int row) {
	QFileDialog dialog(this, tr("Select output file"), QString(), FILE_FILTER);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	dialog.setFileMode(QFileDialog::FileMode::AnyFile);
	if (dialog.exec()) {
		files[row].second = dialog.selectedFiles()[0];
		m_ui->file_list_table->item(row, 1)->setText(files[row].second);
	}
}

void ImageUpscalerQt::update_previews(int start, int end) {
	// Copy links and paths here, because they can change during the process in other thread.
	std::vector<QTableWidgetItem*> items(end - start);
	std::vector<QString> files(end - start);
	for (int i = 0; i < end - start; i++) {
		items[i] = m_ui->file_list_table->item(i + start, 0);
		files[i] = this->files[i + start].first;
	}

	for (int i = 0; i < items.size(); i++) {
		// TODO: load the embedded thumbnails of the images with OpenImageIO 2.3.
		// Arch Linux 24.01.2022: OpenImageIO 2.2.18.0-4. Waiting for 2.3...
		QPixmap icon_pixmap(files[i]);
		if (icon_pixmap.isNull()) {
			items[i]->setIcon(QIcon(":unknown.svg"));
			continue;
		}

		QIcon icon(icon_pixmap.width() > icon_pixmap.height() ?
			icon_pixmap.scaledToWidth(32) :
			icon_pixmap.scaledToHeight(32));

		items[i]->setIcon(icon);
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
	QTableWidgetItem* item_1_col_0 = m_ui->file_list_table->takeItem(index_1, 0);
	QTableWidgetItem* item_2_col_0 = m_ui->file_list_table->takeItem(index_2, 0);
	QTableWidgetItem* item_1_col_1 = m_ui->file_list_table->takeItem(index_1, 1);
	QTableWidgetItem* item_2_col_1 = m_ui->file_list_table->takeItem(index_2, 1);
	m_ui->file_list_table->setItem(index_2, 0, item_1_col_0);
	m_ui->file_list_table->setItem(index_1, 0, item_2_col_0);
	m_ui->file_list_table->setItem(index_2, 1, item_1_col_1);
	m_ui->file_list_table->setItem(index_1, 1, item_2_col_1);

	// Swap items in the file list.
	std::swap(files[index_1], files[index_2]);
}

void ImageUpscalerQt::update_file_buttons() {
	auto cur_row = m_ui->file_list_table->currentRow();
	auto cur_size = files.size();

	m_ui->file_up_button->setEnabled(cur_size > 1 && cur_row > 0);
	m_ui->file_down_button->setEnabled(cur_size > 1 && cur_row != -1 && cur_row < cur_size - 1);
	m_ui->file_remove_button->setEnabled(cur_row != -1);
	m_ui->file_clear_button->setEnabled(cur_size > 0);
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

unsigned long long ImageUpscalerQt::total_pixels() {
	unsigned long long res = 0;
	for (const auto &cur_pair : files) {
		auto input = OIIO::ImageInput::open(cur_pair.first.toStdString());

		if (!input)
			continue;

		auto spec = input->spec();
		res += spec.width * spec.height;
	}
	return res;
}

void ImageUpscalerQt::update_info_text() {
	QString text;
	text += tr("Images: ") + QString::number(files.size()) + '\n';
	text += tr("Tasks: ") + QString::number(tasks.size()) + '\n';
	text += tr("Total pixels: ") + func::pixel_amount_to_string(total_pixels()) + '\n';

	auto nn_mem = max_nn_memory_consumption();
	QString nn_mem_str;
	if (nn_mem == 0)
		nn_mem_str = tr("unknown");
	else if (nn_mem < 250 * 1024 * 1024)
		nn_mem_str = "< 250 MiB";
	else
		nn_mem_str = func::bytes_amount_to_string(nn_mem);
	text += tr("Maximal memory consumption: ") + nn_mem_str + '\n';

	m_ui->info_plain_text_edit->setPlainText(text);
}

// BEGIN Slots

void ImageUpscalerQt::add_files_clicked() {
	// Create a QFileDialog
	QFileDialog dialog(this, tr("Add images and folders"), QString(), FILE_FILTER);
	dialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
	if (dialog.exec())
		add_files(dialog.selectedFiles());

	update_file_buttons();
	update_info_text();
}

void ImageUpscalerQt::move_file_up_clicked() {
	auto cur_row = m_ui->file_list_table->currentRow();
	auto cur_col = m_ui->file_list_table->currentColumn();
	if (cur_row == -1 || cur_row == 0 || files.size() < 2)
		return;

	swap_files(cur_row - 1, cur_row);
	m_ui->file_list_table->setCurrentCell(cur_row - 1, cur_col);

	update_file_buttons();
	update_info_text();
}

void ImageUpscalerQt::move_file_down_clicked() {
	auto cur_row = m_ui->file_list_table->currentRow();
	auto cur_col = m_ui->file_list_table->currentColumn();
	if (cur_row == -1 || cur_row >= files.size() - 1 || files.size() < 2)
		return;

	swap_files(cur_row, cur_row + 1);
	m_ui->file_list_table->setCurrentCell(cur_row + 1, cur_col);

	update_file_buttons();
	update_info_text();
}

void ImageUpscalerQt::remove_file_clicked() {
	auto cur_row = m_ui->file_list_table->currentRow();
	if (cur_row == -1 || cur_row > files.size() - 1)
		return;

	// Remove item from GUI.
	m_ui->file_list_table->removeRow(cur_row);

	// Remove item from the list.
	files.erase(files.begin() + cur_row);

	update_file_buttons();
	update_info_text();
}

void ImageUpscalerQt::clear_files_clicked() {
	m_ui->file_list_table->clearContents();
	m_ui->file_list_table->setRowCount(0);
	files.clear();

	update_file_buttons();
	update_info_text();
}

void ImageUpscalerQt::file_selection_changed(int, int, int, int) {
	update_file_buttons();
}

void ImageUpscalerQt::file_cell_double_clicked(int row, int column) {
	if (column == 0)
		reselect_input_file(row);
	else if (column == 1)
		reselect_output_file(row);
}

void ImageUpscalerQt::add_task_clicked() {
	TaskCreationDialog dialog(max_result_image_size());
	if (dialog.exec()) {
		const auto task_desc = dialog.get_task_desc();
		tasks.push_back(task_desc);

		m_ui->task_list_widget->addItem(task_desc->to_string());
	}

	update_task_buttons();
	update_info_text();
}

void ImageUpscalerQt::move_task_up_clicked() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	if (cur_row == -1 || cur_row == 0 || tasks.size() < 2)
		return;

	swap_tasks(cur_row - 1, cur_row);
	m_ui->task_list_widget->setCurrentRow(cur_row - 1);

	update_task_buttons();
	update_info_text();
}

void ImageUpscalerQt::move_task_down_clicked() {
	auto cur_row = m_ui->task_list_widget->currentRow();
	if (cur_row == -1 || cur_row >= tasks.size() - 1 || tasks.size() < 2)
		return;

	swap_tasks(cur_row, cur_row + 1);
	m_ui->task_list_widget->setCurrentRow(cur_row + 1);

	update_task_buttons();
	update_info_text();
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
	update_info_text();
}

void ImageUpscalerQt::clear_tasks_clicked() {
	m_ui->task_list_widget->clear();
	tasks.clear();

	update_task_buttons();
	update_info_text();
}

void ImageUpscalerQt::task_selection_changed(int index) {
	update_task_buttons();
}

void ImageUpscalerQt::about_program_triggered() {
	QMessageBox::about(this, tr("About ImageUpscalerQt"), tr("Version: ") +
														  VERSION + ".\n\n" +
														  tr(ABOUT_TEXT));
}

void ImageUpscalerQt::about_qt_triggered() {
	QMessageBox::aboutQt(this);
}


void ImageUpscalerQt::start_tasks_clicked() {
	// Invalid cases.
	if (tasks.empty()) {
		QMessageBox::warning(this, tr("No tasks"), tr("Impossible to start tasks: task queue is empty."));
		return;
	}
	if (files.empty()) {
		QMessageBox::warning(this, tr("No images"), tr("Impossible to start tasks: no files selected."));
		return;
	}

	TasksWaitingDialog* dialog = new TasksWaitingDialog();
	dialog->setModal(true);
	dialog->show();
	dialog->do_tasks(tasks, files);
}

// END Slots
