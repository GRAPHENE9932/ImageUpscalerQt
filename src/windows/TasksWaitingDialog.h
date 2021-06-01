/*
 * ImageUpscalerQt tasks waiting dialog
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

#pragma once

#include <QScopedPointer>
#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <OpenImageIO/imagebuf.h>

#include "../tasks/Task.h"
#include "../tasks/Worker.h"

namespace Ui {
	class TasksWaitingDialog;
}
class TasksWaitingDialog : public QDialog {
	Q_OBJECT

public:
	TasksWaitingDialog();
	~TasksWaitingDialog();

	void do_tasks(std::vector<Task*> task_queue, QString image_filename);

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
	void save_clicked();
	void progress_check();
};
