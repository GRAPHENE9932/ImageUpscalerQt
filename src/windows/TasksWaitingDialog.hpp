/*
 * ImageUpscalerQt - task waiting dialog header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QScopedPointer>
#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <OpenImageIO/imagebuf.h>

#include "../tasks/TaskDesc.hpp"
#include "../tasks/Worker.hpp"

namespace Ui {
	class TasksWaitingDialog;
}
class TasksWaitingDialog : public QDialog {
	Q_OBJECT

public:
	TasksWaitingDialog();
	~TasksWaitingDialog();

	void do_tasks(std::vector<std::shared_ptr<TaskDesc>> tasks,
				  std::vector<std::pair<QString, QString>> files);

private:
	QScopedPointer<Ui::TasksWaitingDialog> m_ui;

	QTimer* timer;

	bool tasks_complete = false;
	bool cancelled = false;
	bool error_received = false;
	QString error_message;

	bool image_saved = false;

	std::thread* progress_thread;
	std::thread* tasks_thread;

	Worker* worker;
	QElapsedTimer elapsed_timer;

	void reject();

private slots:
	void cancel_clicked();
	void progress_check();
};
