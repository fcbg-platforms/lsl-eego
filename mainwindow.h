#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QThread>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <vector>

// LSL API
#include <lsl/lsl_cpp.h>

// eegoSports
#define WIN32_LEAN_AND_MEAN
#define EEGO_SDK_BIND_STATIC

#ifdef _WIN32
#include <Windows.h>
#endif

#include "eemagine/sdk/factory.h"

// --------------------------------------------------------------------------------
// Reader class
// --------------------------------------------------------------------------------
class Reader : public QObject {
    Q_OBJECT

public:
    Reader();
    ~Reader();

public slots:
    void read();
    void setStop(bool stop) {
        this->stop = stop;
    }
    void setParams(int capId, int samplingRate, double BipRange, double EegRange, unsigned long long hexEegMask, unsigned long long hexBipMask);

private:
    void OpenEegStream();

signals:
    void finished();
    void timeout();
    void ampNotFound();
    void connectionLost();
    void unknownError();

private:
    // SDK classes
    eemagine::sdk::factory fact;
    eemagine::sdk::amplifier* amp;
    eemagine::sdk::stream* eegStream;
    eemagine::sdk::buffer buffer;

    // amplifier parameters
    int samplingRate;
    double EegRange;
    double BipRange;
    unsigned long long hexEegMask;
    unsigned long long hexBipMask;

    // Cap layout
    std::vector<std::string> capLayout;

    // Stop acquisition if set to false.
    bool stop;
};

// --------------------------------------------------------------------------------
// MainWindow Class
// --------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent, const std::string &config_file, const bool linkOnStart);
    ~MainWindow();

private slots:
    void threadFinished();
    void threadTimeout();
    void connectionLost();
    void ampNotFound();
    void unknownError();
    void load_config_dialog();
    void save_config_dialog();

    // start the eegosports connection
    void link();

    // close event (potentially disabled)
    void closeEvent(QCloseEvent *ev);
private:
    // background data reader thread
    void read_thread(std::string deviceNumber, int chunkSize, int samplingRate, bool isSlave, std::string serialNumber, int channelCount, std::vector<std::string> channelLabels);


    // raw config file IO
    void load_config(const std::string &filename);
    void save_config(const std::string &filename);

    Reader *reader;
    QThread *thread;

    Ui::MainWindowClass ui;
};

#endif // MAINWINDOW_H



