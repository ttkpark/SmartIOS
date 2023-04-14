package com.example.inout_nodeapp;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.animation.AnimationUtils;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.isAdmin;
import static com.example.inout_nodeapp.MyApplication.versionName;

public class MenuActivity extends AppCompatActivity {
    MmenuIconContent APSettingBtn,AccountBtn;
    MmenuIconContent unregister,Attendlist,WebSetting;
    boolean mIsBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(isAdmin)
             setContentView(R.layout.activity_menu_admin);
        else setContentView(R.layout.activity_menu);

        if(isAdmin)APSettingBtn = findViewById(R.id.APSettingBtn);
        AccountBtn = findViewById(R.id.AccountBtn);
        Attendlist = findViewById(R.id.AttendList);
        //unregister = findViewById(R.id.unregister);
        WebSetting = findViewById(R.id.toWebView);

        setText();
        setColor();//4개 메뉴 항목의 색을 업데이트함.

        Attendlist.setOnClickListener(v->{
            startActivity(new Intent(this,Activity_simpleAttendList.class));
        });
        if(isAdmin)APSettingBtn.setOnClickListener(v->{
            startActivity(new Intent(this, APSettingActivity.class));
        });
        AccountBtn.setOnClickListener(v->{
            startActivity(new Intent(this, AccountActivity.class));
        });
        WebSetting.setOnClickListener(v->{
            startActivity(new Intent(this,WebViewActivity.class));
        });
        connectService();

        if(isAdmin) {
            Attendlist.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right));
            AccountBtn.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_left));
            APSettingBtn.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right_3));
            WebSetting.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_left_4));
            //WebSetting.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right_5));
        }
        else {
            Attendlist.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right));
            AccountBtn.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_left));
            WebSetting.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right_3));
            //unregister.startAnimation(AnimationUtils.loadAnimation(this, R.anim.anim_show_from_right_3));
        }
    }
    void setText()
    {
        //unregister.setTitle(getStringCode(446));
        Attendlist.setTitle(getStringCode(773));
        if(isAdmin)APSettingBtn.setTitle(getStringCode(10108));//[관리자] 등록 및 삭제
        AccountBtn.setTitle(getStringCode(62) + " " + getStringCode(442));//개인정보 + 수정
        ((TextView)findViewById(R.id.copyright)).setText(getStringCode(9999,getString(R.string.app_name)) + " (" + versionName + ")");
        //WebSetting.setTitle("");

        //unregister.getIconView().setImageResource(R.drawable.ic_baseline_person_24);
        //Attendlist.getIconView().setImageResource(R.);
        //APSettingBtn.getIconView().setImageResource(R.);
        //AccountBtn.getIconView().setImageResource(R.);
    }
    void setColor()
    {
        //unregister.setBackgroundColor(Color.parseColor(AppColor));
        //unregister.setTextColor(Color.parseColor(TextColor));
        Attendlist.setBackgroundColor(Color.parseColor(AppColor));
        Attendlist.setTextColor(Color.parseColor(TextColor));
        AccountBtn.setBackgroundColor(Color.parseColor(AppColor));
        AccountBtn.setTextColor(Color.parseColor(TextColor));
        WebSetting.setBackgroundColor(Color.parseColor(AppColor));
        WebSetting.setTextColor(Color.parseColor(TextColor));
        if(isAdmin)APSettingBtn.setBackgroundColor(Color.parseColor(AppColor));
        if(isAdmin)APSettingBtn.setTextColor(Color.parseColor(TextColor));
    }
    void connectService() {
        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
    }

    class ActivityHandler extends Handler
    {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                default: super.handleMessage(msg); break;
            }
        }
    }
    Messenger mServiceMessenger = null;
    final Messenger mMessenger = new Messenger(new MenuActivity.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_MENU));

                msg = Message.obtain(null,
                        MyService.SEND_VALUE,123456789,0);
                mServiceMessenger.send(msg);
            }catch (RemoteException e){}
        }

        @Override
        public void onServiceDisconnected(ComponentName name) { mServiceMessenger = null; }
    };


    @Override
    protected void onNewIntent(Intent intent) {
        processIntent(intent);
        super.onNewIntent(intent);
    }
    private void processIntent(Intent intent) {
        if (intent != null) {
            String reply = intent.getStringExtra("reply");
            Log.d("processIntent_Main", "reply : " + reply);

            if (reply == null) return;
            if (reply.equals("MessageAlert")) {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageAlert(Title, Message);
            }
            if (reply.equals("MessageSnack")) {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageSnack(Title, Message);
            }
        }
    }
    public void MessageAlert(String Title,String Messages)
    {
        new AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
                .setPositiveButton("OK", (dialog, which) -> {//OK
                }).create().show();
    }
    public void MessageSnack(String Title,String Messages){   }

    @Override
    public void finish() {
        SendMessagetoMyService(MyService.ACTIVITY_1);

        /*
        Intent intent = new Intent(this,MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra("reply","Terminate");
        startActivity(intent);

        moveTaskToBack(true);
        finishAndRemoveTask();
        System.exit(0);
        */
        super.finish();

    }
    void SendMessagetoMyService(int code)
    {
        if(mServiceMessenger != null)
            try {
                mServiceMessenger.send(Message.obtain(null,code));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
    }

}