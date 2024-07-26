#include "am5broadcast.h"
#include "hostinfo.h"

#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QJsonDocument>

Am5Broadcast::Am5Broadcast(QObject *parent)
    : QObject(parent)
    , myIpAddress {HostInfo::ip()}
    , myBroadcastAddress {HostInfo::broadcast()}
    , requestSequence {1}
{
    socket.bind(QHostAddress::AnyIPv4, 3333);
    connect(&socket, &QUdpSocket::readyRead, this, [this] {
        while (socket.hasPendingDatagrams())
        {
            QNetworkDatagram datagram = socket.receiveDatagram();

            // Reject all messages that we ourselves send.
            if (datagram.senderAddress() == myIpAddress)
            {
                continue;
            }
            QJsonDocument requestDocument = QJsonDocument::fromJson(datagram.data());
            QJsonObject request = requestDocument.object();
            //qDebug() << "Receive UDP" << requestDocument;

            if (request["method"] == "scan_am5")
            {
                if (request["result"].isNull())
                {
                    emit onScanRequest(request["id"].toInt(), datagram.senderAddress(), datagram.senderPort());
                } else
                {
                    emit onScanResponse(request);
                }
            }
        }
    });
}

void Am5Broadcast::send(const QJsonObject &jsonObject, const QHostAddress &host, quint16 port)
{
    QByteArray response = QJsonDocument{jsonObject}.toJson(QJsonDocument::Compact);
    //qDebug() << "Send UDP" << jsonObject;
    socket.writeDatagram(response, host, port);
}

void Am5Broadcast::scan()
{
    send({
        {"id", requestSequence++},
        {"method", "scan_am5"},
        {"params", ""}
    });
}
