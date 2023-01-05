#ifndef RFIDLOGGER_H
#define RFIDLOGGER_H

#include "rfidlistener.h"

#include <memory>
#include <nfc/nfc-types.h>
#include <QObject>
#include <QThread>

class RFIDLogger : public QObject
{
    Q_OBJECT
public:
    explicit RFIDLogger(QObject* parent = nullptr);
    ~RFIDLogger();
    static void printInfo(nfc_target *nt);

private slots:
    void    sltRcvTag(std::shared_ptr<nfc_target> nt);
private:
    QThread* m_handler_thread;
    RFIDListener* m_handler;
};

#endif // RFIDLOGGER_H
