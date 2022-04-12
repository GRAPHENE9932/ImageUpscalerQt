/*
 * ImageUpscalerQt - task creation dialog
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDirIterator>
#include <QPushButton>

#include "TaskCreationDialog.hpp"
#include "ui_TaskCreationDialog.h"

#include "../functions/func.hpp"

constexpr int DEF_RES = 512;
constexpr size_t ORANGE_MEM = 1ull * 1024ull * 1024ull * 1024ull; // 1 GiB.
constexpr size_t RED_MEM = 2ull * 1024ull * 1024ull * 1024ull; // 2 GiB.

TaskCreationDialog::TaskCreationDialog() : m_ui(new Ui::TaskCreationDialog) {
	m_ui->setupUi(this);

	// Set dialog icon.
	setWindowIcon(QIcon(":icon.png"));
}

TaskCreationDialog::TaskCreationDialog(QSize size) :
	size(size), m_ui(new Ui::TaskCreationDialog) {
    m_ui->setupUi(this);

	// Set dialog icon.
	setWindowIcon(QIcon(":icon.png"));
}

TaskCreationDialog::~TaskCreationDialog() {

}

QString TaskCreationDialog::mem_consumption_to_string(unsigned long long bytes) {
	if (bytes == 0)
		return tr("no image");

	QString mem_str = func::bytes_amount_to_string(bytes);

	QString begin_tag, end_tag;
	if (bytes > RED_MEM) {
		begin_tag = "<span style=\"color:red;font-weight:bold\">";
		end_tag = "</span>";
	}
	else if (bytes > ORANGE_MEM) {
		begin_tag = "<span style=\"color:orange;font-weight:bold\">";
		end_tag = "</span>";
	}

	return begin_tag + mem_str + end_tag;
}

// BEGIN TaskResize
void TaskCreationDialog::init_resize() {
	m_ui->resize_x->setValue(size.isNull() ? DEF_RES : size.width());
	m_ui->resize_y->setValue(size.isNull() ? DEF_RES : size.height());

	// Disable the keep ratio check box if we don't have image selected yet.
	m_ui->resize_keep_ratio_check_box->setEnabled(size.isNull());
	m_ui->resize_keep_ratio_check_box->setChecked(size.isNull());

	resize_update();
}

bool TaskCreationDialog::valid_resize() {
	return m_ui->resize_x->value() > 0 &&
		   m_ui->resize_y->value() > 0;
}

void TaskCreationDialog::resize_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_resize()
	);
}

TaskResizeDesc TaskCreationDialog::create_resize() {
	return TaskResizeDesc((Interpolation)m_ui->resize_interpolation_combo_box->currentIndex(),
						  QSize(m_ui->resize_x->value(), m_ui->resize_y->value()));
}

void TaskCreationDialog::resize_x_changed(int x) {
	if (m_ui->resize_keep_ratio_check_box->isChecked() && !size.isNull()) {
		// Keep ratio.
		float xy_ratio = static_cast<float>(size.width()) / size.height();

		int y_value = x / xy_ratio;

		m_ui->resize_y->blockSignals(true);
		m_ui->resize_y->setValue(y_value);
		m_ui->resize_y->blockSignals(false);
	}
	resize_update();
}

void TaskCreationDialog::resize_y_changed(int y) {
	if (m_ui->resize_keep_ratio_check_box->isChecked() && !size.isNull()) {
		// Keep ratio.
		float xy_ratio = static_cast<float>(size.width()) / size.height();

		int x_value = y * xy_ratio;

		m_ui->resize_x->blockSignals(true);
		m_ui->resize_x->setValue(x_value);
		m_ui->resize_x->blockSignals(false);
	}
	resize_update();
}

void TaskCreationDialog::resize_keep_ratio_changed(bool checked) {
	if (checked)
		resize_x_changed(m_ui->resize_x->value());
}

void TaskCreationDialog::resize_interpolation_changed(int) {
	resize_update();
}

void TaskCreationDialog::ccs_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(
		valid_ccs()
	);
}
// END TaskResize

// BEGIN TaskConvertColorSpace
void TaskCreationDialog::init_ccs() {
	// Nothing to do here.
	ccs_update();
}

bool TaskCreationDialog::valid_ccs() {
	return true; // Always valid.
}

TaskConvertColorSpaceDesc TaskCreationDialog::create_ccs() {
	return TaskConvertColorSpaceDesc((ColorSpaceConversion)m_ui->color_space_combo_box->currentIndex());
}

void TaskCreationDialog::ccs_combo_box_changed(int) {
	ccs_update();
}
// END TaskConvertColorSpace

// BEGIN TaskSRCNN
void TaskCreationDialog::init_srcnn() {
	srcnn_list.clear();

	// Iterate through all resources to find SRCNNs and add them to our list (vector).
	QDirIterator iter(":/srcnn");
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

	srcnn_update();
}

bool TaskCreationDialog::valid_srcnn() {
	return (m_ui->srcnn_split_check_box->isChecked() && m_ui->srcnn_block_size_spin_box->value() >= 16) ||
		   !m_ui->srcnn_split_check_box->isChecked();
}

QSize TaskCreationDialog::srcnn_block_size() {
	if (m_ui->srcnn_split_check_box->isChecked())
		return QSize(m_ui->srcnn_block_size_spin_box->value(), m_ui->srcnn_block_size_spin_box->value());

	return size;
}

void TaskCreationDialog::srcnn_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(valid_srcnn());
	m_ui->srcnn_block_size_spin_box->setEnabled(m_ui->srcnn_split_check_box->isChecked());

	// Construct the memory consumption string.
	QString mem_str;
	if (size.isNull()) {
		mem_str = tr("unknown");
	}
	else {
		unsigned long long mem = func::predict_cnn_memory_consumption(
			srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()], srcnn_block_size()
		);
		mem_str = mem_consumption_to_string(mem);
	}

	// Construct the operations amount string.
	QString opers_str;
	if (size.isNull()) {
		opers_str = tr("unknown");
	}
	else {
		unsigned long long opers = func::srcnn_operations_amount(
			srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()], srcnn_block_size()
		);
		opers *= func::blocks_amount(size, srcnn_block_size());
		opers_str = func::big_number_to_string(opers);
	}

	QString result_str = QString("<p>Total operations: %1</p>"
								 "<p>Approximate memory consumption: %2</p>").arg(
								 opers_str, mem_str);

	m_ui->srcnn_info_label->setText(result_str);
}

TaskSRCNNDesc TaskCreationDialog::create_srcnn() {
	int block_size;
	if (m_ui->srcnn_split_check_box->isChecked())
		block_size = m_ui->srcnn_block_size_spin_box->value();
	else
		block_size = 0;

	return TaskSRCNNDesc(srcnn_list[m_ui->srcnn_architecture_combo_box->currentIndex()],
						 block_size);
}

void TaskCreationDialog::srcnn_architecture_changed(int index) {
	if (index == -1)
		return;
	srcnn_update();
}

void TaskCreationDialog::srcnn_split_changed(bool) {
	srcnn_update();
}

void TaskCreationDialog::srcnn_block_size_changed(int) {
	srcnn_update();
}
// END TaskSRCNN

// BEGIN TaskFSRCNN
void TaskCreationDialog::update_fsrcnn_list() {
	fsrcnn_list.clear();

	// Iterate through all resources to find SRCNNs and add them to our list (vector).
	QDirIterator iter(":/fsrcnn");
	while (iter.hasNext()) {
		QString cur_file_name = iter.next().section('/', -1);
		// Allow only FSRCNNs with the selected size multiplier.
		if (!cur_file_name.startsWith(m_ui->fsrcnn_multiplier_combo_box->currentText()))
			continue;

		FSRCNNDesc cur_desc;
		// Leave only filename without extension to pass it to the parser.
		QString cur_desc_str = cur_file_name.section('.', -2, -2);
		if (FSRCNNDesc::from_string(cur_desc_str, &cur_desc)) // If parsing successful.
			fsrcnn_list.push_back(cur_desc);
	}

	// Sort this list
	std::sort(fsrcnn_list.begin(), fsrcnn_list.end());

	// Add entries to combo box.
	m_ui->fsrcnn_architecture_combo_box->clear();
	for (const FSRCNNDesc& cur_desc : fsrcnn_list)
		m_ui->fsrcnn_architecture_combo_box->addItem(cur_desc.to_string(false));
}

void TaskCreationDialog::init_fsrcnn() {
	update_fsrcnn_list();
	fsrcnn_update();
}

bool TaskCreationDialog::valid_fsrcnn() {
	return (m_ui->fsrcnn_split_check_box->isChecked() && m_ui->fsrcnn_block_size_spin_box->value() >= 16) ||
		   !m_ui->fsrcnn_split_check_box->isChecked();
}

QSize TaskCreationDialog::fsrcnn_block_size() {
	if (m_ui->fsrcnn_split_check_box->isChecked())
		return QSize(m_ui->fsrcnn_block_size_spin_box->value(), m_ui->fsrcnn_block_size_spin_box->value());

	return size;
}

void TaskCreationDialog::fsrcnn_update() {
	m_ui->main_button_box->button(QDialogButtonBox::Ok)->setEnabled(valid_fsrcnn());
	m_ui->fsrcnn_block_size_spin_box->setEnabled(m_ui->fsrcnn_split_check_box->isChecked());
	m_ui->fsrcnn_margin_spin_box->setMinimum(-m_ui->fsrcnn_block_size_spin_box->value() / 2);

	QString mem_str;
	if (size.isNull()) {
		mem_str = tr("unknown");
	}
	else {
		// Construct the memory consumption string.
		unsigned long long mem = func::predict_cnn_memory_consumption(
			fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()], fsrcnn_block_size()
		);
		mem_str = mem_consumption_to_string(mem);
	}

	QString opers_str;
	if (size.isNull()) {
		opers_str = tr("unknown");
	}
	else {
		// Construct the operations amount string.
		unsigned long long opers = func::fsrcnn_operations_amount(
			fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()], fsrcnn_block_size()
		);
		opers *= func::blocks_amount(size, fsrcnn_block_size());
		opers_str = func::big_number_to_string(opers);
	}

	QString result_str = QString("<p>Total operations: %1</p>"
								 "<p>Approximate memory consumption: %2</p>").arg(
								 opers_str, mem_str);

	m_ui->fsrcnn_info_label->setText(result_str);
}

TaskFSRCNNDesc TaskCreationDialog::create_fsrcnn() {
	int block_size;
	if (m_ui->fsrcnn_split_check_box->isChecked())
		block_size = m_ui->fsrcnn_block_size_spin_box->value();
	else
		block_size = 0;

	int margin = m_ui->fsrcnn_margin_spin_box->value();
	return TaskFSRCNNDesc(fsrcnn_list[m_ui->fsrcnn_architecture_combo_box->currentIndex()],
						  block_size, margin);
}

void TaskCreationDialog::fsrcnn_multiplier_changed(int) {
	update_fsrcnn_list();
	fsrcnn_update();
}

void TaskCreationDialog::fsrcnn_architecture_changed(int index) {
	if (index == -1)
		return;
	fsrcnn_update();
}

void TaskCreationDialog::fsrcnn_split_changed(bool) {
	fsrcnn_update();
}

void TaskCreationDialog::fsrcnn_block_size_changed(int) {
	fsrcnn_update();
}

void TaskCreationDialog::fsrcnn_margin_changed(int) {
	fsrcnn_update();
}

// END TaskFSRCNN

std::shared_ptr<TaskDesc> TaskCreationDialog::get_task_desc() {
	switch ((TaskKind)m_ui->parameters_stacked_widget->currentIndex()) {
	case TaskKind::resize:
		return std::make_shared<TaskResizeDesc>(create_resize());
		break;
	case TaskKind::convert_color_space:
		return std::make_shared<TaskConvertColorSpaceDesc>(create_ccs());
		break;
	case TaskKind::srcnn:
		return std::make_shared<TaskSRCNNDesc>(create_srcnn());
		break;
	case TaskKind::fsrcnn:
		return std::make_shared<TaskFSRCNNDesc>(create_fsrcnn());
		break;
	default:
		return nullptr; // Impossible.
		break;
	}
}

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
