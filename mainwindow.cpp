#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <thread>

using namespace eemagine::sdk;

//---------------------------------------------------------------------------
// Layouts
//---------------------------------------------------------------------------
// Default masks: every EEG channel of two cascaded amplifiers (128 bits) and
// no bipolar/AUX channel. Bits beyond the channels actually available on the
// connected amplifier(s) are ignored, so this also works with a single 64 ch amp.
static const char *DEFAULT_EEG_MASK = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char *DEFAULT_BIP_MASK = "0x000000";

// CA-203 (32 ch)
static const std::vector<std::string> layout_CA203 = {
    "Fp1",       "Fpz",       "Fp2",      "F7",      "F3",       "Fz",       "F4",
    "F8",        "FC5",       "FC1",      "FC2",     "FC6",      "M1",       "T7",
    "C3",        "Cz",        "C4",       "T8",      "M2",       "CP5",      "CP1",
    "CP2",       "CP6",       "P7",       "P3",      "Pz",       "P4",       "P8",
    "POz",       "O1",        "Oz",       "O2"
};

// CA-208 (64 ch)
static const std::vector<std::string> layout_CA208 = {
    "Fp1",       "Fpz",       "Fp2",       "F7",        "F3",        "Fz",        "F4",
    "F8",        "FC5",       "FC1",       "FC2",       "FC6",       "M1",        "T7",
    "C3",        "Cz",        "C4",        "T8",        "M2",        "CP5",       "CP1",
    "CP2",       "CP6",       "P7",        "P3",        "Pz",        "P4",        "P8",
    "POz",       "O1",        "O2",        "EOG",       "AF7",       "AF3",       "AF4",
    "AF8",       "F5",        "F1",        "F2",        "F6",        "FC3",       "FCz",
    "FC4",       "C5",        "C1",        "C2",        "C6",        "CP3",       "CP4",
    "P5",        "P1",        "P2",        "P6",        "PO5",       "PO3",       "PO4",
    "PO6",       "FT7",       "FT8",       "TP7",       "TP8",       "PO7",       "PO8",
    "OZ"
};

// CA-209 (128 ch, two cascaded amplifiers)
static const std::vector<std::string> layout_CA209 = {
    "Fp1",     "Fpz",     "Fp2",      "F7",       "F3",       "Fz",       "F4",
    "F8",      "FC5",     "FC1",      "FC2",      "FC6",      "M1",       "T7",
    "C3",      "Cz",      "C4",       "T8",       "M2",       "CP5",      "CP1",
    "CP2",     "CP6",     "P7",       "P3",       "Pz",       "P4",       "P8",
    "POz",     "O1",      "O2",       "HEOGR",    "AF7",      "AF3",      "AF4",
    "AF8",     "F5",      "F1",       "F2",       "F6",       "FC3",      "FCz",
    "FC4",     "C5",      "C1",       "C2",       "C6",       "CP3",      "CP4",
    "P5",      "P1",      "P2",       "P6",       "HEOGL",    "PO3",      "PO4",
    "VEOGU",   "FT7",     "FT8",      "TP7",      "TP8",      "PO7",      "PO8",
    "VEOGL",   "FT9",     "FT10",     "TPP9h",    "TPP10h",   "PO9",      "PO10",
    "P9",      "P10",     "AFF1",     "AFz",      "AFF2",     "FFC5h",    "FFC3h",
    "FFC4h",   "FFC6h",   "FCC5h",    "FCC3h",    "FCC4h",    "FCC6h",    "CCP5h",
    "CCP3h",   "CCP4h",   "CCP6h",    "CPP5h",    "CPP3h",    "CPP4h",    "CPP6h",
    "PPO1",    "PPO2",    "I1",       "Iz",       "I2",       "AFP3h",    "AFP4h",
    "AFF5h",   "AFF6h",   "FFT7h",    "FFC1h",    "FFC2h",    "FFT8h",    "FTT9h",
    "FTT7h",   "FCC1h",   "FCC2h",    "FTT8h",    "FTT10h",   "TTP7h",    "CCP1h",
    "CCP2h",   "TTP8h",   "TPP7h",    "CPP1h",    "CPP2h",    "TPP8h",    "PPO9h",
    "PPO5h",   "PPO6h",   "PPO10h",   "POO9h",    "POO3h",    "POO4h",    "POO10h",
    "OI1h",    "OI2h"
};

static std::vector<std::string> numberedLabels(const std::string &prefix, unsigned int count) {
    std::vector<std::string> labels;
    labels.reserve(count);
    for (unsigned int k = 1; k <= count; k++)
        labels.push_back(prefix + std::to_string(k));
    return labels;
}

// Index in the "CAP ID" combo box -> EEG channel labels
static std::vector<std::string> layoutForCap(int capId) {
    switch (capId) {
    case 0:  return layout_CA203;
    case 1:  return layout_CA208;
    case 2:  return layout_CA209;
    case 3:  return numberedLabels("Ch", 64);
    case 4:
    default: return numberedLabels("Ch", 128);
    }
}

// Label of the k-th channel (0-based) in a layout, falling back to a numbered label.
static std::string labelAt(const std::vector<std::string> &labels, size_t k, const std::string &fallbackPrefix) {
    if (k < labels.size())
        return labels[k];
    return fallbackPrefix + std::to_string(k + 1);
}

// Parse a hexadecimal channel mask of arbitrary length ("0x" prefix optional,
// spaces and underscores ignored). Bit 0 (least significant) is the first channel.
// Returns false if the string is empty or contains non-hexadecimal characters.
static bool parseHexMask(const std::string &text, std::vector<bool> &bits) {
    std::string digits;
    for (char c : text) {
        if (!std::isspace(static_cast<unsigned char>(c)) && c != '_')
            digits += c;
    }
    if (digits.size() >= 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
        digits.erase(0, 2);
    if (digits.empty())
        return false;

    bits.clear();
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        if (!std::isxdigit(static_cast<unsigned char>(*it)))
            return false;
        const int value = std::stoi(std::string(1, *it), nullptr, 16);
        for (int b = 0; b < 4; b++)
            bits.push_back(((value >> b) & 1) != 0);
    }
    return true;
}

static bool maskBit(const std::vector<bool> &mask, size_t k) {
    return k < mask.size() && mask[k];
}


MainWindow::MainWindow(QWidget *parent, const std::string &config_file, const bool linkOnStart) : QMainWindow(parent)
{
    reader = nullptr;
    thread = nullptr;
    ui.setupUi(this);

    // Init initial indexes
    if(!config_file.empty()) {
        load_config(config_file);
    } else {
        ui.Cap_ID->setCurrentIndex(1);
        ui.samplingRate->setCurrentIndex(1);
        ui.EEG_Range->setCurrentIndex(0);
        ui.BIP_Range->setCurrentIndex(2);
    }


    // make GUI connections
    QObject::connect(ui.actionLoad_Configuration, SIGNAL(triggered()), this, SLOT(load_config_dialog()));
    QObject::connect(ui.actionSave_Configuration, SIGNAL(triggered()), this, SLOT(save_config_dialog()));
    QObject::connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui.linkButton, SIGNAL(clicked()), this, SLOT(link()));
    if(linkOnStart) {
        link();
    }
}

void MainWindow::load_config_dialog() {
    QString sel = QFileDialog::getOpenFileName(this, "Load Configuration File", "", "Configuration Files (*.cfg)");
    if (!sel.isEmpty())
        load_config(sel.toStdString());
}

void MainWindow::save_config_dialog() {
    QString sel = QFileDialog::getSaveFileName(this, "Save Configuration File", "", "Configuration Files (*.cfg)");
    if (!sel.isEmpty())
        save_config(sel.toStdString());
}

void MainWindow::closeEvent(QCloseEvent *ev) {
    if (reader != nullptr) {
        ev->ignore();
    }
}

void MainWindow::load_config(const std::string &filename) {
    using boost::property_tree::ptree;
    ptree pt;

    // parse file
    try {
        read_xml(filename, pt);
    }
    catch (std::exception &e) {
        QMessageBox::information(this, "Error", (std::string("Cannot read config file: ") += e.what()).c_str(), QMessageBox::Ok);
        return;
    }

    // get config values
    try {
        ui.Cap_ID->setCurrentIndex(pt.get<int>("settings.capId", 1));
        ui.samplingRate->setCurrentIndex(pt.get<int>("settings.samplingrate", 2));
        ui.EEG_Range->setCurrentIndex(pt.get<int>("settings.EegRange", 0));
        ui.BIP_Range->setCurrentIndex(pt.get<int>("settings.BipRange", 2));

        std::string tmp = pt.get<std::string>("settings.hexEegMask", DEFAULT_EEG_MASK);
        ui.EEG_LineEdit->setText(QString::fromStdString(tmp));
        tmp = pt.get<std::string>("settings.hexBipMask", DEFAULT_BIP_MASK);
        ui.BIP_LineEdit->setText(QString::fromStdString(tmp));
    }
    catch (std::exception &) {
        QMessageBox::information(this, "Error in Config File", "Could not read out config parameters.", QMessageBox::Ok);
        return;
    }
}

void MainWindow::save_config(const std::string &filename) {
    using boost::property_tree::ptree;
    ptree pt;

    // transfer UI content into property tree
    try {
        pt.put("settings.capId", ui.Cap_ID->currentIndex());
        pt.put("settings.samplingrate", ui.samplingRate->currentIndex());
        pt.put("settings.EegRange", ui.EEG_Range->currentIndex());
        pt.put("settings.BipRange", ui.BIP_Range->currentIndex());

        QString tmp = ui.EEG_LineEdit->text();
        pt.put("settings.hexEegMask", tmp.toStdString());
        tmp = ui.BIP_LineEdit->text();
        pt.put("settings.hexBipMask", tmp.toStdString());

    }
    catch (std::exception &e) {
        QMessageBox::critical(this, "Error", (std::string("Could not prepare settings for saving: ") += e.what()).c_str(), QMessageBox::Ok);
    }

    // write to disk
    try {
        write_xml(filename, pt);
    }
    catch (std::exception &e) {
        QMessageBox::critical(this, "Error", (std::string("Could not write to config file: ") += e.what()).c_str(), QMessageBox::Ok);
    }
}

void MainWindow::link() {
    if (reader != nullptr) {
        reader->setStop(true);
        ui.linkButton->setText("Link");
    }
    else {
        // === perform link action ===
        try {
            // get the UI parameters...
            QString tmpParam;

            // CapId
            int capId = ui.Cap_ID->currentIndex();

            // Samping rate
            tmpParam = ui.samplingRate->currentText();
            int samplingRate = tmpParam.toInt();

            // Eeg channels Range
            tmpParam = ui.EEG_Range->currentText();
            double EegRange = tmpParam.toDouble()/ 1000;

            // BIP channels range
            tmpParam = ui.BIP_Range->currentText();
            double BipRange = tmpParam.toDouble() / 1000;

            // EEG / BIP channel masks (arbitrary length, so >64 channels are supported)
            std::vector<bool> eegMask, bipMask;
            if (!parseHexMask(ui.EEG_LineEdit->text().toStdString(), eegMask)) {
                QMessageBox::critical(this, "Error", "Invalid EEG channel mask: it must be a hexadecimal number (e.g. 0xFFFFFFFF).", QMessageBox::Ok);
                return;
            }
            if (!parseHexMask(ui.BIP_LineEdit->text().toStdString(), bipMask)) {
                QMessageBox::critical(this, "Error", "Invalid BIP channel mask: it must be a hexadecimal number (e.g. 0x000000).", QMessageBox::Ok);
                return;
            }

            thread = new QThread();
            reader = new Reader();

            reader->setParams(layoutForCap(capId), samplingRate, BipRange, EegRange, eegMask, bipMask);

            reader->moveToThread(thread);
            connect(thread, SIGNAL(started()), reader, SLOT(read()));

            connect(reader, SIGNAL(finished()), thread, SLOT(quit()));
            connect(thread, SIGNAL(finished()), this, SLOT(threadFinished()));
            connect(reader, SIGNAL(finished()), reader, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(reader, SIGNAL(timeout()), this, SLOT(threadTimeout()));
            connect(reader, SIGNAL(connectionLost()), this, SLOT(connectionLost()));
            connect(reader, SIGNAL(ampNotFound()), this, SLOT(ampNotFound()));
            connect(reader, SIGNAL(unknownError()), this, SLOT(unknownError()));
            connect(reader, SIGNAL(errorMessage(QString)), this, SLOT(errorMessage(QString)));
            thread->start();
        }
        catch (std::exception &e) {
            // try to decode the error message
            std::string msg = "Could not query driver message because the device is not open";
            QMessageBox::critical(this, "Error", ("Could not initialize the eego Sport interface: " + (e.what() + (" (driver message: " + msg + ")"))).c_str(), QMessageBox::Ok);
            return;
        }

        // done, all successful
        ui.linkButton->setText("Unlink");
    }
}

void MainWindow::threadFinished() {
    reader = nullptr;
    thread = nullptr; // deleted through deleteLater()
    ui.linkButton->setText("Link");
}

void MainWindow::threadTimeout() {
    QMessageBox::critical(this, "Error", (std::string("Error: eego Sport Conncetion timed out")).c_str(), QMessageBox::Ok);
}

void MainWindow::ampNotFound() {
    QMessageBox::critical(this, "Error", (std::string("Error: Amp not found or license file not present.\nPlease connect the amplifier and make sure that the amp is turned on.")).c_str(), QMessageBox::Ok);
}

void MainWindow::connectionLost() {
    QMessageBox::critical(this, "Error", (std::string("Error: Amp connection lost")).c_str(), QMessageBox::Ok);
}

void MainWindow::unknownError() {
    QMessageBox::critical(this, "Error", (std::string("Error: Unknown")).c_str(), QMessageBox::Ok);
}

void MainWindow::errorMessage(const QString &message) {
    QMessageBox::critical(this, "Error", "Error: " + message, QMessageBox::Ok);
}

// --- CONSTRUCTOR ---
Reader::Reader() {
    stop = false;
    amp = nullptr;
    eegStream = nullptr;
    samplingRate = 0;
    EegRange = 0;
    BipRange = 0;
}

// --- DECONSTRUCTOR ---
Reader::~Reader() {
    // free resources
}

void Reader::setParams(const std::vector<std::string> &capLayout, int samplingRate, double BipRange, double EegRange, const std::vector<bool> &eegMask, const std::vector<bool> &bipMask) {
    this->capLayout = capLayout;
    this->samplingRate = samplingRate;
    this->BipRange = BipRange;
    this->EegRange = EegRange;
    this->eegMask = eegMask;
    this->bipMask = bipMask;
}

// Sorting the amplifiers according to their serial number. First amplifier to connect should be the one with the lowest SN.
template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {

    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
         [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

    return idx;
}

void Reader::OpenEegStream() {
    std::vector<eemagine::sdk::amplifier*> amps;
    std::vector<std::string> serialTmp;
    amps = fact.getAmplifiers();

    // Check if several amplifiers connected (128Ch)
    if ( amps.size() > 1 ) {

        for (unsigned int i = 0; i < amps.size(); ++i)
            serialTmp.push_back(amps.at(i)->getSerialNumber());
        std::vector<eemagine::sdk::amplifier*>  tmpAmps;
        for (auto i : sort_indexes(serialTmp)) {
            tmpAmps.push_back(amps.at(i));
        }
        amp = fact.createCascadedAmplifier(tmpAmps);
        // or only one
    } else if(amps.size() == 1 )
        amp = amps.at(0);
    else
        throw(exceptions::notFound("No amp found"));

    if (!amp)
        throw(exceptions::notFound("No amp found"));

    // Build the channel list from the masks ourselves: the SDK convenience
    // overload only takes 64-bit masks, which cannot address the 128 EEG /
    // 48 BIP channels of cascaded amplifiers.
    eegLabels.clear();
    bipLabels.clear();
    std::vector<channel> selected;
    size_t refCounter = 0;
    size_t bipCounter = 0;
    for (const channel &ch : amp->getChannelList()) {
        switch (ch.getType()) {
        case channel::reference:
            if (maskBit(eegMask, refCounter)) {
                selected.push_back(ch);
                eegLabels.push_back(labelAt(capLayout, refCounter, "Ch"));
            }
            ++refCounter;
            break;
        case channel::bipolar:
            if (maskBit(bipMask, bipCounter)) {
                selected.push_back(ch);
                bipLabels.push_back("AUX" + std::to_string(bipCounter + 1));
            }
            ++bipCounter;
            break;
        default:
            break;
        }
    }
    std::cout << "eegoSports: amplifier exposes " << refCounter << " EEG and " << bipCounter
              << " BIP channels; streaming " << eegLabels.size() << " EEG and " << bipLabels.size()
              << " BIP channels." << std::endl;
    if (selected.empty())
        throw std::runtime_error("The EEG/BIP masks do not select any channel.");

    // EEG stream's instanciation
    eegStream = amp->OpenEegStream(samplingRate, EegRange, BipRange, selected);
}

// One column of the LSL data stream
struct OutputChannel {
    unsigned int bufferIndex; // column in the SDK buffer
    double scale;             // V -> uV for EEG/AUX, 1 for triggers
};

// --- PROCESS ---
// Start processing data.
void Reader::read() {
    try {

        OpenEegStream();
        const std::vector<channel> channelList = eegStream->getChannelList();

        std::vector<OutputChannel> outputs;

        // Map every SDK channel to an LSL channel, based on its type rather than
        // its position (cascaded amplifiers may expose several trigger / sample
        // counter channels). Sample counters are not streamed.
        size_t eegIdx = 0, bipIdx = 0, trgIdx = 0;
        int triggerColumn = -1;
        struct Desc { std::string label, type, unit; };
        std::vector<Desc> descs;
        for (unsigned int i = 0; i < channelList.size(); i++) {
            switch (channelList[i].getType()) {
            case channel::reference:
                descs.push_back({labelAt(eegLabels, eegIdx++, "Ch"), "EEG", "uV"});
                outputs.push_back({i, 1e6});
                break;
            case channel::bipolar:
                descs.push_back({labelAt(bipLabels, bipIdx++, "AUX"), "AUX", "uV"});
                outputs.push_back({i, 1e6});
                break;
            case channel::trigger:
                descs.push_back({trgIdx == 0 ? std::string("TRIGGER") : "TRIGGER" + std::to_string(trgIdx + 1), "Markers", "[]"});
                outputs.push_back({i, 1.0});
                if (triggerColumn < 0)
                    triggerColumn = static_cast<int>(i);
                ++trgIdx;
                break;
            case channel::sample_counter:
                break;
            default:
                descs.push_back({"Misc" + std::to_string(i + 1), "Misc", "[]"});
                outputs.push_back({i, 1.0});
                break;
            }
        }

        // create data streaminfo and append some meta-data
        lsl::stream_info data_info("eegoSports " + amp->getSerialNumber(), "EEG", static_cast<int32_t>(outputs.size()), samplingRate, lsl::cf_double64, "eegoSports_" + amp->getSerialNumber());
        lsl::xml_element channels = data_info.desc().append_child("channels");
        for (const Desc &d : descs) {
            channels.append_child("channel")
                    .append_child_value("label", d.label)
                    .append_child_value("type", d.type)
                    .append_child_value("unit", d.unit);
        }

        data_info.desc().append_child("acquisition")
                .append_child_value("manufacturer", "antneuro")
            .append_child_value("serial_number", amp->getSerialNumber().c_str());

        // make a data outlet
        lsl::stream_outlet data_outlet(data_info);

        // create marker streaminfo and outlet
        lsl::stream_info marker_info("eegoSports-" + amp->getSerialNumber() + "_markers", "Markers", 1, 0, lsl::cf_string, "eegoSports_" + amp->getSerialNumber() + "_markers");
        lsl::stream_outlet marker_outlet(marker_info);

        double last_mrk = 0;
        while (!stop) {

            std::this_thread::sleep_for(std::chrono::milliseconds(8));

            buffer = eegStream->getData();
            const unsigned int channelCount = buffer.getChannelCount();
            const unsigned int sampleCount = buffer.getSampleCount();
            if (sampleCount == 0)
                continue;
            if (channelCount != channelList.size())
                throw std::runtime_error("Unexpected number of channels in the eego buffer: " + std::to_string(channelCount)
                                         + " (expected " + std::to_string(channelList.size()) + ")");

            // stream data in uV (better than small values in V) except triggers
            std::vector<std::vector<double>> send_buffer(sampleCount, std::vector<double>(outputs.size()));
            for (unsigned int s = 0; s < sampleCount; s++) {
                for (size_t c = 0; c < outputs.size(); c++)
                    send_buffer[s][c] = buffer.getSample(outputs[c].bufferIndex, s) * outputs[c].scale;
            }

            // 'now' is the timestamp of the most recent sample of the chunk
            double now = lsl::local_clock();
            data_outlet.push_chunk(send_buffer, now);

            if (triggerColumn >= 0) {
                for (unsigned int s = 0; s < sampleCount; s++) {
                    double mrk = buffer.getSample(static_cast<unsigned int>(triggerColumn), s);
                    if (mrk != last_mrk) {
                        std::string mrk_string = std::to_string(mrk);
                        double ts = now - static_cast<double>(sampleCount - 1 - s) / samplingRate;
                        marker_outlet.push_sample(&mrk_string, ts);
                        last_mrk = mrk;
                    }
                }
            }
        }
    }
    catch (const exceptions::notFound &) {
        emit ampNotFound();
    }
    catch (const exceptions::notConnected &) {
        emit connectionLost();
    }
    catch (const exceptions::unknown &) {
        emit unknownError();
    }
    catch (const std::exception &e) {
        std::cout<<e.what()<<std::endl;
        emit errorMessage(QString::fromStdString(e.what()));
    }
    delete eegStream;
    delete amp;
    eegStream = nullptr;
    amp = nullptr;
    emit finished();
}

MainWindow::~MainWindow() {
}
