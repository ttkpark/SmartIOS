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
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import java.util.Objects;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.getStringCodeHash;
import static com.example.inout_nodeapp.MyApplication.languageCode;
import static com.example.inout_nodeapp.MyApplication.loginType;

public class NewAccountActivity extends AppCompatActivity {
    Toolbar toolbar;
    ImageButton Home;
    Button OK,Cancel;
    TextView Title;
    EditText ID,Name,Clientcode,Password,Password2;
    Spinner UserType,Language;
    CheckBox Check;

    static boolean noBackMode = false;
    boolean mIsBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_new_account);
        SetToolBar();

        ID = findViewById(R.id.editText101);
        Name = findViewById(R.id.editText102);
        Clientcode = findViewById(R.id.editText103);
        UserType = findViewById(R.id.Sensor_spinner104);
        Language = findViewById(R.id.Sensor_spinner105);
        Password = findViewById(R.id.editText106);
        Password2 = findViewById(R.id.editText107);
        Check = findViewById(R.id.checkBox);

        OK = findViewById(R.id.loginbtn);
        Cancel = findViewById(R.id.oncancel);
        OK.setOnClickListener(v->onOK());
        Cancel.setOnClickListener(v->finish());

        SpinnerSetting();

        SetText();
        setColor();//색 설정 : 조회 버튼, 툴바
        if(mServiceMessenger == null) {
            Intent intent = new Intent(this, MyService.class);
            mIsBound = bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
    }
    boolean isValidText(String in)
    {
        return in.length() > 0;
    }
    String ConvertStringInTable(String source,String[][] Table) throws ArrayIndexOutOfBoundsException
    {
        for(String[] row : Table)
        {
            if(source.equals(row[1]))
                return row[0];
        }
        throw new ArrayIndexOutOfBoundsException();
    }
    void onOK()
    {
        if(!Check.isChecked())
        {
            Toast.makeText(this, "Please Check and Agree.", Toast.LENGTH_SHORT).show();
            return;
        }
        if(!Objects.equals(Password.getText().toString(), Password2.getText().toString()))
        {
            Toast.makeText(this, "Check your Password again.", Toast.LENGTH_SHORT).show();
            return;
        }


        MyApplication.NewAccountData data = new MyApplication.NewAccountData();
        data.ID = ID.getText().toString();
        data.Name = Name.getText().toString();
        data.clientcode = Clientcode.getText().toString();
        int Type = UserType.getSelectedItemPosition();
        int languagePos = Language.getSelectedItemPosition();
        data.Password = Password.getText().toString();
        data.phone = "010-6891-7112";

        if (!isValidText(data.ID) || !isValidText(data.Name) || !isValidText(data.clientcode) || !isValidText(data.Password)) {
            Toast.makeText(this, "Check Text again.", Toast.LENGTH_SHORT).show();
        }

        try{
            data.Type = loginType[Type][0];
            data.Language = languageCode[languagePos][0];
        }catch (ArrayIndexOutOfBoundsException e){
            Toast.makeText(this, "Select Type or Language.", Toast.LENGTH_SHORT).show();
        }
        getStringCodeHash.put("10903",data.Type );//선택한 Type
        getStringCodeHash.put("10904",data.Language);//선택한 language
        ((MyApplication)getApplication()).saveCodeData();

        try {
            mServiceMessenger.send(Message.obtain(null,MyService.NEW_ACCOUNT,data));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }
    void terminate()
    {
        finishAffinity();
        System.runFinalization();
        System.exit(0);
    }
    void SpinnerSetting()
    {
        String[] TypeList = new String[loginType.length];
        for(int i=0;i<TypeList.length;i++)
            TypeList[i] = loginType[i][1];
        final ArrayAdapter<String> adapter = new ArrayAdapter<>(this, R.layout.spinner_item, TypeList);
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        UserType.setAdapter(adapter);

        String[] LanguageList = new String[languageCode.length];
        for(int i=0;i<LanguageList.length;i++)
            LanguageList[i] = languageCode[i][1];
        final ArrayAdapter<String> adapter1 = new ArrayAdapter<>(this, R.layout.spinner_item, LanguageList);
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        Language.setAdapter(adapter1);
    }

    @Override
    public void finish() {
        if(noBackMode) terminate();
        else {
            try {
                mServiceMessenger.send(Message.obtain(null, MyService.ACTIVITY_1));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            Intent intent = new Intent(this, MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.putExtra("isNEW",false);
            startActivity(intent);
        }
    }

    void SetText() {
        Intent intent = getIntent();
        if(intent != null)
        {
            String email = intent.getStringExtra("email");
            String name  = intent.getStringExtra("name");
            //String type  = intent.getStringExtra("type");
            String lang  = intent.getStringExtra("lang");

            ID.setText(email);
            Name.setText(name);

            /*
            for(int i=0;i<loginType.length;i++)
            {
                if(Objects.equals(loginType[i][0], type))
                {
                    UserType.setSelection(i);
                    break;
                }
            }
            */

            for(int i=0;i<languageCode.length;i++)
            {
                if(Objects.equals(languageCode[i][0], lang)) {
                    Language.setSelection(i);
                    break;
                }
            }

        }
        //((TextView)findViewById(R.id.textView101)).setText();
    }

    void SetToolBar()
    {
        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        Title = findViewById(R.id.toolbarText);
        Title.setText(getStringCode(9998,getString(R.string.app_name)));
        actionBar.setDisplayShowTitleEnabled(false);
        //Home = findViewById(R.id.imageBtn);
        //Home.setOnClickListener((v)->{
       //     Intent intent = new Intent(this,MainActivity.class);
       //     intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
       //     startActivity(intent);
       // });
    }
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        OK.setBackgroundColor(Color.parseColor(AppColor));
        OK.setTextColor(Color.parseColor(TextColor));
        Cancel.setBackgroundColor(Color.parseColor(AppColor));
        Cancel.setTextColor(Color.parseColor(TextColor));
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
    final Messenger mMessenger = new Messenger(new NewAccountActivity.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_NEW_ACCOUNT));

                msg = Message.obtain(null,
                        MyService.SEND_VALUE,123456789,0);
                mServiceMessenger.send(msg);
            }catch (RemoteException e){}
        }

        @Override
        public void onServiceDisconnected(ComponentName name) { mServiceMessenger = null; }
    };
    //noBackMode = true;

    @Override
    protected void onNewIntent(Intent intent) {
        processIntent(intent);
        super.onNewIntent(intent);
    }
    private void processIntent(Intent intent)
    {
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
            if(reply.equals("NewAccountSucceed"))
            {
                noBackMode = false;
                finish();
            }
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
}