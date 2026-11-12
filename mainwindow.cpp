#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace eemagine::sdk;

//---------------------------------------------------------------------------
// Layouts
//---------------------------------------------------------------------------
// Default BIP selection (saved as a hexadecimal mask in configuration files): none.
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

// NA-265 (64 ch, 10-10 layout, REF at Cz/CPz, GND at AFz/Fpz)
// Channel order from the datasheet pinning table (DRN-15891 rev. 5):
// channels 1-32 on connector 1, 33-64 on connector 2.
static const std::vector<std::string> layout_NA265 = {
    "Fp1",     "Fp2",     "F9",       "F7",       "F3",       "Fz",       "F4",
    "F8",      "F10",     "FC5",      "FC1",      "FC2",      "FC6",      "T9",
    "T7",      "C3",      "C4",       "T8",       "T10",      "CP5",      "CP1",
    "CP2",     "CP6",     "P9",       "P7",       "P3",       "Pz",       "P4",
    "P8",      "P10",     "O1",       "O2",
    "AF7",     "AF3",     "AF4",      "AF8",      "F5",       "F1",       "F2",
    "F6",      "FC3",     "FCz",      "FC4",      "C5",       "C1",       "C2",
    "C6",      "CP3",     "CP4",      "P5",       "P1",       "P2",       "P6",
    "PO5",     "PO3",     "PO4",      "PO6",      "FT7",      "FT8",      "TP7",
    "TP8",     "PO7",     "PO8",      "POz"
};

// Caps, in the order of the "CAP ID" combo box. The name is what is saved in
// configuration files, so that the combo box can be reordered safely.
struct CapInfo {
    const char *name;
    const std::vector<std::string> *labels; // EEG channel labels, in amplifier order
};
static const std::vector<CapInfo> caps = {
    {"CA-203", &layout_CA203},
    {"CA-208", &layout_CA208},
    {"NA-265", &layout_NA265},
};
static const int DEFAULT_CAP_ID = 1; // CA-208 (64 ch)

static const CapInfo &capInfo(int capId) {
    if (capId < 0 || capId >= static_cast<int>(caps.size()))
        capId = DEFAULT_CAP_ID;
    return caps[static_cast<size_t>(capId)];
}

// One eego amplifier has 24 bipolar (AUX) inputs.
static const int BIP_CHANNEL_COUNT = 24;

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

// Format the first nbits of a mask as a hexadecimal string with exactly ceil(nbits / 4) digits.
static std::string formatHexMask(const std::vector<bool> &bits, unsigned int nbits) {
    static const char hexDigits[] = "0123456789ABCDEF";
    std::string text = "0x";
    const int digits = static_cast<int>((nbits + 3) / 4);
    for (int d = digits - 1; d >= 0; d--) {
        int value = 0;
        for (int b = 0; b < 4; b++) {
            const unsigned int k = static_cast<unsigned int>(d * 4 + b);
            if (k < nbits && maskBit(bits, k))
                value |= 1 << b;
        }
        text += hexDigits[value];
    }
    return text;
}


MainWindow::MainWindow(QWidget *parent, const std::string &config_file, const bool linkOnStart) : QMainWindow(parent)
{
    reader = nullptr;
    thread = nullptr;
    ui.setupUi(this);

    // Caps
    ui.Cap_ID->clear();
    for (const CapInfo &cap : caps)
        ui.Cap_ID->addItem(QString::fromLatin1(cap.name) + QString("  (%1 ch)").arg(static_cast<int>(cap.labels->size())));

    // BIP channels: one checkable entry per AUX input
    ui.BIP_List->clear();
    for (int k = 1; k <= BIP_CHANNEL_COUNT; k++) {
        QListWidgetItem *item = new QListWidgetItem(QString("AUX%1").arg(k), ui.BIP_List);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    // Init initial indexes
    if(!config_file.empty()) {
        load_config(config_file);
    } else {
        ui.Cap_ID->setCurrentIndex(DEFAULT_CAP_ID);
        ui.samplingRate->setCurrentIndex(1);
        ui.EEG_Range->setCurrentIndex(0);
        ui.BIP_Range->setCurrentIndex(2);
    }


    // make GUI connections
    QObject::connect(ui.BIP_All, SIGNAL(clicked()), this, SLOT(selectAllBip()));
    QObject::connect(ui.BIP_None, SIGNAL(clicked()), this, SLOT(selectNoBip()));
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
        // The cap is saved by name. Older files only have the combo box index: of
        // those, only CA-203 (0) and CA-208 (1) still exist, others use the default cap.
        int capId = DEFAULT_CAP_ID;
        const std::string capName = pt.get<std::string>("settings.cap", "");
        if (!capName.empty()) {
            for (size_t k = 0; k < caps.size(); k++) {
                if (capName == caps[k].name)
                    capId = static_cast<int>(k);
            }
        } else {
            const int legacyId = pt.get<int>("settings.capId", DEFAULT_CAP_ID);
            if (legacyId == 0 || legacyId == 1)
                capId = legacyId;
        }
        ui.Cap_ID->setCurrentIndex(capId);
        ui.samplingRate->setCurrentIndex(pt.get<int>("settings.samplingrate", 2));
        ui.EEG_Range->setCurrentIndex(pt.get<int>("settings.EegRange", 0));
        ui.BIP_Range->setCurrentIndex(pt.get<int>("settings.BipRange", 2));

        // BIP selection, saved as a hexadecimal mask (bit 0 = AUX1)
        std::vector<bool> bipMask;
        if (!parseHexMask(pt.get<std::string>("settings.hexBipMask", DEFAULT_BIP_MASK), bipMask))
            bipMask.clear();
        setBipSelection(bipMask);
    }
    catch (std::exception &) {
        QMessageBox::information(this, "Error in Config File", "Could not read out config parameters.", QMessageBox::Ok);
        return;
    }
}

std::vector<bool> MainWindow::bipSelection() const {
    std::vector<bool> mask(static_cast<size_t>(ui.BIP_List->count()), false);
    for (int k = 0; k < ui.BIP_List->count(); k++)
        mask[static_cast<size_t>(k)] = ui.BIP_List->item(k)->checkState() == Qt::Checked;
    return mask;
}

void MainWindow::setBipSelection(const std::vector<bool> &mask) {
    for (int k = 0; k < ui.BIP_List->count(); k++)
        ui.BIP_List->item(k)->setCheckState(maskBit(mask, static_cast<size_t>(k)) ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::selectAllBip() {
    setBipSelection(std::vector<bool>(BIP_CHANNEL_COUNT, true));
}

void MainWindow::selectNoBip() {
    setBipSelection(std::vector<bool>());
}

void MainWindow::save_config(const std::string &filename) {
    using boost::property_tree::ptree;
    ptree pt;

    // transfer UI content into property tree
    try {
        pt.put("settings.cap", std::string(capInfo(ui.Cap_ID->currentIndex()).name));
        pt.put("settings.samplingrate", ui.samplingRate->currentIndex());
        pt.put("settings.EegRange", ui.EEG_Range->currentIndex());
        pt.put("settings.BipRange", ui.BIP_Range->currentIndex());
        pt.put("settings.hexBipMask", formatHexMask(bipSelection(), BIP_CHANNEL_COUNT));

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

            // All the EEG channels of the cap are streamed, plus the checked BIP channels
            thread = new QThread();
            reader = new Reader();

            reader->setParams(*capInfo(capId).labels, samplingRate, BipRange, EegRange, bipSelection());

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

        // done, all successful: freeze the parameters while linked
        setSettingsEnabled(false);
        ui.linkButton->setText("Unlink");
    }
}

void MainWindow::threadFinished() {
    reader = nullptr;
    thread = nullptr; // deleted through deleteLater()
    ui.linkButton->setText("Link");
    setSettingsEnabled(true);
}

void MainWindow::setSettingsEnabled(bool enabled) {
    ui.groupBox->setEnabled(enabled);
    ui.actionLoad_Configuration->setEnabled(enabled);
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

void Reader::setParams(const std::vector<std::string> &capLayout, int samplingRate, double BipRange, double EegRange, const std::vector<bool> &bipMask) {
    this->capLayout = capLayout;
    this->samplingRate = samplingRate;
    this->BipRange = BipRange;
    this->EegRange = EegRange;
    this->bipMask = bipMask;
}

void Reader::OpenEegStream() {
    std::vector<eemagine::sdk::amplifier*> amps = fact.getAmplifiers();
    if (amps.empty())
        throw(exceptions::notFound("No amp found"));

    // A single amplifier is used. If several are connected, take the one with the
    // lowest serial number and release the others.
    std::sort(amps.begin(), amps.end(), [](amplifier *a, amplifier *b) {
        return a->getSerialNumber() < b->getSerialNumber();
    });
    amp = amps.front();
    for (size_t k = 1; k < amps.size(); k++)
        delete amps[k];
    if (amps.size() > 1)
        std::cout << "eegoSports: " << amps.size() << " amplifiers connected, using " << amp->getSerialNumber() << std::endl;

    // The amplifier is always opened with ALL its EEG and BIP channels; the channel
    // selection is applied in software when building the LSL stream (see read()).
    // (The eego SDK mishandles channel subsets with cascaded amplifiers, so the
    // full channel list is the safest way to open a stream.)
    // eegLabels / bipLabels hold one entry per amplifier channel; an empty label
    // means that the channel is not streamed.
    eegLabels.clear();
    bipLabels.clear();
    std::vector<channel> allChannels;
    size_t eegStreamed = 0;
    size_t bipStreamed = 0;
    for (const channel &ch : amp->getChannelList()) {
        switch (ch.getType()) {
        case channel::reference: {
            const size_t k = eegLabels.size();
            // All the electrodes of the selected cap are streamed (e.g. 32 for a
            // CA-203 on a 64 ch amplifier), the other inputs are dropped.
            const bool streamed = k < capLayout.size();
            eegLabels.push_back(streamed ? labelAt(capLayout, k, "Ch") : std::string());
            eegStreamed += streamed ? 1 : 0;
            allChannels.push_back(ch);
            break;
        }
        case channel::bipolar: {
            const size_t k = bipLabels.size();
            const bool streamed = maskBit(bipMask, k);
            bipLabels.push_back(streamed ? "AUX" + std::to_string(k + 1) : std::string());
            bipStreamed += streamed ? 1 : 0;
            allChannels.push_back(ch);
            break;
        }
        default:
            break;
        }
    }
    std::cout << "eegoSports: amplifier exposes " << eegLabels.size() << " EEG and " << bipLabels.size()
              << " BIP channels; streaming " << eegStreamed << " EEG and " << bipStreamed
              << " BIP channels." << std::endl;
    if (eegLabels.size() < capLayout.size())
        throw std::runtime_error("The selected cap has " + std::to_string(capLayout.size())
                                 + " EEG channels but the connected amplifier only provides "
                                 + std::to_string(eegLabels.size()) + ".");

    // EEG stream's instanciation
    eegStream = amp->OpenEegStream(samplingRate, EegRange, BipRange, allChannels);
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

        // Map SDK channels to LSL channels, based on their type rather than their
        // position. EEG/BIP channels not selected by the masks (empty label) and
        // sample counters are not streamed.
        size_t eegIdx = 0, bipIdx = 0, trgIdx = 0;
        size_t eegOut = 0, bipOut = 0;
        int triggerColumn = -1;
        struct Desc { std::string label, type, unit; };
        std::vector<Desc> descs;
        for (unsigned int i = 0; i < channelList.size(); i++) {
            switch (channelList[i].getType()) {
            case channel::reference: {
                const size_t k = eegIdx++;
                if (k < eegLabels.size() && !eegLabels[k].empty()) {
                    descs.push_back({eegLabels[k], "EEG", "uV"});
                    outputs.push_back({i, 1e6});
                    ++eegOut;
                }
                break;
            }
            case channel::bipolar: {
                const size_t k = bipIdx++;
                if (k < bipLabels.size() && !bipLabels[k].empty()) {
                    descs.push_back({bipLabels[k], "AUX", "uV"});
                    outputs.push_back({i, 1e6});
                    ++bipOut;
                }
                break;
            }
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

        std::cout << "eegoSports: LSL stream has " << eegOut << " EEG, " << bipOut << " BIP and "
                  << trgIdx << " trigger channel(s)." << std::endl;

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
