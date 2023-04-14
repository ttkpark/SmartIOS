package com.example.inout_nodeapp;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

public class AutoRunReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {

        Log.i("AutoRunReceiver","BOOT_COMPLETED");

        if(intent.getAction().equals("android.intent.action.BOOT_COMPLETED"))
        {
            Intent in = new Intent(context,BackgroundService.class);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                context.startForegroundService(in);
            else
                context.startService(in);
        }
    }
}