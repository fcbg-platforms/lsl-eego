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
    void setParams(const std::vector<std::string> &capLayout, int samplingRate, double BipRange, double EegRange, const std::vector<bool> &eegMask, const std::vector<bool> &bipMask);

private:
    void OpenEegStream();

signals:
    void finished();
    void timeout();
    void ampNotFound();
    void connectionLost();
    void unknownError();
    void errorMessage(const QString &message);

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
    // Channel selection masks, one entry per channel (index 0 = first channel).
    // Stored as bit vectors so that cascaded amplifiers (128 EEG + 48 BIP channels)
    // are not limited by the 64-bit masks of the SDK convenience API.
    std::vector<bool> eegMask;
    std::vector<bool> bipMask;

    // Cap layout (labels of the EEG/reference channels, in amplifier order)
    std::vector<std::string> capLayout;

    // Labels of the selected channels, in the order they were requested from the SDK
    std::vector<std::string> eegLabels;
    std::vector<std::string> bipLabels;

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
    void errorMessage(const QString &message);
    void capChanged(int index);
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

    // make the EEG/BIP masks match the selected cap (32/64/128 EEG, 24/24/48 BIP)
    void applyCapToMasks(bool resetEegMask);

    Reader *reader;
    QThread *thread;

    Ui::MainWindowClass ui;
};

#endif // MAINWINDOW_H



