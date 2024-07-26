#pragma once
#include <QHostAddress>

class HostInfo
{
public:
    static QHostAddress ip();
    static QHostAddress broadcast();
    static QHostAddress broadcast(const QHostAddress &address);
};

