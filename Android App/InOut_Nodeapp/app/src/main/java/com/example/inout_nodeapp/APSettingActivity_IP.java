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
import android.util.Base64;
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

import java.util.Objects;

import static com.example.inout_nodeapp.MyApplication.APInfoList;
import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.SelectedAPIndex;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;

public class APSettingActivity_IP extends AppCompatActivity {
    View mainLayout;
    MyApplication APP = (MyApplication) getApplication();
    boolean mIsBound;
    EditText ModuleName,Edit_IP,Edit_DefGate,Edit_SubMask,Edit_DNS1,Edit_DNS2;
    Button OKBtn;

    MyApplication.IPSettingData IPDatatoSendAPI = new MyApplication.IPSettingData();
    MyApplication.APInfo privAPSetting = APInfoList[SelectedAPIndex];

    String TitleMessage = "IP SETTINGS";//"AP선택 IP수정";
    Toolbar toolbar;
    ImageButton Home;
    TextView Title;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ap_setting_ip);
        SetToolBar();
        mainLayout = findViewById(R.id.vieww);
        ModuleName = findViewById(R.id.moduleName);
        Edit_IP = findViewById(R.id.IPAddr);
        Edit_DefGate = findViewById(R.id.DefGate);
        Edit_SubMask = findViewById(R.id.SubMask);
        Edit_DNS1 = findViewById(R.id.DNS1);
        Edit_DNS2 = findViewById(R.id.DNS2);
        OKBtn = findViewById(R.id.savebtn2);

        ModuleName.setEnabled(false);
        //ModuleName.setTextColor(Color.parseColor("#000000"));
        ModuleName.setText(privAPSetting.apname);

        OKBtn.setOnClickListener(v -> onClickOK());

        connectService();
        setColor();//색 설정 : 툴바, 저장

        ParseAddressByte(privAPSetting.IPAddr);
    }
    void SetToolBar()
    {
        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        Title = findViewById(R.id.toolbarText);
        Title.setText(getStringCode(9998,getString(R.string.app_name)));
        actionBar.setDisplayShowTitleEnabled(false);
        Home = findViewById(R.id.imageBtn);
        Home.setOnClickListener((v)->{
            Intent intent = new Intent(this,MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.putExtra("isNEW",false);
            startActivity(intent);
        });
    }
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        Home.setBackgroundColor(Color.parseColor(AppColor));
        Home.setColorFilter(Color.parseColor(TextColor));
        OKBtn.setBackgroundColor(Color.parseColor(AppColor));
        OKBtn.setTextColor(Color.parseColor(TextColor));
    }
    void ParseAddressByte(String data)
    {
        byte[] binData = Base64.decode(data.getBytes(),0);
        if(binData.length != 20)return;

        byte[][] IPs = new byte[5][];
        for(int i=0;i<5;i++) {
            IPs[i] = new byte[4];
            System.arraycopy(binData,i*4, IPs[i], 0, 4);
        }
        Edit_IP.setText(convertIP(IPs[0]));
        Edit_DefGate.setText(convertIP(IPs[1]));
        Edit_SubMask.setText(convertIP(IPs[2]));
        Edit_DNS1.setText(convertIP(IPs[3]));
        Edit_DNS2.setText(convertIP(IPs[4]));
    }
    String convertIP(byte[] binIP) {
        if (binIP[0] == 0 && binIP[1] == 0 && binIP[2] == 0 && binIP[3] == 0) return "";
        int[] IPAddr = new int[4];
        for (int i = 0; i < 4; i++)
        {
            IPAddr[i] = binIP[i];
            if(IPAddr[i]<0)
                IPAddr[i] += 256;
        }

        return String.format("%d.%d.%d.%d",IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);
    }
    void connectService() {
        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
    }
    byte[] ConvertIPAddress(String IPAddr) throws IntegrityException
    {
        byte[] IPByte = new byte[4];
        if ("".equals(IPAddr) || Objects.equals(null, IPAddr))
            throw new IntegrityException();

        String[] splits = IPAddr.split("\\.");

        if(splits.length != 4)
            throw new IntegrityException();
        for(int i=0;i<splits.length;i++)
            IPByte[i] = (byte) Integer.parseInt(splits[i]);


        return IPByte;
    }

    void UpdateValue() throws IntegrityException
    {
        String Text_IP = Edit_IP.getText().toString();
        String Text_DefGate = Edit_DefGate.getText().toString();
        String Text_SubMask = Edit_SubMask.getText().toString();
        String Text_DNS1 = Edit_DNS1.getText().toString();
        String Text_DNS2 = Edit_DNS2.getText().toString();

        byte[] ZeroIP = {0,0,0,0};

        try {IPDatatoSendAPI.IP = ConvertIPAddress(Text_IP);
        } catch (IntegrityException e) { throw new IntegrityException(); }//IP가 잘못되면 에러 발생

        IPDatatoSendAPI.APCode = privAPSetting.apcode;
        IPDatatoSendAPI.SensorType = privAPSetting.sensType;

        if(Text_DefGate.equals("")
                || Text_SubMask.equals("") || Text_DNS1.equals("") || Text_DNS2.equals(""))
        {
            //IP만 세팅
            IPDatatoSendAPI.DefGate = ZeroIP;
            IPDatatoSendAPI.SubMask = ZeroIP;
            IPDatatoSendAPI.DNS1 = ZeroIP;
            IPDatatoSendAPI.DNS2 = ZeroIP;
        }
        else
        {
            try {
                IPDatatoSendAPI.DefGate = ConvertIPAddress(Text_DefGate);
                IPDatatoSendAPI.SubMask = ConvertIPAddress(Text_SubMask);
                IPDatatoSendAPI.DNS1 = ConvertIPAddress(Text_DNS1);
                IPDatatoSendAPI.DNS2 = ConvertIPAddress(Text_DNS2);
            } catch (IntegrityException e) {
                e.printStackTrace();
                throw new IntegrityException();
            }
        }
    }

    void onClickOK()
    {
        try {
            UpdateValue();
            IPDatatoSendAPI.CheckValue();
            mServiceMessenger.send(Message.obtain(null,MyService.APSetting_IP,IPDatatoSendAPI));
        } catch (IntegrityException e) {
           MessageAlert(TitleMessage,getStringCode(792));//"값이 유효하지 않습니다.");
           e.printStackTrace();
        }
           catch (RemoteException e) {
            e.printStackTrace();
        }
    }




    @Override
    public void finish() {
        try {
            mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_3)); //back acticity is APSettingActivity.

            super.finish();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    static class ActivityHandler extends Handler {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                default: super.handleMessage(msg); break;
            }
        }
    }
    Messenger mServiceMessenger = null;
    final Messenger mMessenger = new Messenger(new APSettingActivity_general.ActivityHandler());
    private final ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_APSettingActivity_IP));//Sets the "where to send the reply" of the Service : This Activity

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
            if(reply.equals("MessageSnack"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageSnack(Title,Message);
            }
            if(reply.equals("MessageTerminate"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                TerminateAlert(Title,Message);
            }
            if(reply.equals("Succeed"))//APSetSucceed
                TerminateAlert(MyService.SetAPName,getStringCode(791));//"요청 성공"
        }
    }

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