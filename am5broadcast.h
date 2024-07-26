#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QJsonObject>

class Am5Broadcast: public QObject
{
    Q_OBJECT
public:
    explicit Am5Broadcast(QObject *parent = nullptr);
    void send(const QJsonObject &jsonObject,const QHostAddress &host = QHostAddress::Broadcast, quint16 port = 3333);
    void scan();

protected:
    QUdpSocket socket;
    QHostAddress myIpAddress;
    QHostAddress myBroadcastAddress;
    int requestSequence; // Sending message, "id" field is incremented by 1.

signals:
    void onScanRequest(int id, const QHostAddress &host, quint16 port);
    void onScanResponse(const QJsonObject &info);
};
