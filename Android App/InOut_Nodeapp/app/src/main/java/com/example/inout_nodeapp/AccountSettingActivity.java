package com.example.inout_nodeapp;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;

import static com.example.inout_nodeapp.MyApplication.accountData;
import static com.example.inout_nodeapp.MyApplication.getStringCode;

public class AccountSettingActivity extends AppCompatActivity {
    EditText Usr_Name,Usr_ID,Usr_birth;
    View mainLayout;
    boolean mIsBound;
    Button OKBtn;//,useYNBtn;
    // boolean useY = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_account_setting);
        SetToolBar();
        Usr_Name = findViewById(R.id.UsrName);
        Usr_ID = findViewById(R.id.Usr_ID);
        Usr_birth = findViewById(R.id.Usr_Birth);
        mainLayout = findViewById(R.id.mainLayout);
        OKBtn = findViewById(R.id.savebtn);
        // useYNBtn = findViewById(R.id.btn_useYN);

        Usr_Name.setText(accountData.Name);
        //Usr_birth.setText(accountData.birthday);
        Usr_ID.setText(accountData.ID);

        OKBtn.setOnClickListener(v->onOK());
        // useYNBtn.setOnClickListener(v->onuseYN());

        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
    }
    void onOK()
    {
        accountData.Name = Usr_Name.getText().toString();
        //accountData.birthday = Usr_birth.getText().toString();
        accountData.ID = Usr_ID.getText().toString();

        SendMessagetoMyService(MyService.person_modify);
    }
    //> 전체 로직
    //  -- 휴대폰 분실(p_guid='')로 인하여 핸드폰번호와 핸드폰번호암호코드를 삭제하여
    //     휴대폰에서 로그인을 막는다.
    //  -- 이후 휴대폰을 구매하였을 경우 웹으로 로그인하여 휴대폰번호를
    //     저장(핸드폰번호암호코드도 생성)한 이후 휴대폰에서 '메뉴-등록'을 통해
    //     재사용할 수 있다.
    /*void onuseYN()
    {
        String suffix;
        if(useY)suffix = "pers0";
        else suffix = "pers1";
        SendMessagetoMyService(MyService.person_useYN,suffix);
    }
    */
    void SetToolBar()
    {
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        ((TextView)findViewById(R.id.toolbarText)).setText(getStringCode(9998,getString(R.string.app_name)));
        actionBar.setDisplayShowTitleEnabled(false);
        ((ImageButton)findViewById(R.id.imageBtn)).setOnClickListener((v)->{
            Intent intent = new Intent(this,MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.putExtra("isNEW",false);
            startActivity(intent);
        });
    }
    void SendMessagetoMyService(int code) {
        if(mServiceMessenger != null)
            try {
                mServiceMessenger.send(Message.obtain(null,code));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
    }
    /*
    void SendMessagetoMyService(int code,Object obj) {
        if(mServiceMessenger != null)
            try {
                mServiceMessenger.send(Message.obtain(null,code,obj));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
    }
    */

    @Override
    public void finish() {
        SendMessagetoMyService(MyService.ACTIVITY_ACCOUNT);
        super.finish();
    }
    class ActivityHandler extends Handler {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                default: super.handleMessage(msg); break;
            }
        }
    }
    Messenger mServiceMessenger = null;
    final Messenger mMessenger = new Messenger(new AccountSettingActivity.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_ACCOUNTSetting));//Sets the "where to send the reply" of the Service : This Activity

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
        if(intent != null)
        {
            String reply = intent.getStringExtra("reply");

            if(reply == null)return;
            if(reply.equals("MessageAlert"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageAlert(Title,Message);
            }
            else if(reply.equals("MessageSnack"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageSnack(Title,Message);
            }
            else if(reply.equals("MessageTerminate"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                TerminateAlert(Title,Message);
            }/*
            else if(reply.equals("AppTerminate"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                Log.w(APIClass.TaskName,"Now App Terminating");
                AppTerminate(Title,Message);
            }*/
            else if(reply.equals("UsrModifySucceed"))
                TerminateAlert(MyService.UsrModifyName,getStringCode(791));//"요청 성공"
        }
    }

    /*
    private void AppTerminate(String Title, String Message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Title).setMessage(Message)
                .setPositiveButton("Exit", (dialog, which) -> endApp())
                .setOnDismissListener(dialog -> endApp()).create().show();
    }
    */


    public void TerminateAlert(String Title,String Messages)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Title).setMessage(Messages)
                .setPositiveButton("Exit", (dialog, which) -> finish())
                .setOnDismissListener(dialog -> finish()).create().show();
    }
    public void MessageAlert(String Title,String Messages)
    {
        new AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
                .setPositiveButton("OK", (dialog, which) -> {//OK
                }).create().show();
    }
    public void MessageSnack(String Title,String Messages)
    {
        Snackbar snackbar = Snackbar.make(mainLayout, Title + "\n" + Messages, Snackbar.LENGTH_LONG);

        snackbar.setAction("OK", v->snackbar.dismiss()).show();//OK
    }
}