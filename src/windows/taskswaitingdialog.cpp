#include "taskswaitingdialog.h"
#include "ui_taskswaitingdialog.h"

TasksWaitingDialog::TasksWaitingDialog()
	: m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);
}

TasksWaitingDialog::~TasksWaitingDialog() {

}
