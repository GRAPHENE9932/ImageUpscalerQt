/*
 * ImageUpscalerQt - task creation dialog header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDialog>
#include <QScopedPointer>

#include "../tasks/TaskDesc.hpp"

namespace Ui {
	class TaskCreationDialog;
}

class TaskCreationDialog : public QDialog {
    Q_OBJECT

public:
	// Image width and height. Used for initialization of default task parameters.
	QSize size;

    explicit TaskCreationDialog();
    explicit TaskCreationDialog(QSize size);
	~TaskCreationDialog() override;

	std::shared_ptr<TaskDesc> get_task_desc();

private:
    QScopedPointer<Ui::TaskCreationDialog> m_ui;
	std::vector<SRCNNDesc> srcnn_list;
	std::vector<FSRCNNDesc> fsrcnn_list;

	QString mem_consumption_to_string(unsigned long long bytes);

	// TaskResize
	void init_resize();
	bool valid_resize();
	void resize_update();
	TaskResizeDesc create_resize();

	// TaskConvertColorSpace
	void init_ccs();
	bool valid_ccs();
	void ccs_update();
	TaskConvertColorSpaceDesc create_ccs();

	// TaskSRCNN
	void init_srcnn();
	QSize srcnn_block_size();
	bool valid_srcnn();
	void srcnn_update();
	TaskSRCNNDesc create_srcnn();

	// TaskFSRCNN
	void update_fsrcnn_list();
	void init_fsrcnn();
	bool valid_fsrcnn();
	QSize fsrcnn_block_size();
	void fsrcnn_update();
	TaskFSRCNNDesc create_fsrcnn();

private slots:
	void task_changed(int index);

	void resize_x_changed(int x);
	void resize_y_changed(int y);
	void resize_keep_ratio_changed(bool checked);
	void resize_interpolation_changed(int index);

	void ccs_combo_box_changed(int index);

	void srcnn_architecture_changed(int index);
	void srcnn_split_changed(bool checked);
	void srcnn_block_size_changed(int size);

	void fsrcnn_multiplier_changed(int);
	void fsrcnn_architecture_changed(int index);
	void fsrcnn_split_changed(bool checked);
	void fsrcnn_block_size_changed(int size);
	void fsrcnn_margin_changed(int);
};
