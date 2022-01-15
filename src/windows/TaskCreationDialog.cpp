/*
 * ImageUpscalerQt - task creation dialog
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDirIterator>
#include <QPushButton>

#include "TaskCreationDialog.h"
#include "ui_TaskCreationDialog.h"

#include "../functions/func.h"

constexpr int DEF_RES = 512;
constexpr int DEF_MAX = INT_MAX;
constexpr size_t ORANGE_MEM = 1ull * 1024ull * 1024ull * 1024ull; // 1 GiB.
constexpr size_t RED_MEM = 2ull * 1024ull * 1024ull * 1024ull; // 2 GiB.

TaskCreationDialog::TaskCreationDialog() : m_ui(new Ui::TaskCreationDialog) {
	m_ui->setupUi(this);

	// Set dialog icon.
	setWindowIcon(QIcon(":icon.png"));
}

TaskCreationDialog::TaskCreationDialog(int x_size, int y_size, char ch_n) :
	x_size(x_size), y_size(y_size), ch_n(ch_n), m_ui(new Ui::TaskCreationDialog) {
    m_ui->setupUi(this);

	// Set dialog icon.
	setWindowIcon(QIcon(":icon.png"));
}

TaskCreationDialog::~TaskCreationDialog() {

}

// BEGIN Initialization for every task kind.

void TaskCreationDialog::init_resize() {
	m_ui->resize_x->setValue(x_size == -1 ? DEF_RES : x_size);
	m_ui->resize_y->setValue(y_size == -1 ? DEF_RES : y_size);

	// Disable the keep ratio check box if we don't have image selected yet.
	m_ui->resize_keep_ratio_check_box->setEnabled(x_size != -1 && y_size != -1);
	m_ui->resize_keep_ratio_check_box->setChecked(x_size != -1 && y_size != -1);

	resize_update();
}

void TaskCreationDialog::init_ccs() {
	// Nothing to do here.

	ccs_update();
}

void TaskCreationDialog::init_srcnn() {
	srcnn_list.clear();

	// Iterate through all resources to find SRCNNs and add them to our list (vector).
	QDirIterator iter(":/SRCNN");
	while (iter.hasNext()) {
		SRCNNDesc cur_desc;
		// Leave only filename without extension to pass it to the parser.
		QString cur_desc_str = iter.next().section('/', -1).section('.', -2, -2);
		if (SRCNNDesc::from_string(cur_desc_str, &cur_desc)) // If parsing successful.
			srcnn_list.push_back(cur_desc);
	}

	// Sort this list
	std::sort(srcnn_list.begin(), srcnn_list.end());

	// Add entries to combo box.
	m_ui->srcnn_architecture_combo_box->clear();
	for (SRCNNDesc cur_desc : srcnn_list)
		m_ui->srcnn_architecture_combo_box->addItem(cur_desc.to_string());

	// Set maximum to the block size spin box.
	m_ui->srcnn_block_size_spin_box->setMaximum(
		x_size == -1 || y_size == -1 ? DEF_MAX : std::max(x_size, y_size)
	);

	srcnn_update();
}

void TaskCreationDialog::init_fsrcnn() {
	fsrcnn_list.clear();

	// Iterate through all resources to find SRCNNs and add them to our list (vector).
	QDirIterator iter(":/FSRCNN");
	while (iter.hasNext()) {
		FSRCNNDesc cur_desc;
		// Leave only filename without extension to pass it to the parser.
		QString cur_desc_str = iter.next().section('/', -1).section('.', -2, -2);
		if (FSRCNNDesc::from_string(cur_desc_str, &cur_desc)) // If parsing successful.
			fsrcnn_list.push_back(cur_desc);
	}

	// Sort this list
	std::sort(fsrcnn_list.begin(), fsrcnn_list.end());

	// Add entries to combo box.
	m_ui->fsrcnn_architecture_combo_box->clear();
	for (FSRCNNDesc cur_desc : fsrcnn_list)
		m_ui->fsrcnn_architecture_combo_box->addItem(cur_desc.to_string());

	// Set maximum to the block size spin box.
	m_ui->fsrcnn_block_size_spin_box->setMaximum(
		x_size == -1 || y_size == -1 ? DEF_MAX : std::max(x_size, y_size)
	);

	fsrcnn_update();
}

// END Initialization for every task kind.

// BEGIN Validate parameters for every task kind.

bool TaskCreationDialog::valid_resize() {
	return m_ui->resize_x->value() > 0 &&
		   m_ui->resize_y->value() > 0;
}

bool TaskCreationDialog::valid_ccs() {
	return true; // Always valid.
}

bool TaskCreationDialog::valid_srcnn() {
	return (m_ui->srcnn_split_check_box->isChecked() && m_ui->srcnn_block_size_spin_box->value() >= 16) ||
		   !m_ui->srcnn_split_check_box->isChecked();
}

bool TaskCreationDialog::valid_fsrcnn() {
	return (m_ui->fsrcnn_split_check_box->isChecked() && m_ui->fsrcnn_block_size_spin_box->value() >= 16) ||
		   !m_ui->fsrcnn_split_check_box->isChecked();
}

// END Validate parameters for every task kind.

// BEGIN Update every task kind.

void TaskCreationDialog::resize_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_resize()
	);
}

void TaskCreationDialog::ccs_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_ccs()
	);
}

void TaskCreationDialog::srcnn_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_srcnn()
	);

	// Display total operations and memory consumption
	QString opers_str, mem_str;
	size_t mem = 0;
	if (x_size == -1 || y_size == -1 || srcnn_list.empty()) {
		opers_str = "no image";
		mem_str = "no image";
	}
	else {
		opers_str = func::big_number_to_string(
			func::srcnn_operations_amount(
				srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()],
				x_size, y_size
			), ' '
		);
		mem = func::predict_cnn_memory_consumption(
			srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()],
			x_size, y_size);
		mem_str = func::bytes_amount_to_string(mem);
	}

	// Mark memory text as bold and red or orange.
	QString mem_begin_tag, mem_end_tag;
	if (mem > RED_MEM) {
		mem_begin_tag = "<span style=\"color:red;font-weight:bold\"";
		mem_end_tag = "</span>";
	}
	else if (mem > ORANGE_MEM) {
		mem_begin_tag = "<span style=\"color:orange;font-weight:bold\"";
		mem_end_tag = "</span>";
	}

	m_ui->srcnn_info_label->setText(
		QString("Total operations: %1\n"
		"Approximate memory consumption: %2%3%4").arg(
			opers_str, mem_begin_tag, mem_str, mem_end_tag)
	);
}

void TaskCreationDialog::fsrcnn_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_fsrcnn()
	);

	// Display total operations and memory consumption
	QString opers_str, mem_str;
	size_t mem = 0;
	if (x_size == -1 || y_size == -1 || srcnn_list.empty()) {
		opers_str = "no image";
		mem_str = "no image";
	}
	else {
		opers_str = func::big_number_to_string(
			func::fsrcnn_operations_amount(
				fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()],
				x_size, y_size
			), ' '
		);
		mem = func::predict_cnn_memory_consumption(
			fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()],
			x_size, y_size);
		mem_str = func::bytes_amount_to_string(mem);
	}

	// Mark memory text as bold and red or orange.
	QString mem_begin_tag, mem_end_tag;
	if (mem > RED_MEM) {
		mem_begin_tag = "<span style=\"color:red;font-weight:bold\"";
		mem_end_tag = "</span>";
	}
	else if (mem > ORANGE_MEM) {
		mem_begin_tag = "<span style=\"color:orange;font-weight:bold\"";
		mem_end_tag = "</span>";
	}

	m_ui->fsrcnn_info_label->setText(
		QString("Total operations: %1\n"
		"Approximate memory consumption: %2%3%4").arg(
			opers_str, mem_begin_tag, mem_str, mem_end_tag)
	);
}

// END Update every task kind.

// BEGIN Construct every task description

TaskResizeDesc TaskCreationDialog::create_resize() {
	return TaskResizeDesc((Interpolation)m_ui->resize_interpolation_combo_box->currentIndex(),
						  m_ui->resize_x->value(), m_ui->resize_x->value());
}

TaskConvertColorSpaceDesc TaskCreationDialog::create_ccs() {
	return TaskConvertColorSpaceDesc((ColorSpaceConversion)m_ui->color_space_combo_box->currentIndex());
}

TaskSRCNNDesc TaskCreationDialog::create_srcnn() {
	return TaskSRCNNDesc(srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()],
						 m_ui->srcnn_block_size_spin_box->value());
}

TaskFSRCNNDesc TaskCreationDialog::create_fsrcnn() {
	return TaskFSRCNNDesc(fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()],
						  m_ui->fsrcnn_block_size_spin_box->value());
}

// END Construct every task description

// BEGIN Slots.

void TaskCreationDialog::task_changed(int index) {
	m_ui->parameters_stacked_widget->setCurrentIndex(index);

	switch ((TaskKind)index) {
	case TaskKind::resize:
		init_resize();
		break;
	case TaskKind::convert_color_space:
		init_ccs();
		break;
	case TaskKind::srcnn:
		init_srcnn();
		break;
	case TaskKind::fsrcnn:
		init_fsrcnn();
		break;
	}
}

void TaskCreationDialog::resize_x_changed(int x) {
	if (m_ui->resize_keep_ratio_check_box->isChecked() && x_size != -1 && y_size != -1) {
		float xy_ratio = (float)x_size / y_size;

		int y_value = x / xy_ratio;

		m_ui->resize_y->blockSignals(true);
		m_ui->resize_y->setValue(y_value);
		m_ui->resize_y->blockSignals(false);
	}
}

void TaskCreationDialog::resize_y_changed(int y) {
	if (m_ui->resize_keep_ratio_check_box->isChecked() && x_size != -1 && y_size != -1) {
		float xy_ratio = (float)x_size / y_size;

		int x_value = y * xy_ratio;

		m_ui->resize_x->blockSignals(true);
		m_ui->resize_x->setValue(x_value);
		m_ui->resize_x->blockSignals(false);
	}
}

void TaskCreationDialog::resize_keep_ratio_changed(bool checked) {
	if (checked)
		resize_x_changed(m_ui->resize_x->value());
}

void TaskCreationDialog::resize_interpolation_changed(int) {
	// Do nothing.
}

void TaskCreationDialog::ccs_combo_box_changed(int) {
	// Do nothing.
}

void TaskCreationDialog::srcnn_architecture_changed(int) {
	// Do nothing.
}

void TaskCreationDialog::srcnn_split_changed(bool checked) {
	m_ui->srcnn_split_check_box->setEnabled(checked);
}

void TaskCreationDialog::srcnn_block_size_changed(int) {
	// Do nothing.
}

void TaskCreationDialog::fsrcnn_architecture_changed(int) {
	// Do nothing.
}

void TaskCreationDialog::fsrcnn_split_changed(bool checked) {
	m_ui->fsrcnn_split_check_box->setEnabled(checked);
}

void TaskCreationDialog::fsrcnn_block_size_changed(int) {
	 // Do nothing.
}

// END Slots.
