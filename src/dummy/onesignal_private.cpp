#include "onesignal_private.h"

OneSignal_private::OneSignal_private(QObject *parent) : QObject(parent)
{

}

void OneSignal_private::Init(const QString apiKey){
    Q_UNUSED(apiKey);
}
