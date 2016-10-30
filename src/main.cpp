#include <QCoreApplication>
#include <QtCore/QtCore>

#include <QtCore/QScopedPointer>
#include <QSerialPortInfo>
#include <QSerialPort>

#include "uartcommunicator.h"

#ifdef Q_OS_WIN32
    #define TEST_PORT_NAME "COM15"
#elif defined(Q_OS_LINUX)
    #define TEST_PORT_NAME "/dev/ttyUSB0"
#else
    #pragma message("Please, specify COM/UART port name.")
#endif

QByteArray testPacket(const unsigned char deviceAddress, const unsigned char command) {
    QByteArray  ret;

    ret.resize(4);
    ret.fill(0x00);

    ret[0] = deviceAddress;
    ret[1] = ~deviceAddress;
    ret[2] = command;
    ret[3] = ~command;

    return ret;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    bool ok = false;

    Q_ASSERT(ASoftwareNineBitSender::tests());

    //listing all avail port to output console
    qDebug() << "avail ports:";
    for(const QSerialPortInfo &item : QSerialPortInfo::availablePorts()) {
        qDebug() << item.portName();
    }
    qDebug() << endl;

#ifdef Q_OS_WIN32
    QPointer<ANineBitSenderBase>  processor = QPointer<ANineBitSenderBase>(new AHardwareNineBitSender());
#elif defined(Q_OS_LINUX)
    QPointer<ANineBitSenderBase>  processor = QPointer<ANineBitSenderBase>(new ASoftwareNineBitSender());
#else
    #pragma message("Please, select needful class.")
#endif

    QPointer<QSerialPort>         port = QPointer<QSerialPort>(new QSerialPort(processor));

    //opening port
    qDebug() << "opening port: " << TEST_PORT_NAME;
    port->setPortName(QLatin1Literal(TEST_PORT_NAME));
    bool openResult = port->open(QIODevice::ReadWrite);

    if(!openResult) {
        qDebug() << "error opening port:" << port->errorString();
        exit(EXIT_FAILURE);
    }

    //*****************************
    //***** initializing port *****
    //*****************************
    port->setBaudRate(QSerialPort::Baud38400);
    port->setBreakEnabled(false);
    port->setDataBits(QSerialPort::Data8);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    //attaching port to 9-bit emulation processor
    processor->setPort(port);

    //compiling packet which must be send to the device
    QByteArray  sndBuff = testPacket(0x06, 0x54);

    processor->setMarkedTransmissionMode(QSerialPort::MarkParity);
    processor->setDefaultTransmissionMode(QSerialPort::SpaceParity);

    //all transmissions after this command will be with marking
    processor->setMarkingEnabled(true);

    //*****************************************************
    //***** transferring address (with 9-bit enabled) *****
    //*****************************************************
    processor->writeData(sndBuff.data(), 1);

    //***************************************************
    //***** transferring data (with 9-bit disabled) *****
    //***************************************************
    processor->setMarkingEnabled(false);

    //writing bytes without arking
    ok = processor->writeData(sndBuff.data() + 1, sndBuff.size() - 1);
    if(!ok) {
        qDebug() << "error writing data to the device: " << processor->port()->errorString();
        return EXIT_FAILURE;
    }

    //reading response from device
    QByteArray response = processor->readData(sndBuff.size() + 1, 2000, &ok);

    if(!ok) {
        qDebug() << "error reading response: " << port->errorString();
        return EXIT_FAILURE;
    }
    qDebug() << "RX: " << QString(QLatin1Literal(response.toHex()));

    port->close();

    qDebug() << "COMPLETE";
    QTimer::singleShot(0, &a, SLOT(quit()));
    return a.exec();
}
