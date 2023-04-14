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
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;

import static com.example.inout_nodeapp.MyApplication.APInfoList;
import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.SelectedAPIndex;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;

public class APSettingActivity_general extends AppCompatActivity {
    View mainLayout;
    boolean mIsBound;
    EditText ModuleName,APName,ModuleDistance;
    Button OKBtn,RemoveBtn;
    Spinner Sensor_spinner;
    CheckBox useYN;

    MyApplication.InformationSettingData DatatoSendAPI = new MyApplication.InformationSettingData();
    MyApplication.APInfo privAPSetting = APInfoList[SelectedAPIndex];

    String[] spinner_field = {"(Select)","BEACON","RFID","QRcode","barcode"};
    String[] spinner_SensorType = {"","bco","rfi","qrc","bac"};

    String TitleMessage = "SETTINGS";//"AP선택 정보수정";
    Toolbar toolbar;
    ImageButton Home;
    TextView Title;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ap_setting_general);
        mainLayout = findViewById(R.id.mainLayout);
        SetToolBar();
        ModuleName = findViewById(R.id.ModuleName);
        APName = findViewById(R.id.APName);
        ModuleDistance = findViewById(R.id.ModuleDistance);
        Sensor_spinner = findViewById(R.id.Sensor_spinner);
        useYN = findViewById(R.id.useYN);
        OKBtn = findViewById(R.id.savebtn);
        //RemoveBtn = findViewById(R.id.removeBtn);

        APName.setText(privAPSetting.apname);
        ModuleName.setEnabled(false);
        ModuleName.setText(privAPSetting.apcode);
        ModuleDistance.setText(privAPSetting.dist);
        useYN.setChecked(privAPSetting.useyn.equals("Y"));

        if(privAPSetting.useyn.equals("N"))
        {
            ModuleDistance.setEnabled(false);
            Sensor_spinner.setEnabled(false);
        }
        else
        {
            //APName.setTextColor(Color.parseColor("#000000"));
            //ModuleName.setTextColor(Color.parseColor("#000000"));
        }

        //체크박스
        //이전 상태가 Y면 -> enable
        //이전 상태가 N이면 -> disable
        //Y 상태에서 체크 해제 -> 이때는 notuse처리됨.
        //N 상태에서 체크 -> 실시간 적용되지는 않지만 use 처리됨.

        SetText();
        setColor();//색 설정 : 툴바,저장

        SetSpinner();
        OKBtn.setOnClickListener(this::onOKBtn);
        //RemoveBtn.setOnClickListener(this::onremoveBtn);
        setDefaultSensType();

        connectService();
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
    void SetText() {
        ((TextView)findViewById(R.id.textView4)).setText(getStringCode(10101));//AP고유번호
        ((TextView)findViewById(R.id.textView6)).setText(getStringCode(10102));//센서 종류
        ((TextView)findViewById(R.id.textView5)).setText(getStringCode(770));//거리값
        ((CheckBox)findViewById(R.id.useYN))    .setText(getStringCode(207));//사용
        ((Button)findViewById(R.id.savebtn))    .setText(getStringCode(3999));//저장
        ((Button)findViewById(R.id.removeBtn))  .setText(getStringCode(4999));//삭제
    }

    void setDefaultSensType(){
        for(int i=0;i<spinner_SensorType.length;i++)
        {
            if(spinner_SensorType[i].equals(privAPSetting.sensType))
            {
                Sensor_spinner.setSelection(i);
                return;
            }
        }
        Sensor_spinner.setSelection(0);
    }
    void SetSpinner() {
        final ArrayAdapter<String> adapter = new ArrayAdapter<>(this, R.layout.spinner_item, spinner_field);
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        Sensor_spinner.setAdapter(adapter);
        Sensor_spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if(Sensor_spinner.getSelectedItemPosition() >= 0)
                    DatatoSendAPI.SensorType = spinner_SensorType[position];
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }
    void connectService() {
        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
    }

    void onOKBtn(View v) {
        try {
            UpdateValue();
            DatatoSendAPI.CheckValue();

            mServiceMessenger.send(Message.obtain(null,MyService.APSetting_General,DatatoSendAPI));
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (IntegrityException e) {
            MessageAlert(TitleMessage,getStringCode(792));//"값이 유효하지 않습니다.");
        }

    }
    void onremoveBtn(View v) {
        UpdateValue();
        DatatoSendAPI.str_suffix = "out";
        try {
            mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_APSettingActivity_general));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }
    void UpdateValue() {
        if(DatatoSendAPI.SensorType == null || "".equals(DatatoSendAPI.SensorType))
            DatatoSendAPI.SensorType = privAPSetting.sensType;
        

        DatatoSendAPI.str_ModuleName = ModuleName.getText().toString();
        DatatoSendAPI.str_ModuleDistance = ModuleDistance.getText().toString();

        if(DatatoSendAPI.str_ModuleDistance.charAt(DatatoSendAPI.str_ModuleDistance.length()-1) == '.')//"12." -> "12"
            DatatoSendAPI.str_ModuleDistance = DatatoSendAPI.str_ModuleDistance.substring(0,DatatoSendAPI.str_ModuleDistance.length()-1);

        ModuleDistance.setText(DatatoSendAPI.str_ModuleDistance);

        if(privAPSetting.useyn.equals("Y"))
        {
            if(useYN.isChecked())DatatoSendAPI.str_suffix = "in"; // Y->Y
            else DatatoSendAPI.str_suffix = "notuse"; // Y->N
        }
        else
        {
            if(useYN.isChecked())DatatoSendAPI.str_suffix = "use"; // N->Y
            else DatatoSendAPI.str_suffix = "notuse"; //N->N
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
    final Messenger mMessenger = new Messenger(new ActivityHandler());
    private final ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_APSettingActivity_general));//Sets the "where to send the reply" of the Service : This Activity

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