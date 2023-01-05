#include "rfidlistener.h"
#define CHECK_INIT if (!m_isInit || !m_readerDevice) return;

RFIDListener::RFIDListener(QObject* parent)
    : QObject(parent), m_isInit{false}, m_readerDevice{nullptr}
{
}

RFIDListener::~RFIDListener()
{
    delete m_timer;
    qDebug() << "Its ok";
    nfc_close(m_readerDevice);
    if (m_isInit)
        nfc_exit(m_ctx);
}

void RFIDListener::init()
{
    m_timer = new QTimer(this);
    nfc_init(&m_ctx);
    if (m_ctx) {
        m_isInit = true;
        m_readerDevice = nfc_open(m_ctx, NULL);
        QObject::connect(m_timer, &QTimer::timeout, this, &RFIDListener::poll);
        emit initFinished();
    } else {
        qDebug() << "Nfc init failed";
    }
}

static void print_hex(QDebug& dbg, const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    dbg << ("%02x  ", pbtData[szPos]);
  }
  dbg << ("\n");
}

void RFIDListener::startPolling()
{
    CHECK_INIT

    if (nfc_initiator_init(m_readerDevice) < 0) {
        qDebug() << "Cannot init initiator mode (reader)";
        return;
    }

    m_polling = true;
    m_timer->start(POLL_DELAY);
    qDebug() << "Start polling !";
}

void RFIDListener::stopPolling()
{
    CHECK_INIT
    m_timer->stop();
    nfc_abort_command(m_readerDevice);
    m_polling = false;
}

void RFIDListener::printInfo(nfc_target *nt)
{
    QDebug dbg = qDebug().noquote().nospace();
    dbg << "The following (NFC) ISO14443A tag was found:\n";
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

void RFIDListener::poll()
{
    m_timer->stop();
    constexpr nfc_modulation nmModulations[5] = {
        { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443B, .nbr = NBR_106 },
        { .nmt = NMT_FELICA, .nbr = NBR_212 },
        { .nmt = NMT_FELICA, .nbr = NBR_424 },
        { .nmt = NMT_JEWEL, .nbr = NBR_106 },
    };
    constexpr size_t szModulations = 5;

    nfc_target nt;
    int err = nfc_initiator_poll_target(m_readerDevice, nmModulations, szModulations, 20, 2, &nt);

    if (err < 0 && err != -6) { // Error
        nfc_perror(m_readerDevice, "nfc_initiator_poll_target");
        return;
    }

    if (err == -6 || err == 0) { // Timeout
        m_timer->start(POLL_DELAY);
        return;
    }
    qRegisterMetaType<std::shared_ptr<nfc_target>>();
    emit rcvTag(std::make_shared<nfc_target>(nt));
    while (nfc_initiator_target_is_present(m_readerDevice, nullptr) == 0);
    nfc_perror(m_readerDevice, "nfc_initiator_target_is_present");
    m_timer->start(POLL_DELAY);
}
