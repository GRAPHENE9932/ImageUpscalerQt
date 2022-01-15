/*
 * ImageUpscalerQt - task creation dialog header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDialog>
#include <QScopedPointer>

#include "../tasks/TaskDesc.h"

namespace Ui {
	class TaskCreationDialog;
}

class TaskCreationDialog : public QDialog {
    Q_OBJECT

public:
	// Image width, height and amount of channels. Used for initialization of default task parameters.
	int x_size = -1, y_size = -1;
	char ch_n;

    explicit TaskCreationDialog();
    TaskCreationDialog(int x_size, int y_size, char ch_n);
	~TaskCreationDialog() override;

private:
    QScopedPointer<Ui::TaskCreationDialog> m_ui;
	std::vector<SRCNNDesc> srcnn_list;
	std::vector<FSRCNNDesc> fsrcnn_list;

	// Initialization for every task kind.
	void init_resize();
	void init_ccs();
	void init_srcnn();
	void init_fsrcnn();

	// Validate parameters for every task kind.
	bool valid_resize();
	bool valid_ccs();
	bool valid_srcnn();
	bool valid_fsrcnn();

	// Parameters update event for every task kind.
	void resize_update();
	void ccs_update();
	void srcnn_update();
	void fsrcnn_update();

	// Construct every task description from the UI.
	TaskResizeDesc create_resize();
	TaskConvertColorSpaceDesc create_ccs();
	TaskSRCNNDesc create_srcnn();
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

	void fsrcnn_architecture_changed(int index);
	void fsrcnn_split_changed(bool checked);
	void fsrcnn_block_size_changed(int size);
};
