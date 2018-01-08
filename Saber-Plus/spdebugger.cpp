#include "spdebugger.h"

#include "spprojectbuilderservice.h"

#include <QDebug>

void SPDebuggerDelegate::debuggerDidGetProcessOutput(SPDebugger *debugger, QString processOutput) {

    qDebug() << "Unused SPDebuggerDelegate debuggerDidGetProcessOutput call "<< debugger << " ; " << processOutput;

}

SPDebugger::SPDebugger(QObject *parent) : QObject(parent) {

    process = nullptr;

}

void SPDebugger::sendCommand(shared_ptr<string> command) {

    if (process == nullptr) {

        return;

    }

    if (command.get() == nullptr) {

        return;

    }

    process->write(command->c_str());
    process->write("\n");

}

void SPDebugger::printVariables() {

    if (process == nullptr) {

        return;

    }

    process->write("fr var --ptr-depth=0\n");

}

void SPDebugger::printStack() {

    if (process == nullptr) {

        return;

    }

    process->write("thread backtrace\n");

}

void SPDebugger::start() {

    if (SPProjectBuilderService::resolveProjectExecutableIfNeeded(project) == false) {

        return;

    }

    process = new QProcess();
    process->setWorkingDirectory(QString(project->projectWorkingDirectoryPath->c_str()));
    process->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(process, &QProcess::readyReadStandardOutput, this, &SPDebugger::readyReadStandardOutput);
    QObject::connect(process, &QProcess::stateChanged, this, &SPDebugger::stateChanged);

    auto runFilePath = QString(project->projectExecutablePath->c_str());

    process->start("lldb " + runFilePath);

}

void SPDebugger::run() {

    if (process == nullptr) {

        return;

    }

    for (auto item : project->pathToFilesMap) {

        QString filePath = QString(item.first.c_str());

        QString fileName = filePath.split("/").last();

        auto file = item.second;

        for (auto breakpointItem : file->linesToBreakpointsMap) {

            QString breakpointLine = QString::number(breakpointItem.first);

            QString breakpointCommand = "breakpoint set --file " + fileName + " --line  " + breakpointLine + "\n";

            process->write(breakpointCommand.toUtf8());
        }

    }

    process->write("run\n");

}

void SPDebugger::kill() {

    if (process == nullptr) {

        return;

    }

    process->kill();
    process->deleteLater();

    process = nullptr;
}

void SPDebugger::toogleBreakpointForFilePathAtLine(QString filePath, int line) {

    if (filePath.startsWith("/")) {

        qDebug() << "SPDebugger::toogleBreakpointForFilePathAtLine incorrect path: " << filePath << "; only relative path supported.";

        return;

    }

    auto filePathString = make_shared<string>(filePath.toUtf8());

    auto file = project->fileAtPath(filePathString);

    if (file.get() == nullptr) {

        file = project->makeFileWithPath(filePathString);

    }

    if (file.get() == nullptr) {

        return;

    }

    file->toggleBreakpointAtLine(line);

}

void SPDebugger::readyReadStandardOutput() {

    auto output = process->readAllStandardOutput();

    delegate->debuggerDidGetProcessOutput(this, output);

}

void SPDebugger::stateChanged(QProcess::ProcessState newState) {

}
