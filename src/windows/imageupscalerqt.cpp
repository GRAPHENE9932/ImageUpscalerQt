#include <filesystem>

#include <QtDebug>
#include <QDir>
#include <QMessageBox>

#include "imageupscalerqt.h"
#include "ui_imageupscalerqt.h"
#include "../Algorithms.h"

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt)
{
    m_ui->setupUi(this);

	//BEGIN Connect signals
	connect(m_ui->add_task_button, SIGNAL(clicked()), this, SLOT(add_task_clicked()));
	connect(m_ui->task_kind_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(task_kind_changed(int)));
	//END Connect signals
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

void ImageUpscalerQt::add_task_clicked() {
	qDebug() << "Add task button clicked";

	//Add task
	switch ((TaskKind)m_ui->task_kind_combobox->currentIndex()) {
		case TaskKind::resize: {
			TaskResize* cur_task = new TaskResize(
				m_ui->resize_x->value(),
				m_ui->resize_y->value(),
				(Interpolation)m_ui->resize_interpolation_combobox->currentIndex()
			);

			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::srcnn: {

			break;
		}
		case TaskKind::fsrcnn: {
			break;
		}
	}
}

void ImageUpscalerQt::task_kind_changed(int index) {
	qDebug() << "Task kind changed!" << index;

	switch ((TaskKind)index) {
		//BEGIN SRCNN
		case TaskKind::srcnn: {
			//Get folder that contains .pt files
			std::string nn_storage_path = QDir::currentPath().toStdString() + "/SRCNN/";

			//Check if this folder exists
			if (!std::filesystem::exists(nn_storage_path)) {
				QMessageBox::critical(this, "Missing files", "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList names;
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(nn_storage_path); iter != end; iter++) {
				//Check name if it's srcnn architecture
				std::string cur_name = iter->path().stem();
				std::string cur_extension = iter->path().extension();
				if (Algorithms::parse_srcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == ".pt")
					names.push_back(QString::fromStdString(cur_name));
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}
			//Insert they into combobox
			m_ui->srcnn_architecture_combobox->clear();
			m_ui->srcnn_architecture_combobox->addItems(names);
		}
		//END SRCNN

		//BEGIN FSRCNN
		case TaskKind::fsrcnn: {
			//Get folder that contains .pt files
			std::string nn_storage_path = QDir::currentPath().toStdString() + "/FSRCNN/";

			//Check if this folder exists
			if (!std::filesystem::exists(nn_storage_path)) {
				QMessageBox::critical(this, "Missing files", "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList names;
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(nn_storage_path); iter != end; iter++) {
				//Check name if it's fsrcnn architecture
				std::string cur_name = iter->path().stem();
				std::string cur_extension = iter->path().extension();
				if (Algorithms::parse_fsrcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == ".pt")
					names.push_back(QString::fromStdString(cur_name));
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Insert they into combobox
			m_ui->fsrcnn_architecture_combobox->clear();
			m_ui->fsrcnn_architecture_combobox->addItems(names);
		}
		//END FSRCNN
	}

	//Toggle stacked widget
	m_ui->task_settings_widget->setCurrentIndex(index);
}

