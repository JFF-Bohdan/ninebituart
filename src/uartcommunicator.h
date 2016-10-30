#ifndef UARTCOMMUNICATOR_H
#define UARTCOMMUNICATOR_H

#include <QtSerialPort/QtSerialPort>
#include <QtCore/QtCore>
#include <QtSerialPort/QSerialPort>


class ANineBitSenderBase : public QObject
{
    Q_OBJECT
public:
    ANineBitSenderBase(QSerialPort* port = Q_NULLPTR, QObject *parent = Q_NULLPTR);

    virtual ~ANineBitSenderBase();

    //! attaches port to processor
    virtual void setPort(QSerialPort* port);

    //! returns attached port
    QSerialPort* port() const;

    //! sets default (non-marked) transmission mode
    virtual void setDefaultTransmissionMode(const QSerialPort::Parity value);

    //! returns marked transmission mode
    QSerialPort::Parity defaultTransmissionMode() const;

    //! enables/disables marking
    virtual bool setMarkingEnabled(const bool value);

    //! returns marked transmission parity
    QSerialPort::Parity markedTransmissionMode() const;

    //! sets marked transmission parity
    virtual void setMarkedTransmissionMode(const QSerialPort::Parity value);

    //! writes data to attached port
    virtual bool writeData(const char* data, const size_t size) = 0;

    /*!
     * \brief readData reads data from port
     * \param size needful data size
     * \param maxMSecs max wait time (in msecs)
     * \param ok pointer where will be status of operation (can be NULL), when operations succeded here will be true, otherwise false
     * \return read data
     */
    virtual QByteArray readData(const size_t size, const int maxMSecs, bool *ok = Q_NULLPTR);
protected:
    mutable QSerialPort         *m_Port = Q_NULLPTR;
    QSerialPort::Parity         m_MarkParity = QSerialPort::MarkParity;
    QSerialPort::Parity         m_DefaultParity = QSerialPort::SpaceParity;
    bool                        m_MarkingEnabled = false;

    void setBool(bool *ptr, const bool value);
};


class AHardwareNineBitSender : public ANineBitSenderBase
{
    Q_OBJECT

public:
    AHardwareNineBitSender(QSerialPort* port = Q_NULLPTR, QObject *parent = Q_NULLPTR);

    virtual ~AHardwareNineBitSender() Q_DECL_OVERRIDE;

    virtual bool setMarkingEnabled(const bool value) Q_DECL_OVERRIDE;

    virtual bool writeData(const char* data, const size_t size) Q_DECL_OVERRIDE;
};

class ASoftwareNineBitSender : public ANineBitSenderBase
{
    Q_OBJECT

public:
    ASoftwareNineBitSender(QSerialPort* port = Q_NULLPTR, QObject *parent = Q_NULLPTR);

    virtual ~ASoftwareNineBitSender() Q_DECL_OVERRIDE;

    virtual bool writeData(const char* data, const size_t size) Q_DECL_OVERRIDE;

    static bool tests();
private:
    static short int setBitsCount(const unsigned char value);

    QSerialPort::Parity needfulParity(const char ch) const;

    bool updateParityMode(const char ch);
};


#endif //UARTCOMMUNICATOR_H
