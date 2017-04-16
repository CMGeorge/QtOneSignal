#ifndef ONESIGNAL_PRIVATE_H
#define ONESIGNAL_PRIVATE_H

#include <QObject>
#include <QVariantMap>
#include <QSettings>
#include <QNetworkReply>

#include <Windows.Networking.PushNotifications.h>
using namespace ABI::Windows::Networking::PushNotifications;

class OneSignal_private : public QObject
{
    Q_OBJECT
public:
    explicit OneSignal_private(QObject *parent = 0);
    void Init(QString appId,QVariantMap startupParams = QVariantMap());

public:
    const QString VERSION = "010101";

private:
    const QString BASE_URL = "https://onesignal.com/api/v1/";
    QString m_appId;
    QString m_playerId, m_channelUri;
    long lastPingTime;
    QSettings settings;
    bool m_initDone;
    bool m_foreground;
    bool m_sessionCallInProgress, m_sessionCallDone;
private:
    void getPushNotification();
    QNetworkAccessManager m_nam;
	//QNetworkAccessManager *m_nam;
	QNetworkRequest newRequest;
    IPushNotificationChannel *notificationChannel;
    void registerForNotifications();
signals:
    void pushIdFound(QString);
    void pushNotificationChannelReceived(IPushNotificationChannel* pnc);
public slots:

private slots:
    void finished(QNetworkReply *reply);
    void handleSSLErrors(QNetworkReply*);
    void sendSession(QString currentChannelUri);
    void onPushNotificationChannelReceived(IPushNotificationChannel* pnc);

	
};

#endif // ONESIGNAL_PRIVATE_H
