#ifndef TASKSWAITINGDIALOG_H
#define TASKSWAITINGDIALOG_H

#include <QScopedPointer>
#include <QDialog>

namespace Ui {
	class TasksWaitingDialog;
}
class TasksWaitingDialog : public QDialog {
		Q_OBJECT

	public:
		/**
		 * Default constructor
		 */
		TasksWaitingDialog();

		/**
		 * Destructor
		 */
		~TasksWaitingDialog();

	private:
		QScopedPointer<Ui::TasksWaitingDialog> m_ui;
};

#endif // TASKSWAITINGDIALOG_H
