#include "onesignal.h"

OneSignal::OneSignal(QObject *parent) : QObject(parent)
{
    m_oneSignal = new OneSignal_private(this);
}

void OneSignal::Init(const QString apiKey){
    m_oneSignal->Init(apiKey);
}
