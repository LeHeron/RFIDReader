#include "rfidlogger.h"

RFIDLogger::RFIDLogger(QObject *parent)
    : QObject{parent}
{
    m_handler_thread = new QThread(this);
    m_handler = new RFIDListener();
    m_handler->moveToThread(m_handler_thread);

    QObject::connect(m_handler,        &RFIDListener::rcvTag,    this,               &RFIDLogger::sltRcvTag);
    QObject::connect(m_handler_thread, &QThread::started,       m_handler,          &RFIDListener::init);
    QObject::connect(m_handler_thread, &QThread::finished,      m_handler_thread,   &QThread::deleteLater);

    // Single connection to trigger polling at start
    QMetaObject::Connection * const connection = new QMetaObject::Connection;
    *connection = connect(m_handler, &RFIDListener::initFinished, m_handler, [this, connection](){
        m_handler->startPolling();
        QObject::disconnect(*connection);
        delete connection;
    });

    m_handler_thread->start();
}

RFIDLogger::~RFIDLogger()
{
    delete m_handler;
    delete m_handler_thread;
}

static void print_hex(QDebug& dbg, const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    dbg << ("%02x  ", pbtData[szPos]);
  }
  dbg << ("\n");
}

void RFIDLogger::printInfo(nfc_target *nt)
{
    QDebug dbg = qDebug().noquote().nospace();
    switch (nt->nm.nmt) {
    case NMT_ISO14443A:
        dbg << "The following (NFC) ISO14443A tag was found:\n";
        break;
    case NMT_ISO14443B:
        dbg << "The following (NFC) ISO14443B tag was found:\n";
        return; //break;
    case NMT_ISO14443BI:
        dbg << "The following (NFC) ISO14443BI tag was found:\n";
        return; //break;
    case NMT_ISO14443B2SR:
        dbg << "The following (NFC) ISO14443B2SR tag was found:\n";
        return; //break;
    case NMT_ISO14443B2CT:
        dbg << "The following (NFC) ISO14443B2CT tag was found:\n";
        return; //break;
    case NMT_FELICA:
        dbg << "The following (NFC) FELICA tag was found:\n";
        return; //break;
    case NMT_JEWEL:
        dbg << "The following (NFC) JEWEL tag was found:\n";
        return; //break;
    case NMT_DEP:
        dbg << "The following (NFC) DEP tag was found:\n";
        return; //break;
    default:
        break;
    }

    dbg << "    ATQA (SENS_RES): ";
    print_hex(dbg, nt->nti.nai.abtAtqa, 2);
    dbg << "       UID (NFCID" << (nt->nti.nai.abtUid[0] == 0x08 ? '3' : '1') << "): ";
    print_hex(dbg, nt->nti.nai.abtUid, nt->nti.nai.szUidLen);
    dbg << "      SAK (SEL_RES): ";
    print_hex(dbg, &nt->nti.nai.btSak, 1);
    if (nt->nti.nai.szAtsLen) {
        dbg << "          ATS (ATR): ";
        print_hex(dbg, nt->nti.nai.abtAts, nt->nti.nai.szAtsLen);
    }
}

void RFIDLogger::sltRcvTag(std::shared_ptr<nfc_target> nt)
{
    //printInfo(nt.get());
    if (nt->nm.nmt != NMT_ISO14443A) {
        return;
    }

    uint64_t uid = 0;
    for (int szPos = 0; szPos < nt->nti.nai.szUidLen; szPos++) {
        uid <<= 8;
        uid += nt->nti.nai.abtUid[szPos];
    }
    switch (uid) {
    case 1459085183:
        qDebug() << "Coffe key logged";
        break;
    case 174692409:
        qDebug() << "DOGA badge logged";
        break;
    case 1885304799:
        qDebug() << "Home badge logged";
        break;
    case 16909060:
        qDebug() << "OnePlus 7T logged";
        break;
    default:
        break;
    }
}
