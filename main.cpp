/*!
 * \file %{Cpp:License:FileName}
 * \brief file for the definition of the class "%{Cpp:License:ClassName}"
 * \author poles
 * \date 2022-11-3
 *
 * \details
 *
 * \copyright
 ****************************************************************************
 *        Ce logiciel est la propriete de DOGA®.
 *         -------------------------------------
 *
 *    Il ne peut etre reproduit ni communique en totalite ou partie sans
 *    son autorisation ecrite.
 *
 ****************************************************************************
 *        This software is the property of DOGA®.
 *         -------------------------------------
 *
 *    It cannot be reproduced nor disclosed entirely or partially without
 *    a written agreement.
 *
 ****************************************************************************
 */
#include <QCoreApplication>
#include <csignal>
#include <functional>

#include "rfidlogger.h"

std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    shutdown_handler = [&](int sig)->void{
        a.quit();
    };
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    //RFIDHandler& handler = RFIDHandler::GetInstance();
    //handler.startPolling();

    RFIDLogger logger;

    return a.exec();
}
