#include "onesignal.h"
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <Windows.ApplicationModel.h>
#include <Windows.System.UserProfile.h>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <Windows.h>
#include <windows.foundation.h>
#include <Windows.Security.ExchangeActiveSyncProvisioning.h>
#include <QtCore/qfunctions_winrt.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Security::ExchangeActiveSyncProvisioning;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::System::UserProfile;

OneSignal::OneSignal(QObject *parent):
    QObject(parent)
{
    m_initDone = false;
    m_foreground = true;
    m_sessionCallInProgress = m_sessionCallDone = false;

    connect(this,
            SIGNAL(pushIdFound(QString)),
            this,
            SLOT(sendSession(QString)));

    connect(this,
            SIGNAL(pushNotificationChannelReceived(IPushNotificationChannel*)),
            this,
            SLOT(onPushNotificationChannelReceived(IPushNotificationChannel*)));
}

void OneSignal::Init(QString appId,QVariantMap startupParams){
    Q_UNUSED(startupParams)
    connect(&m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(&m_nam, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(handleSSLErrors(QNetworkReply*)));
    m_appId = appId;
    m_playerId = settings.value("OneSignalPlayerId","").toString();
    m_channelUri = settings.value("OneSignalChannelUri","").toString();
    if (m_initDone){
        return;
    }
    lastPingTime = QDateTime::currentSecsSinceEpoch();

    //let's get the push id
    getPushNotification();
    m_initDone = true;
}
void OneSignal::getPushNotification(){
    ComPtr<IPushNotificationChannelManagerStatics> channelManager;
    HRESULT hr = GetActivationFactory(HStringReference(L"Windows.Networking.PushNotifications.PushNotificationChannelManager").Get(), &channelManager);
    IAsyncOperation<PushNotificationChannel*>* asyncOp;
    hr = channelManager->CreatePushNotificationChannelForApplicationAsync(&asyncOp);

    asyncOp->put_Completed(Callback<Implements<RuntimeClassFlags<ClassicCom>, IAsyncOperationCompletedHandler<PushNotificationChannel*>, FtmBase>>(
                               [this](IAsyncOperation<PushNotificationChannel*>* operation, AsyncStatus status)
    {
                               if (status == Error) {
                                   qDebug() << "Error Retrieving push notification channel";
                                   //emit error();
                               }
                               else if (status == AsyncStatus::Started) {
                                   qDebug() << "Operation Started";
                               }
                               else if (status == AsyncStatus::Completed) {
                                   qDebug() << "Operation complete";
                                   operation->GetResults(&notificationChannel);
                                   emit pushNotificationChannelReceived(notificationChannel);
                                   registerForNotifications();
                                   return S_OK;
                               }
                               return S_OK;
                           }).Get());
}
void OneSignal::onPushNotificationChannelReceived(IPushNotificationChannel* pnc){
    HSTRING pushNotificationId;
    pnc->get_Uri(&pushNotificationId);
    sendSession(QString::fromStdWString(WindowsGetStringRawBuffer(pushNotificationId, nullptr)));
    registerForNotifications();
}

//currentChannelUri
void OneSignal::sendSession(QString currentChannelUri){
    if (m_sessionCallInProgress || m_sessionCallDone){
        //Call already in progress
        return;
    }
    m_sessionCallInProgress = true;

    if (currentChannelUri != NULL && m_channelUri != currentChannelUri) {
        m_channelUri = currentChannelUri;
        settings.setValue("OneSignalChannelUri",m_channelUri);
    }
    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);

	/*
	Product Name
	*/
    ComPtr<IEasClientDeviceInformation> deviceInformation;
    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Security_ExchangeActiveSyncProvisioning_EasClientDeviceInformation).Get(),
                                    &deviceInformation);
    HSTRING systemProductName;
    deviceInformation->get_SystemProductName(&systemProductName);
	QString productName = QString::fromStdWString(WindowsGetStringRawBuffer(systemProductName, nullptr));

	/*
	Get application version
	*/
    ComPtr<IPackageStatics> packageStatics;
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_Package).Get(), &packageStatics);
    ComPtr<IPackage> package;
    hr = packageStatics->get_Current(&package);
    IPackageId *packageId;
    package->get_Id(&packageId);
    PackageVersion packageVersion;
    packageId->get_Version(&packageVersion);

    qDebug() << "Version: " << packageVersion.Major;
	QString appVersion = QString::number(packageVersion.Major) + "." + 
		QString::number(packageVersion.Minor) + "." + 
		QString::number(packageVersion.Revision) +"."+ 
		QString::number(packageVersion.Revision);
    
    
	/*
	Advertising ID
	*/
	ComPtr<IAdvertisingManagerStatics> advertisingManagerForUser;
	hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_System_UserProfile_AdvertisingManager).Get(), &advertisingManagerForUser);
	HSTRING advertisingId;
	advertisingManagerForUser->get_AdvertisingId(&advertisingId);
	QString adId = QString::fromStdWString(WindowsGetStringRawBuffer(advertisingId, nullptr));
    //TODO: Implement Language name
    QLocale userLocale;
    QString languageISOName = userLocale.bcp47Name().toUpper();

	/*
	Create JSON file
	*/
    QJsonObject jsonObject;
    jsonObject["device_type"] = 6;
    jsonObject["app_id"] = m_appId;
    jsonObject["identifier"] = m_channelUri;
    jsonObject["ad_id"] = adId;
    jsonObject["device_model"] = productName;
    jsonObject["game_version"] = appVersion;
    jsonObject["language"] = languageISOName;
    jsonObject["timezone"] = QString::number(lastPingTime);
    jsonObject["sdk"] = VERSION;

    QJsonDocument jsonDocument(jsonObject);
    qDebug()<<"Will submit to server: "<<jsonDocument.toJson();

	/*
	Prepare Submit URL
	*/
    QString urlString = "players";
    if (m_playerId != NULL) {
        urlString += "/" + m_playerId + "/on_session";
    }else{
        qDebug()<<"No Player ID";
    }
    qDebug()<<"Should request "<<BASE_URL+""+urlString;

	/*
	Execut request
	*/
    QNetworkRequest newRequest(QUrl(BASE_URL + "" + urlString));
    newRequest.setRawHeader("Content-Type", "application/json");
    m_nam.post(newRequest, jsonDocument.toJson());
}
void OneSignal::finished(QNetworkReply* replay){
    m_sessionCallInProgress = false;
    qDebug() << "Replay Finished!!!!";
    qDebug()<<"Network error "<<replay->error()<<" = "<<replay->errorString();
    if (!replay->error()){
        QString returnData = replay->readAll();
        qDebug()<<"Network Replay "<<returnData;
        QJsonDocument returnJsonDocument = QJsonDocument::fromJson(returnData.toLatin1());
        if (returnJsonDocument.isObject()){
            m_sessionCallDone = true;
            QJsonObject returnObject = returnJsonDocument.object();
            QString newId = returnObject.value("id").toString();
            if (newId!=""){
                m_playerId = newId;
                settings.setValue("OneSignalPlayerId",newId);
            }
        }else{
            //TODO: This is a error. Process it
        }
    }

}
void OneSignal::handleSSLErrors(QNetworkReply*){
    qDebug()<<"SSL error!!!!";
}
void OneSignal::registerForNotifications(){
    EventRegistrationToken addedToken;
    auto callback = Callback<ITypedEventHandler<PushNotificationChannel*,
            PushNotificationReceivedEventArgs*>>(
                [this](IPushNotificationChannel* sender, IPushNotificationReceivedEventArgs* e) {
        PushNotificationType  notificationType;
        e->get_NotificationType(&notificationType);

        switch (notificationType){
        case PushNotificationType_Badge:
            qDebug() << "Badge Notification";
            break;
        case PushNotificationType_Tile:
            qDebug() << "Tile Notification";
            break;
        case PushNotificationType_Toast: {
            HRESULT hr;
            qDebug() << "Toast Notification";
            IToastNotification *toastNotification;
            e->get_ToastNotification(&toastNotification);
            qDebug()<<"Toast notification is "<<toastNotification;
            IXmlDocument *xmlDocument;
            hr = toastNotification->get_Content(&xmlDocument);
			ComPtr<IXmlElement> firstElement;
			xmlDocument->get_DocumentElement(&firstElement);
			HSTRING mutableString;
			WindowsCreateString((const wchar_t*)QString("launch").utf16(), QString("launch").size(), &mutableString);
			HSTRING launchAttributeValue;
			firstElement->GetAttribute(mutableString, &launchAttributeValue);
			QString launchAttibuteValueString = QString::fromStdWString(WindowsGetStringRawBuffer(launchAttributeValue, nullptr));
			if (launchAttibuteValueString != NULL) {
				QJsonDocument document = QJsonDocument::fromJson(launchAttibuteValueString.toLatin1());
				qDebug() << "Document is object? " << document.isObject();
				//TODO: Use "Custom"  in application for custom data informations
				qDebug() << "Document keys " << document.object().toVariantMap().value("custom");
				if (document.isObject() && !document.object().toVariantMap().value("custom").isNull()) {
					ComPtr<IXmlNodeSelector> nodeSelector;
					ComPtr<IXmlDocument> ss(xmlDocument);
					ss.As(&nodeSelector);
					ComPtr<IXmlNode> xmlNode;
					WindowsCreateString((const wchar_t*)QString("/toast/visual/binding").utf16(), QString("/toast/visual/binding").size(), &mutableString);
					nodeSelector->SelectSingleNode(mutableString,&xmlNode);
					xmlNode.As(&nodeSelector);
					WindowsCreateString((const wchar_t*)QString("text[@id='2']").utf16(), QString("text[@id='2']").size(), &mutableString);
					ComPtr<IXmlNode> xmlNode2;
					nodeSelector->SelectSingleNode(mutableString, &xmlNode2);
					ComPtr<IXmlNodeSerializer> nodeSerializer;
					xmlNode2.As(&nodeSerializer);
					nodeSerializer->get_InnerText(&mutableString);
					qDebug() << "Push Notification message " << QString::fromStdWString(WindowsGetStringRawBuffer(mutableString, nullptr));
				}
				else {
					qDebug() << "No custom value??";
				}
			}
			else {
				qDebug() << "Invalid launch attribute???";
			}

			/*
			============================================================
			Keep this for debug
			*/
			ComPtr<IXmlNodeSerializer> nodeSerializer;
			ComPtr<IXmlDocument> ss(xmlDocument);
			ss.As(&nodeSerializer);
			HSTRING string;
			nodeSerializer->GetXml(&string);
			qDebug() << "Json Content " << QString::fromStdWString(WindowsGetStringRawBuffer(string, nullptr));
			/*
			===========================================================
			*/
            e->put_Cancel(true);
        }
            break;

        case PushNotificationType_Raw:
            qDebug() << "Raw Notification";
            break;
        }

        return S_OK;
    });
    HRESULT hr = notificationChannel->add_PushNotificationReceived(callback.Get(), &addedToken);
}
