package com.onesignal;

import com.onesignal.OneSignal;
import android.content.Context;
//import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtApplication;
import android.os.Bundle;
public class OneSignalPrivate extends QtApplication{
    private static OneSignalPrivate singleton;

    //@Override
//    public void onCreate(Bundle bundle) {
//        super.onCreate(bundle);
//        System.out.println("On Created");
//        // Logging set to help debug issues, remove before releasing your app.
//              OneSignal.setLogLevel(OneSignal.LOG_LEVEL.DEBUG, OneSignal.LOG_LEVEL.WARN);
//              OneSignal.startInit(this).init();
//    }
    public OneSignalPrivate(){
        singleton=this;
    }

    public static String OneSignalPrivate(Context c){
        System.out.println("Should register for notification");
        OneSignal.setLogLevel(OneSignal.LOG_LEVEL.DEBUG,
                                OneSignal.LOG_LEVEL.WARN);
        OneSignal.startInit(c).init();
        return "OK";

    }
}
