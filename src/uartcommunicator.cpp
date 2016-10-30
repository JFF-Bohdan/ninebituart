#include <QElapsedTimer>
#include <QThread>
#include "uartcommunicator.h"

/*
 ****************** ANineBitSenderBase **********************
 */

void ANineBitSenderBase::setBool(bool *ptr, const bool value)
{
    if(!ptr)
        return;

    *ptr = value;
}

ANineBitSenderBase::ANineBitSenderBase(QSerialPort *port, QObject *parent) :
    QObject(parent),
    m_Port(port)
{
    //

}

ANineBitSenderBase::~ANineBitSenderBase()
{
    //
}

void ANineBitSenderBase::setPort(QSerialPort *port)
{
    m_Port = port;
}

QSerialPort *ANineBitSenderBase::port() const
{
    return m_Port;
}

void ANineBitSenderBase::setDefaultTransmissionMode(const QSerialPort::Parity value)
{
    m_DefaultParity = value;
}

QSerialPort::Parity ANineBitSenderBase::defaultTransmissionMode() const
{
    return m_DefaultParity;
}


bool ANineBitSenderBase::setMarkingEnabled(const bool value)
{
    if(value == m_MarkingEnabled)
        return true;

    m_MarkingEnabled = value;
    return true;
}

QSerialPort::Parity ANineBitSenderBase::markedTransmissionMode() const
{
    return m_MarkParity;
}

void ANineBitSenderBase::setMarkedTransmissionMode(const QSerialPort::Parity value)
{
    m_MarkParity = value;
}

QByteArray ANineBitSenderBase::readData(const size_t size, const int maxMSecs, bool *ok)
{
    QElapsedTimer   timer;
    QByteArray      iret;

    setBool(ok, false);

    timer.start();
    if(!m_Port->bytesAvailable()) {
        if(!m_Port->waitForReadyRead(maxMSecs))
            return QByteArray();
    }

    if(timer.hasExpired(maxMSecs))
        return QByteArray();

    iret.resize(size);
    iret.fill(0x00);

    int     timeLeft = 0;
    size_t  readBytes = 0;

    while(true) {
        timeLeft = (maxMSecs - timer.elapsed());

        if(!m_Port->bytesAvailable()) {
            if(!m_Port->waitForReadyRead(timeLeft))
                return QByteArray(iret.data(), readBytes);
        }

        size_t bytesAvail = m_Port->bytesAvailable();
        size_t bytesNeed = (readBytes + bytesAvail <= size ? bytesAvail : size - readBytes);

        int delta = m_Port->read(iret.data() + readBytes, bytesNeed);

        if(-1 == delta)
            return QByteArray(iret.data(), readBytes);

        readBytes += delta;

        Q_ASSERT(readBytes <= size);

        if(readBytes == size)
            break;

        if( (timer.hasExpired(maxMSecs)) && (readBytes < size) )
            return QByteArray(iret.data(), readBytes);
    }

    setBool(ok, (static_cast<size_t>(iret.size()) == size));

    return QByteArray(iret.data(), readBytes);
}
/*
 ****************** AHardwareNineBitSender **********************
 */

AHardwareNineBitSender::AHardwareNineBitSender(QSerialPort* port, QObject *parent):
    ANineBitSenderBase(port, parent)
{
    //
}

AHardwareNineBitSender::~AHardwareNineBitSender()
{
    //
}

bool AHardwareNineBitSender::setMarkingEnabled(const bool value)
{
    if(!ANineBitSenderBase::setMarkingEnabled(value))
        return false;

#ifdef Q_OS_WIN32
    if(m_MarkingEnabled) {
        if(!m_Port->setParity(m_MarkParity))
            return false;
    } else {
        if(!m_Port->setParity(m_DefaultParity))
            return false;
    }
#endif

    return true;
}

bool AHardwareNineBitSender::writeData(const char *data, const size_t size)
{
    qint64      totalWrittenBytes = 0;
    qint64      writtenBytes = 0;

    while(true) {
        writtenBytes = m_Port->write(data + totalWrittenBytes, size - totalWrittenBytes);

        if(-1 == writtenBytes)
            return false;

        totalWrittenBytes += writtenBytes;

        Q_ASSERT(totalWrittenBytes <= size);

        if(size == totalWrittenBytes)
            break;
    }

    Q_ASSERT(size == totalWrittenBytes);

    qDebug() << "TX:" << QString(QLatin1Literal(QByteArray(data, size).toHex()));

    return true;
}

/*
 ****************** ASoftwareNineBitSender **********************
 */
ASoftwareNineBitSender::ASoftwareNineBitSender(QSerialPort *port, QObject *parent):
    ANineBitSenderBase(port, parent)
{
    //
}

ASoftwareNineBitSender::~ASoftwareNineBitSender()
{
    //
}

bool ASoftwareNineBitSender::writeData(const char *data, const size_t size)
{
    qint64      totalWrittenBytes = 0;
    qint64      writtenBytes = 0;
    char        ch = 0;

    while(true) {
        if(!updateParityMode(ch))
            break;

        ch = data[totalWrittenBytes];

        writtenBytes = m_Port->write(&ch, 1);

        if(-1 == writtenBytes)
            return false;

        totalWrittenBytes += writtenBytes;

        Q_ASSERT(totalWrittenBytes <= size);

        if(size == totalWrittenBytes)
            break;
    }

    Q_ASSERT(size == totalWrittenBytes);

    qDebug() << "TX:" << QString(QLatin1Literal(QByteArray(data, size).toHex()));

    return true;
}

bool ASoftwareNineBitSender::tests()
{
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x00) == 0);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x01) == 1);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x03) == 2);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x07) == 3);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x0F) == 4);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x1F) == 5);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x3F) == 6);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0x7F) == 7);
    Q_ASSERT(ASoftwareNineBitSender::setBitsCount(0xFF) == 8);

    return true;
}

short int ASoftwareNineBitSender::setBitsCount(const unsigned char value)
{
    unsigned char   t = value;
    short int       ret = 0;

    while(t > 0) {
        if (t & 0x01)
            ret++;

        t >>= 1;
    }

    return ret;
}

QSerialPort::Parity ASoftwareNineBitSender::needfulParity(const char ch) const
{
    if(!markedTransmissionMode())
        return defaultTransmissionMode();

    QSerialPort::Parity parity = QSerialPort::NoParity;

    if((ASoftwareNineBitSender::setBitsCount(ch) % 2) != 0) {
        parity = QSerialPort::EvenParity;
    } else {
        parity = QSerialPort::OddParity;
    }

    return parity;
}

bool ASoftwareNineBitSender::updateParityMode(const char ch)
{
    QSerialPort::Parity parity = this->needfulParity(ch);

    if(m_Port->parity() == parity)
        return true;

    qDebug() << "updating parity";
    return m_Port->setParity(parity);
}

