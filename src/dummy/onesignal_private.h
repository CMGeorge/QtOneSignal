#ifndef ONESIGNAL_PRIVATE_H
#define ONESIGNAL_PRIVATE_H

#include <QObject>

class OneSignal_private : public QObject
{
    Q_OBJECT
public:
    explicit OneSignal_private(QObject *parent = 0);
    void Init(const QString apiKey = "");
signals:

public slots:
};

#endif // ONESIGNAL_PRIVATE_H
