#ifndef RFIDLISTENER_H
#define RFIDLISTENER_H

#include <memory>
#include <nfc/nfc.h>
#include <QDebug>
#include <QList>
#include <QTimer>

#define POLL_DELAY 200

class RFIDListener : public QObject
{
    Q_OBJECT
public:
    RFIDListener(QObject* parent = nullptr);
    ~RFIDListener();

    void                init();
    void                startPolling();
    void                stopPolling();
    static void         printInfo(nfc_target* nt);

private:
    void                poll();

signals:
    void                initFinished();
    void                rcvTag(std::shared_ptr<nfc_target>);

private:
    bool                m_isInit;
    bool                m_polling;
    nfc_context*        m_ctx;
    QTimer*             m_timer;
    nfc_device*         m_readerDevice;
};

Q_DECLARE_METATYPE(std::shared_ptr<nfc_target>)

#endif // RFIDLISTENER_H
