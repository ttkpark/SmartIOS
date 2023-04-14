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

import java.util.concurrent.atomic.AtomicBoolean;

import static com.example.inout_nodeapp.MainActivity.isComlstfromNoClientCode;
import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.accountData;
import static com.example.inout_nodeapp.MyApplication.clientcode;
import static com.example.inout_nodeapp.MyApplication.companyList;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.getStringCodeHash;
import static com.example.inout_nodeapp.MyApplication.privClientcode;

public class AccountActivity extends AppCompatActivity {
    EditText Usr_Name,Usr_ID,Usr_birth;
    View mainLayout;
    TextView PageTitle, UUIDText;
    Button OKBtn,unregister,NewAccount;
    boolean mIsBound;
    Toolbar toolbar;
    ImageButton Home;
    TextView Title;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_account);
        SetToolBar();
        Usr_Name = findViewById(R.id.UsrName);
        Usr_ID = findViewById(R.id.Usr_ID);
        Usr_birth = findViewById(R.id.Usr_Birth);
        mainLayout = findViewById(R.id.mainLayout);
        PageTitle = findViewById(R.id.PageTitle);
        OKBtn = findViewById(R.id.savebtn4);
        unregister = findViewById(R.id.unregister);
        UUIDText = findViewById(R.id.UUIDText);
        OKBtn.setOnClickListener(v->onOK());
        NewAccount = findViewById(R.id.NewAccount);
        NewAccount.setOnClickListener(this::onNewAccountClicked);
        unregister.setOnClickListener(v->{
            if(mIsBound) new AlertDialog.Builder(this)
                    .setTitle(getStringCode(446)).setMessage(getStringCode(466))
                    .setPositiveButton(getStringCode(446),(dialog, which)->
                            SendMessagetoMyService(MyService.UNREGISTER)).create().show();
        });
        setText();
        setColor();//색 설정. : 툴바 OK버튼

        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
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
    void setText()
    {
        NewAccount.setText(getStringCode(865));
        unregister.setText(getStringCode(446));
        if(accountData != null) {
            Usr_Name.setText(accountData.Name);
            //Usr_birth.setText(accountData.birthday);
            Usr_ID.setText(accountData.ID);
        }
        PageTitle.setText(getStringCode(62));//개인정보
        OKBtn.setText(getStringCode(442));//수정
        UUIDText.setText("SN : " + BackgroundService.UUID);
    }
    void setColor()
    {
        //unregister.setBackgroundColor(Color.parseColor(AppColor));
        unregister.setTextColor(Color.parseColor(AppColor));
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        Home.setBackgroundColor(Color.parseColor(AppColor));
        Home.setColorFilter(Color.parseColor(TextColor));
        OKBtn.setBackgroundColor(Color.parseColor(AppColor));
        OKBtn.setTextColor(Color.parseColor(TextColor));
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

    @Override
    public void finish() {
        SendMessagetoMyService(MyService.ACTIVITY_1);
        Intent intent = new Intent(this,MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra("isNEW",false);
        startActivity(intent);
    }
    boolean isNeedAccountLoad = false;
    @Override
    protected void onPostResume() {
        super.onPostResume();
        if(isNeedAccountLoad) {
            isNeedAccountLoad = false;
            requestPersonCheckAPI();
        }
    }

    void AfterSettingClientCode(){//comlst 호출 후 사용자가 회사를 선택하면
        /*
        try {//언어를 호출한다. LANGUAGE -> LOGIN -> StartTask
            mServiceMessenger.send(Message.obtain(null, MyService.LANGUAGE));
        } catch (RemoteException e) {
            e.printStackTrace();
            TerminateAlert("Login", "APP Internal Error.");//"startTask 실행 오류."
        }
        */
        Log.d("메인","탈퇴 후 메인으로 간다.");
        SendMessagetoMyService(MyService.ACTIVITY_1);
        Intent intent = new Intent(this, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(intent);
    }
    void onNewAccountClicked(android.view.View v)
    {
        //Email
        //Name
        //Type
        //Language
        Intent intent = new Intent(this,NewAccountActivity.class);
        intent.putExtra("email",accountData.ID);
        intent.putExtra("name",accountData.Name);
        //intent.putExtra("type",getStringCode(10903));
        intent.putExtra("lang",accountData.lang);
        NewAccountActivity.noBackMode = false;
        startActivity(intent);
    }

    private void ShowmeNewAccountPage() {
        ((MyApplication)getApplication()).ResetFile();
        NewAccountActivity.noBackMode = true;
        startActivity(new Intent(this,NewAccountActivity.class));
    }

    void onClientCodeTextClick_Dialog() {

        if(companyList.length == 1) {
            setclientCode(0);
            AfterSettingClientCode();
            return;
        }
        String[] strings = new String[companyList.length];
        for(int i=0;i<companyList.length;i++)
            strings[i] = companyList[i].CompanyName;

        AtomicBoolean OnSelected = new AtomicBoolean(false);
        DataAdapter dataAdapter = new DataAdapter(strings, (index) ->{
            setclientCode(index);
            AfterSettingClientCode();
            OnSelected.set(true);
        });

        SelectClientDialog dlg = new SelectClientDialog(this,dataAdapter);
        dlg.setListener(new SelectClientListener() {
            @Override
            public void OnDismiss() {
                //MessageSnack(MyService.ComlstName,"회사 선택 취소");
                TerminateAlert("unregister",getStringCode(790));//"앱을 종료합니다."
            }
        });
        dlg.show();
    }
    void setclientCode(int position){
        clientcode = companyList[position].code;
        privClientcode = clientcode;
        Log.e("SetClientCode","clientcode -> " + clientcode);

        getStringCodeHash.put("10900",clientcode);//clientcode
        getStringCodeHash.put("10902",privClientcode);//privClientcode

        ((MyApplication)getApplication()).ResetFile();
        ((MyApplication)getApplication()).saveCodeData();
    }

    void requestPersonCheckAPI()
    {
        SendMessagetoMyService(MyService.person_check);
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
    final Messenger mMessenger = new Messenger(new AccountActivity.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_ACCOUNT));//Sets the "where to send the reply" of the Service : This Activity

                mServiceMessenger.send(Message.obtain(null,MyService.person_check));

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
            if(reply.equals("GetPersSucceed"))
            {
                Usr_Name.setText(accountData.Name);
                //Usr_birth.setText(accountData.birthday);
                Usr_ID.setText(accountData.ID);
            }

            if(reply.equals("ComlstSucceed"))
            {
                if(isComlstfromNoClientCode)//clientCode가 null이여서 호출됐다면
                    reply = "LoginNeedClientCode";//LoginNeedClientCode와 하는 작업이 같다.
                else
                    onClientCodeTextClick_Dialog();

            }
            if(reply.equals("ComlstNoAccount"))
            {
                ShowmeNewAccountPage();
            }
            if(reply.equals("unregister"))
            {
                SendMessagetoMyService(MyService.COMLST);
            }
        }
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

    public void TerminateAlert(String Title,String Messages)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Title).setMessage(Messages)
                .setPositiveButton("Exit", (dialog, which) -> {//Exit
                    //종료
                })
                .setOnDismissListener(dialog -> {
                    terminate();
                }).create().show();
    }
    void terminate()
    {
        finishAffinity();
        System.runFinalization();
        System.exit(0);
    }

    /*
    Menu m_menu;//메뉴 : Settings
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_account, menu);
        m_menu = menu;
        MenuTextUpdate();
        return true;
    }

    void MenuTextUpdate() {
        //m_menu.findItem(R.id.Modify).setTitle("Settings");
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        if (id == R.id.Modify) {//Account Settings를 클릭하면
            isNeedAccountLoad = true;
            startActivity(new Intent(this,AccountSettingActivity.class));
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
    */
}