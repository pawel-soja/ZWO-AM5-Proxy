#include "hostinfo.h"
#include <QNetworkInterface>

// Returns the IP address of the device on which the program is running
QHostAddress HostInfo::ip()
{
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
            return address;
        }
    }
    return QHostAddress();
}

QHostAddress HostInfo::broadcast()
{
    return broadcast(ip());
}

// Finds the broadcast address of the network card with the given IP address.
QHostAddress HostInfo::broadcast(const QHostAddress &address)
{
    for (const auto &interfaces: QNetworkInterface::allInterfaces())
    {
        for (const auto &entry: interfaces.addressEntries())
        {
            if (entry.ip() == address)
            {
                return entry.broadcast();
            }
        }
    }
    return QHostAddress();
}
