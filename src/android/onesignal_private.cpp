#include "onesignal_private.h"
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroid>
#include <QDebug>
OneSignal_private::OneSignal_private(QObject *parent) : QObject(parent)
{

}

void OneSignal_private::Init(QString apiKey){

    QAndroidJniObject str = QAndroidJniObject::callStaticObjectMethod("com/onesignal/OneSignalPrivate",
                                                                      "OneSignalPrivate",
                                                                      "(Landroid/content/Context;)Ljava/lang/String;",
                                                                      QtAndroid::androidActivity().object<jobject>());
}
