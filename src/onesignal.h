#ifndef ONESIGNAL_H
#define ONESIGNAL_H

#include <QObject>

#include "onesignal_private.h"

class OneSignal : public QObject
{
    Q_OBJECT
public:
    explicit OneSignal(QObject *parent = 0);
    void Init(const QString apiKey = "");
private:
    OneSignal_private *m_oneSignal;
signals:

public slots:
};

#endif // ONESIGNAL_H
