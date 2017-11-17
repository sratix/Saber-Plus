#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDebug>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_CMAKE_Project_triggered()
{
    currentCmakeFilePath = QFileDialog::getOpenFileName(nullptr, "Open CMAKE project file", "", "", nullptr, nullptr);

    currentCMakeRootPath = QFileInfo(currentCmakeFilePath).absoluteDir().absolutePath();

    filesystemModel = new QFileSystemModel;
    filesystemModel->setRootPath(currentCMakeRootPath);

    ui->treeView->setModel(filesystemModel);
    ui->treeView->setRootIndex(filesystemModel->setRootPath(currentCMakeRootPath));
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,"Saber-Plus","Simple, fast code editor by Demens Deum 2017");
}

void MainWindow::saveCurrentOpenedSourceFilePath()
{
    if (currentOpenedSourceFilePath.length() > 0) {

        QFile outputFile(currentOpenedSourceFilePath);
        outputFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream fileOutput(&outputFile);
        fileOutput << ui->textEdit->toPlainText();

        outputFile.close();

        qDebug() << "Changes to file saved " << currentOpenedSourceFilePath;
    }
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    qDebug() << "treeViewClicked";

    auto filePath = filesystemModel->filePath(index);

    qDebug() << filePath;

    if (filePath.contains(".cpp") || filePath.contains(".h") || filePath.contains(".txt")) {

        qDebug() << "Source file found";

        if (currentOpenedSourceFilePath.length() > 0) {

            saveCurrentOpenedSourceFilePath();
        }

        QFile sourceFile(filePath);

        sourceFile.open(QIODevice::ReadOnly);

        auto sourceFileContent = sourceFile.readAll();

        sourceFile.close();

        ui->textEdit->setText(sourceFileContent);

        currentOpenedSourceFilePath = filePath;
    }
}

void MainWindow::on_actionSave_triggered()
{
    saveCurrentOpenedSourceFilePath();
}

void MainWindow::on_actionBuild_triggered()
{
    prebuild();
    build();
}

void MainWindow::build()
{
    QString buildString;
    buildString = "make";

    QProcess process;

    process.setWorkingDirectory(currentCMakeRootPath);
    process.start(buildString);
    process.waitForFinished();

    auto output = process.readAllStandardOutput();

    ui->textBrowser->setText(output);
}

void MainWindow::prebuild()
{
    QString buildString;
    buildString = "cmake .";

    QProcess process;

    process.setWorkingDirectory(currentCMakeRootPath);
    process.start(buildString);
    process.waitForFinished();

    auto output = process.readAllStandardOutput();

    ui->textBrowser->setText(output);
}

void MainWindow::clean()
{
    QString cmakeFilesPath = currentCMakeRootPath + "/CMakeFiles";
    QString cmakeInstallFilePath = currentCMakeRootPath + "/cmake_install.cmake";
    QString cmakeCachePath = currentCMakeRootPath + "/CMakeCache.txt";
    QString makeFilePath = currentCMakeRootPath + "Makefile";

    auto projectDirectory = QDir(cmakeFilesPath);
    projectDirectory.removeRecursively();

    QFile::remove(cmakeInstallFilePath);
    QFile::remove(cmakeCachePath);
    QFile::remove(makeFilePath);
}

void MainWindow::on_actionClean_triggered()
{
    clean();
}
