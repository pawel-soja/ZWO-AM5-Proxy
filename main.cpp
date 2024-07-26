#include <QCoreApplication>

#include "am5broadcast.h"
#include "hostinfo.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QFile>

class TcpProxy: public QObject
{
public:
    TcpProxy(const QHostAddress &targetAddress, quint16 targetPort = 4030, quint16 serverPort = 4030)
    {
        server.listen(QHostAddress::Any, serverPort);

        // If we have a new client connection, we connect to mount to redirect data.
        connect(&server, &QTcpServer::newConnection, this, [this, targetAddress, targetPort] {

            QTcpSocket *target = new QTcpSocket(&server);
            target->connectToHost(targetAddress, targetPort);

            connect(target, &QTcpSocket::connected, this, [this, target] {
                QTcpSocket *client = server.nextPendingConnection();

                // client -> target
                connect(client, &QTcpSocket::readyRead, target, [client, target] {
                    QByteArray data = client->readAll();
                    qDebug() << "client -> target" << data;
                    target->write(data);
                });

                connect(client, &QTcpSocket::disconnected, target, [target] {
                    target->close();
                    target->deleteLater();
                });

                // target -> client
                connect(target, &QTcpSocket::readyRead, client, [client, target] {
                    QByteArray data = target->readAll();
                    qDebug() << "client <- target" << data;
                    client->write(data);
                });

                connect(target, &QTcpSocket::disconnected, client, [client] {
                    client->close();
                    client->deleteLater();
                });
            });

        });
    }

protected:
    QTcpServer server;
};

struct ProxyMount
{
    QJsonObject info;
    QSharedPointer<TcpProxy> proxy;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Am5Broadcast am5broadcast;

    // Sending a question about mounts on the local network
    am5broadcast.scan();

    // List of detected mounts
    QMap<QString /* guid */, ProxyMount /* data */> mounts;

    // Detected mount, saving to map, creating proxy server
    QObject::connect(&am5broadcast, &Am5Broadcast::onScanResponse, [&](const QJsonObject &response) {

        QString guid = response["result"]["guid"].toString();
        QHostAddress address = QHostAddress(response["result"]["ip"].toString());

        qDebug() << "Found AM5" << address.toString();

        mounts[guid].info = response;

        // We can't create multiple servers. There is no port number in the message to the client to set another one.
        if (mounts.count() > 1)
        {
            qFatal("Too many mounts. Not supported. Exiting...");
        }

        if (mounts[guid].proxy.isNull())
        {
            mounts[guid].proxy = QSharedPointer<TcpProxy>(new TcpProxy(address));
        }

    });

    /*
     * Some application (e.g.StarGazing) in the local network is asking about ASI mounts.
     * We impersonate detected mounts.
     */
    QObject::connect(&am5broadcast, &Am5Broadcast::onScanRequest, [&](int id, const QHostAddress &host, quint16 port) {
        for (const auto &mount: mounts)
        {
            const auto &info = mount.info;

            // We send the original message to the client with the changed name and IP address.
            QJsonObject proxyMount{
                {"jsonrpc", "1.7.15"},
                {"method", "scan_am5"},
                {"result", QJsonObject{
                    {"guid",   info["result"]["guid"].toString()},
                    {"ip",     HostInfo::ip().toString()},
                    {"ssid",   QString("Proxy_%1").arg(info["result"]["ssid"].toString())},
                    {"device", info["result"]["device"].toString()},
                    {"model",  info["result"]["model"].toString()}
                }},
                {"code", 0},
                {"id", id}
            };
            am5broadcast.send(proxyMount, host, port);
        }
    });



    return a.exec();
}
