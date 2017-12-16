#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDebug>
#include <QThread>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    runFilePath = "";

    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_CMAKE_Project_triggered()
{
    currentCmakeFilePath = QFileDialog::getOpenFileName(nullptr, "Open CMAKE project file", "", "", nullptr, nullptr);

    if (currentCmakeFilePath.length() < 1)
    {
        return;
    }

    currentCMakeRootPath = QFileInfo(currentCmakeFilePath).absoluteDir().absolutePath();

    filesystemModel = new QFileSystemModel;
    filesystemModel->setRootPath(currentCMakeRootPath);

    ui->treeView->setModel(filesystemModel);
    ui->treeView->setRootIndex(filesystemModel->setRootPath(currentCMakeRootPath));
    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnHidden(2, true);
    ui->treeView->setColumnHidden(3, true);

    runFilePath = "";
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

    if (filePath.contains(".cpp") || filePath.contains(".h") || filePath.contains(".txt") || filePath.endsWith("CMakeLists.txt")) {

        qDebug() << "Source file found";

        if (currentOpenedSourceFilePath.length() > 0) {

            saveCurrentOpenedSourceFilePath();
        }

        QFile sourceFile(filePath);

        sourceFile.open(QIODevice::ReadOnly);

        auto sourceFileContent = sourceFile.readAll();

        sourceFile.close();

        ui->textEdit->document()->setPlainText(sourceFileContent);

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

    processOutput = "";

    process = new QProcess();
    process->setWorkingDirectory(currentCMakeRootPath);
    process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);
    //QObject::connect(process, &QProcess::readyReadStandardError, this, &MainWindow::readyReadStandardError);

    process->start(buildString);
}

void MainWindow::prebuild()
{
    QString buildString;
    buildString = "cmake -DCMAKE_BUILD_TYPE=Debug .";

    processOutput = "";

    process = new QProcess();

    process->setWorkingDirectory(currentCMakeRootPath);
    process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);
    //QObject::connect(process, &QProcess::readyReadStandardError, this, &MainWindow::readyReadStandardError);

    process->start(buildString);
}

void MainWindow::clean()
{
    QString cmakeFilesPath = currentCMakeRootPath + "/CMakeFiles";
    QString cmakeInstallFilePath = currentCMakeRootPath + "/cmake_install.cmake";
    QString cmakeCachePath = currentCMakeRootPath + "/CMakeCache.txt";
    QString makeFilePath = currentCMakeRootPath + "/Makefile";

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

void MainWindow::readyReadStandardOutput()
{
    auto output = process->readAllStandardOutput();

    processOutput += output;

    ui->textBrowser->setText(processOutput);

    auto verticalScrollBar = ui->textBrowser->verticalScrollBar();
    verticalScrollBar->setValue(verticalScrollBar->maximum());
}

void MainWindow::readyReadStandardError()
{
    auto output = process->readAllStandardError();

    processOutput += output;

    ui->textBrowser->setText(processOutput);
}

void MainWindow::run()
{
    if (runFilePath.length() < 1)
    {
        runFilePath = QFileDialog::getOpenFileName(nullptr, "Select file to run", currentCMakeRootPath, "", nullptr, nullptr);
    }

    if (runFilePath.length() < 1)
    {
        return;
    }

    processOutput = "";

    process = new QProcess();
    process->setWorkingDirectory(currentCMakeRootPath);
    process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);

    process->start(runFilePath);
}

void MainWindow::on_actionBuild_Run_triggered()
{
    prebuild();
    build();
    run();
}

void MainWindow::on_actionDebug_triggered()
{
    if (runFilePath.length() < 1)
    {
        runFilePath = QFileDialog::getOpenFileName(nullptr, "Select file to run", currentCMakeRootPath, "", nullptr, nullptr);
    }

    if (runFilePath.length() < 1)
    {
        return;
    }

    process = new QProcess();
    process->setWorkingDirectory(currentCMakeRootPath);
    process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);

    process->start("lldb " + runFilePath);
}

void MainWindow::on_pushButton_clicked()
{
    auto commandText = ui->plainTextEdit->toPlainText() + "\n";

    process->write(commandText.toUtf8());
}
