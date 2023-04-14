package com.example.inout_nodeapp;

import android.Manifest;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.content.ContextCompat;

import com.google.android.material.snackbar.Snackbar;

import org.altbeacon.beacon.BeaconTransmitter;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;

import static com.example.inout_nodeapp.BackgroundService.beacon;
import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.GatePositionToRemark;
import static com.example.inout_nodeapp.MyApplication.ReadUUID;
import static com.example.inout_nodeapp.MyApplication.SelectColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.clientcode;
import static com.example.inout_nodeapp.MyApplication.companyList;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.getStringCodeHash;
import static com.example.inout_nodeapp.MyApplication.isMainActivityAlive;
import static com.example.inout_nodeapp.MyApplication.iscodeDataVaild;
import static com.example.inout_nodeapp.MyApplication.privClientcode;
import static com.example.inout_nodeapp.MyApplication.versionName;

public class MainActivity extends AppCompatActivity {
    //초기설정이 완료되었는가를 김지하는 플래그. 이것에 따라 버튼작동여부가 갈린다.
    static boolean isSetupComplpted = false;

    TextView Text_Clientcode;
    TextView txt_BLEState;
    TextView copyrightView;
    View mainLayout;
    Button QRCodeBtn,OpenDoorBtn;
    ImageView BLEStatusView;
    String BLEStateString = "";

    String UUID = "";
    boolean isFirst = false;
    boolean isBLEEnable = false;
    boolean mIsBound = false;
    SelectClientDialog dlg;
    Toolbar toolbar;
    MmenuIconContent QRCodeCtrl,ClientCodeCtrl;
    MmenuIconContent NowINI,NowINO,NowOUT,Toggle_IO;

    private static final int REQUEST_ENABLE_BT = 0;
    private Menu m_menu;
    static boolean inited = false;
    boolean isNEW = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        if(intent != null)
          isNEW = intent.getBooleanExtra("isNEW",true);
        /*
        app_preferences = PreferenceManager.getDefaultSharedPreferences(this);
        int appColor = app_preferences.getInt("color", 0);
        int appTheme = app_preferences.getInt("theme", 0);

        if (appColor == 0 || appTheme == 0){
            setTheme(R.style.Theme_MaterialComponents_DayNight_NoActionBar);
        }else{
            setTheme(appTheme);
        }
        */ //앱의 색 입히기 - 보류

        /*
        final String overflowDesc = getString(R.string.accessibility_overflow);
        // The top-level window
        final ViewGroup decor = (ViewGroup) getWindow().getDecorView();
        // Wait a moment to ensure the overflow button can be located
        decor.postDelayed(new Runnable() {
            @Override
            public void run() {
                // The List that contains the matching views
                final ArrayList<View> outViews = new ArrayList<>();
                // Traverse the view-hierarchy and locate the overflow button
                decor.findViewsWithText(outViews, overflowDesc,
                        View.FIND_VIEWS_WITH_CONTENT_DESCRIPTION);
                // Guard against any errors
                if (outViews.isEmpty()) {
                    return;
                }
                try {
                    // Do something with the view
                    final ImageButton overflow = (ImageButton) outViews.get(0);
                    overflow.setColorFilter(Color.parseColor(TextColor));
                    //overflow.setImageResource(R.drawable.ic_loading);
                }catch(Exception e){}
            }
        }, 1000);
        // 앱 메뉴버튼 색 변경하기
        // 출처 : https://stackoverflow.com/questions/22046903/changing-the-android-overflow-menu-icon-programmatically/27672844
        */ //메뉴 아이콘의 색을 변경.-보류
        
        setContentView(R.layout.activity_main);
        toolbar = findViewById(R.id.toolbar);//툴바 설정
        setSupportActionBar(toolbar);

        Objects.requireNonNull(getSupportActionBar()).setTitle(getStringCode(9998,getString(R.string.app_name)));

        //뷰 컨트롤 받아오기
        mainLayout = findViewById(R.id.SnackLayout);
        Text_Clientcode = findViewById(R.id.textview);
        txt_BLEState = findViewById(R.id.NowStatus);
        QRCodeBtn = findViewById(R.id.QRCodeBtn);
        copyrightView = findViewById(R.id.copyright);
        OpenDoorBtn = findViewById(R.id.BtnOpen);
        QRCodeCtrl = findViewById(R.id.QRCode);
        ClientCodeCtrl = findViewById(R.id.ClientCode);
        NowINI = findViewById(R.id.NowINI);
        //NowINO = findViewById(R.id.NowINO);
        NowOUT = findViewById(R.id.NowOUT);
        Toggle_IO = findViewById(R.id.Toggle_IO);

        getStrTextUpdate();

        BLEStatusView = findViewById(R.id.beaconState);
        BLEStatusView.setOnClickListener(v->{
            Toast.makeText(this, BLEStateString, Toast.LENGTH_SHORT).show();
        });
        OpenDoorBtn.setOnClickListener(v->{
            CryptUtil crypt = new CryptUtil();
            beacon.makedata(crypt.AESEncode("01000000001",1));
            beacon.aesNo = 1; beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01010000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01020000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01030000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01040000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01050000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01060000001",1));
            beacon.BLEStart();
            beacon.makedata(crypt.AESEncode("01070000001",1));
            beacon.BLEStart();
            Toast.makeText(this, "Start!!", Toast.LENGTH_SHORT).show();
        });



        //문 열기 버튼 설정
        //OpenDoorBtn.setOnClickListener(v->{if(!isBackLocked())SendMessagetoMyService(MyService.DoorOpen);});//눌리면->API호출(DoorOpen)
        //OpenDoorBtn.setText(getStringCode(806,"Door Open"));

        ((ImageView)findViewById(R.id.imageLogo))//로고 보이기+(살짝 어두운 효과)
                .setColorFilter(Color.parseColor("#E0E0E0"), PorterDuff.Mode.MULTIPLY);

        if(inited || !isNEW){
            inited=true;
            Init_aboutfile();
            StartTask();
            return;
        }
        inited=true;

        if(ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            Init_aboutfile();
        }

        if(!isBLEenabled()){//BLE가 없는지 체크.
            LEErrorCause();//BLE가 없으면 : 없다고 보여 주기
            isBLEEnable = false;
            MyService.UserBLEDisabled = false;
            if(!getStringCode(800).equals("~"))
                new AlertDialog.Builder(this).setTitle("No Bluetooth").setMessage(getStringCode(800))
                        .setPositiveButton("OK", (dialog, which) -> {})
                        .setOnDismissListener((dialog)-> permissionCheck()).create().show();//다이얼로가 사라지면 권한을 확인한다.
            else permissionCheck();
        }else {//BLE가 있다면
            isBLEEnable = true;
            permissionCheck();
        }
    }
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        toolbar.setTitleTextColor(Color.parseColor(TextColor));
        QRCodeBtn.setBackgroundColor(Color.parseColor(AppColor));
        QRCodeBtn.setTextColor(Color.parseColor(TextColor));
        copyrightView.setBackgroundColor(Color.parseColor(AppColor));
        copyrightView.setTextColor(Color.parseColor(TextColor));
        QRCodeCtrl.setBackgroundColor(Color.parseColor(AppColor));
        QRCodeCtrl.setTextColor(Color.parseColor(TextColor));
        ClientCodeCtrl.setBackgroundColor(Color.parseColor(AppColor));
        ClientCodeCtrl.setTextColor(Color.parseColor(TextColor));
        ClientCodeCtrl.setSubTitleColor(Color.parseColor(TextColor));
        NowINI.setBackgroundColor(Color.parseColor(AppColor));
        NowINI.setTextColor(Color.BLACK);
        //NowINO.setBackgroundColor(Color.parseColor(AppColor));
        //NowINO.setTextColor(Color.BLACK);
        NowOUT.setBackgroundColor(Color.parseColor(AppColor));
        NowOUT.setTextColor(Color.BLACK);
    }
    boolean fileinited = false;
    void Init_aboutfile()
    {
        if(!fileinited)fileinited = true; else return;
        Log.i("Init_aboutfile","File Load Start!!");
        if(!fileread() || UUID.length() < 5)isFirst = true;// UUID를 가져온다. 유효하지 않으면 시작 플래그를 1로 만든다.
        ((MyApplication)getApplication()).InitHashes();
        SelectColor();// 앱의 색을 설정한다.
        setColor();

        if(iscodeDataVaild())//데이터가 유효한지 확인
        {
            ((MyApplication)getApplication()).checkFileAndRestore();//파일이 없으면 체크하고 다시 저장한다.
        }else ((MyApplication)getApplication()).InitHashes();// 파일에서 가져온다.

        isMainActivityAlive = true;//앱 상태를 보고한다.
        //Text_Clientcode.setOnClickListener(this::onClientCodeTextClicked);
        ClientCodeCtrl.setOnClickListener(this::onClientCodeTextClicked);
        UpdateTextClientCode();//화면에 ClientCode를 업테이트한다.
        MyService.UserBLEDisabled = false;
        isSetupComplpted = false;
    }
    void onClientCodeTextClicked(View view) {//클라이언트 텍스트가 눌려 클라이언트 다이얼로그를 띄워야 할 때
        if(isBackLocked())return;
        Log.i("COMLST Start","");
        isComlstfromNoClientCode = false;
        SendMessagetoMyService(MyService.COMLST);
    }
    void onNowINOUT_CLICKED(int state)
    {
        if(isBackLocked())return;
        //state 3=외근퇴근 2=외근출근 1=내근 0=퇴근
        Log.d("onNowINOUT_CLICKED",state==3?"INO_O":state==2?"INO_I":state==1?"INI_I":"INI_O");

        //format = 2021-03-26 03:25:56.559
        GatePositionToRemark = new MyApplication.GatePosition();
        SimpleDateFormat dateformat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault());
        Date now = new Date();
        GatePositionToRemark.time = dateformat.format(now);

        if(mServiceMessenger != null)
            try {
                mServiceMessenger.send(Message.obtain(null,MyService.LISTING_REMARK,state,0));
            } catch (RemoteException e) {
                e.printStackTrace();
            }

        //Toast.makeText(this, isIN?"출근 처리되었습니다":"퇴근 처리되었습니다", Toast.LENGTH_SHORT).show();
    }

    boolean isBLENeededtoON()
    {
        int flag = BeaconTransmitter.checkTransmissionSupported(this);
        return flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS || flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER;
    }
    boolean isBLEenabled()
    {
        return BluetoothAdapter.getDefaultAdapter() != null && getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
    }
    void LEErrorCause()
    {
        //new AlertDialog.Builder(this).setTitle("No LE").setMessage(msg)
        //        .setPositiveButton("OK", (dialog, which) -> {}).create().show();
        String msg;
        int res = BeaconTransmitter.checkTransmissionSupported(this);
        switch(res){
            case BeaconTransmitter.NOT_SUPPORTED_BLE:msg = "NOT_SUPPORTED_BLE";
            case BeaconTransmitter.NOT_SUPPORTED_MIN_SDK:msg = "NOT_SUPPORTED_MIN_SDK";
            case BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER:msg = "NOT_SUPPORTED_CANNOT_GET_ADVERTISER";
            case BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS:msg = "NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS";
            default: msg = "Unknown" + res;
        }
        Log.e("LEErrorCause","res : " + res);
        Log.e("LEErrorCause","msg : " + msg);

        MessageSnack("LEErrorCause",msg);
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
    final Messenger mMessenger = new Messenger(new ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_1));

                mServiceMessenger.send(
                        Message.obtain(null,MyService.START,isFirst?1:0,isNEW?1:0,UUID));

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
    private void processIntent(Intent intent)
    {
        if(intent != null)
        {
            String reply = intent.getStringExtra("reply");
            Log.d("processIntent_Main","reply : " + reply);

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
            if(reply.equals("Terminate")){
                terminate();
                return;
            }
            /*
            if(reply.equals("filewrite"))
            {
                Log.e("filewrite","Writing file");
                String NewUUID = intent.getStringExtra("NewUUID");
                filewrite(NewUUID);
            }
            */
            if(reply.equals("UpdateTextview"))
            {
                int BLEStatus = intent.getIntExtra("BLEStatus",0);
                if(BLEStatus != 0)isSetupComplpted = true;
                String BLEState = intent.getStringExtra("BLEState");
                UpdateTextview(BLEState,BLEStatus);
            }

            if(reply.equals("LoginInternetFail"))//도메인 에러 처리
            {
                if(!isTestingNewDomain)
                    ifLoginInternetFail();
                else
                    TerminateAlert(MyService.LoginName,getStringCode(790));//도메인 업테이트 후 재시도 실패. 앱 종료.
            }

            if(reply.equals("getDomainSucceed"))//도메인 얻기 성공하면
            {
                //getStringCodeHash.put("9995", intent.getStringExtra("domain"));//도메인을 업데이트한다.
                ifgetDomainSucceed();
            }
            if(reply.equals("NoClientCode"))//clientCode가 null이라면
            {
                Log.i("COMLST","COMLST start <- (NoClientCode)");
                isComlstfromNoClientCode = true;
                SendMessagetoMyService(MyService.COMLST);
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
            if(reply.equals("LoginNeedClientCode"))//login Rtn이 9가 온다면 : clientcode 선택 요망
            {
                Log.i("LoginNeedClientCode","");
                // 회사 리스트를 다이얼로그로 띄우고
                // 사용자의 선택을 받아
                // 로그인을 호출한다.
                if(loginNeedCliented)
                {
                    //2회 이상 들어옴. 앱을 종료한다.
                    terminate();
                    //TerminateAlert(MyService.ComlstName, "회사 선택 에러. 앱을 종료합니다.");
                    return;
                }
                loginNeedCliented = true;

                if(!isneedToSelectClientCode())// 기존의 선택했던 내용이 있는지 확인.
                {
                    companyList[0].code = privClientcode;
                    setclientCode(0);
                    AfterSettingClientCode();
                    return;
                }

                if(companyList.length == 1) {
                    setclientCode(0);
                    AfterSettingClientCode();
                    //SendMessagetoMyService(MyService.LOGIN);

                    return;
                }
                String[] strings = new String[companyList.length];
                for(int i=0;i<companyList.length;i++)
                    strings[i] = companyList[i].CompanyName;

                AtomicBoolean OnSelected = new AtomicBoolean(false);
                DataAdapter dataAdapter = new DataAdapter(strings, (index) ->{
                    if(dlg != null) dlg.dismiss();
                    setclientCode(index);
                    UpdateTextClientCode();
                    AfterSettingClientCode();
                    OnSelected.set(true);
                });

                dlg = new SelectClientDialog(this,dataAdapter);
                dlg.setListener(new SelectClientListener() {
                    @Override
                    public void OnDismiss() {
                        if(!OnSelected.get())
                            terminate();
                            //TerminateAlert(MyService.ComlstName, "회사 선택 취소. 앱을 종료합니다.");
                    }
                });
                dlg.show();
            }

            if(reply.equals("LoginSucceed"))
            {
                //clientcode = intent.getStringExtra("clientcode");
                UpdateTextClientCode();
                getStrTextUpdate();
                ((MyApplication)getApplication()).saveCodeData();

                try {//Login 성공 -> startTask를 시작한다.
                    mServiceMessenger.send(Message.obtain(null, MyService.STARTTASK));
                } catch (RemoteException e) {
                    e.printStackTrace();
                    TerminateAlert("Login", "APP Internal Error.");//"로그인 정회원 체크","startTask 실행 오류."
                }
            }
            if(reply.equals("Language") || reply.equals("LanguageFail"))//Language 성공하면
            {
                SelectColor();//색을 서버로부터 업데이트한다.
                setColor();//현재 액티비티에 색을 적용한다.

                try {//Language -> login를 시작한다.
                    mServiceMessenger.send(Message.obtain(null, MyService.LOGIN));
                } catch (RemoteException e) {
                    e.printStackTrace();
                    TerminateAlert("Login", "APP Internal Error.");//"startTask 실행 오류."
                }

            }
            if(reply.equals("QRCode"))
            {
                String URL = intent.getStringExtra("URL");
                Log.d("processIntent QRCode","URL :" + URL);

                Dialog dialog = new Dialog(this);
                dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                dialog.setContentView(R.layout.qrcode_dialog);
                WindowManager.LayoutParams params = dialog.getWindow().getAttributes();
                params.width = WindowManager.LayoutParams.MATCH_PARENT;
                params.height = WindowManager.LayoutParams.WRAP_CONTENT;
                dialog.getWindow().setAttributes(params);

                Button OKBtn = dialog.findViewById(R.id.OKBtn);
                OKBtn.setBackgroundColor(Color.parseColor(AppColor));
                OKBtn.setTextColor(Color.parseColor(TextColor));
                OKBtn.setOnClickListener((v)->{
                    dialog.dismiss();
                });

                ImageView QRCodeImg = dialog.findViewById(R.id.QRCode);
                GlideApp.with(this).load(URL).placeholder(R.drawable.ic_loading).error(R.drawable.ic_error).into(QRCodeImg);
                dialog.show();
            }
            if(reply.equals("BLERequire")) {
                CheckBluetooth();
            }
        }
    }

    private void ShowmeNewAccountPage() {
        ((MyApplication)getApplication()).ResetFile();
        NewAccountActivity.noBackMode = true;
        startActivity(new Intent(this,NewAccountActivity.class));
    }

    boolean isneedToSelectClientCode()
    {
        for(MyApplication.CompanyList company : companyList)
            if(company.code.equals(privClientcode))return false;
        privClientcode = "";
        return true;
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
            if(dlg != null) dlg.dismiss();
            setclientCode(index);
            UpdateTextClientCode();
            AfterSettingClientCode();
            OnSelected.set(true);
        });

        dlg = new SelectClientDialog(this,dataAdapter);
        dlg.setListener(new SelectClientListener() {
            @Override
            public void OnDismiss() {
                    //MessageSnack(MyService.ComlstName,"회사 선택 취소");
            }
        });
        dlg.show();
    }

    static boolean loginNeedCliented = false;
    static boolean isComlstfromNoClientCode = false;

    String privclient = "";
    void setclientCode(int position){
        clientcode = companyList[position].code;
        privClientcode = clientcode;
        Log.e("SetClientCode","clientcode -> " + clientcode);

        getStringCodeHash.put("10900",clientcode);//clientcode
        getStringCodeHash.put("10902",privClientcode);//privClientcode
        if(!privclient.equals(getStringCode(10900)))//"" 처음 이 곳으로 왔을 때 clientcode==null이라면 저장하지 않는다.
        {
            privclient = getStringCode(10900); // 두번째부터는 저장.
            Log.d("code 10900",getStringCode(10900));
            Log.d("code 10902",getStringCode(10902));
            ((MyApplication)getApplication()).ResetFile();
            ((MyApplication)getApplication()).saveCodeData();
            return;
        }
    }
    void AfterSettingClientCode(){//comlst 호출 후 사용자가 회사를 선택하면
        try {//언어를 호출한다. LANGUAGE -> LOGIN -> StartTask
            mServiceMessenger.send(Message.obtain(null, MyService.LANGUAGE));
        } catch (RemoteException e) {
            e.printStackTrace();
            TerminateAlert("Login", "APP Internal Error.");//"startTask 실행 오류."
        }

    }

    void requireBluetoothON()
    {
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter != null) {
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBLEIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBLEIntent, REQUEST_ENABLE_BT);
            } else StartTask();
        }else StartTask();
    }
    static boolean isCheckingBLE = false;
    void CheckBluetooth() {
        Init_aboutfile();
        if(isBLENeededtoON()) {
            isCheckingBLE = true;
            LEErrorCause();
            new AlertDialog.Builder(this).setTitle(getStringCode(10110,"비콘 기능이 꺼져 있습니다.")).setMessage(getStringCode(812,"블루투스를 잠깐동안 설정해 주시기 바랍니다."))//getStringCode(800)
                    .setPositiveButton("OK", (dialog, which) -> {})
                    .setOnDismissListener((dialog)-> requireBluetoothON()).create().show();
            return;
        }
        StartTask();
    }
    void StartTask()
    {
        StartBackground();

        // 서비스 호출
        Intent intent = new Intent(this,MyService.class);
        intent.putExtra("isNEW",false);
        mIsBound = bindService(intent,mConnection,Context.BIND_AUTO_CREATE);

        //QRCodeBtn.setOnClickListener(v->OnQRCodeBtn());
        QRCodeCtrl.setOnClickListener(v->OnQRCodeBtn());
        Toggle_IO.setOnClickListener(v->Toggle_AttendIO());
    }
    void StartBackground()
    {
        // 포그라운드 서비스 호출
        Intent serviceintent = new Intent(MainActivity.this,BackgroundService.class);
        startService(serviceintent);
    }
    boolean isINMode = true;//내근(IN) 외근(OUT) 모드
    void Toggle_AttendIO()
    {
        isINMode = !isINMode;
        getStrTextUpdate();
    }

    void OnQRCodeBtn()
    {
        if(isBackLocked())return;
        SendMessagetoMyService(MyService.QRCODE);
    }
    void ifLoginInternetFail()//로그인 때 인터넷이 실패하면 <- 도메인 데이터가 달라져서 연결이 안될 경우.
    {
        // 도메인 변경 부분 : 다 지우고 :
        // 여기로 왔을 때 10111의 도메인으로 다시 로그인한다.
        // 두번째: 앱 종료.

        getStringCodeHash.put("10112",getStringCode(10111));//현재 = 백업
        ifgetDomainSucceed();
    }

    boolean isTestingNewDomain = false; // 첫번째 로그인 실패 플래그 : 새로운 도메인으로 다시 로그인 체크 시도 한다면 true가 된다.(평상시 false)
    void ifgetDomainSucceed() {// 로그인 실패 후 도메인을 다시 불러오면
        isTestingNewDomain = true;
        AfterSettingClientCode();// LANGUAGE -> LOGIN -> StartTask
    }

    void getStrTextUpdate()
    {
        String usrType = getStringCode(10903); // {"S","student"}, {"T","teacher"}, {"P","professor"}, {"E","employee"}, {"M","manager"
        NowINI.setTitle(getStringCode(864));//내근
        //NowINO.setTitle(getStringCode(863));//외근
        NowOUT.setTitle(getStringCode(784));//퇴근
        //데이터가 들어 있는 상태에서
        //회사원이 아니면
        if(!usrType.equals("~") && !usrType.equals("E")) {
            Toggle_IO.setVisibility(View.GONE);//버튼 하나를 지운다.
            //NowINI.setTitle(getStringCode(783));//출근
        }else Toggle_IO.setVisibility(View.VISIBLE);

        if(clientcode.equals("1000") && usrType.equals("E"))
            OpenDoorBtn.setVisibility(View.VISIBLE);
        else OpenDoorBtn.setVisibility(View.GONE);

        copyrightView.setText(getStringCode(9999,getString(R.string.app_name)) + " (" + versionName + ")");
        Objects.requireNonNull(getSupportActionBar()).setTitle(getStringCode(9998));
        UpdateTextClientCode();

        //가운데 색, 좌우중 글씨 바꾸기
        if(isINMode)
        {
            Toggle_IO.setTitle(getStringCode(864));//내근
            NowOUT.setTitle(getStringCode(864) + getStringCode(784));//내근퇴근
            NowINI.setTitle(getStringCode(864) + getStringCode(783));//내근출근
            Toggle_IO.setBackgroundColor(Color.GRAY);
            Toggle_IO.setTextColor(Color.GRAY);
            NowINI.setOnClickListener(v->onNowINOUT_CLICKED(1));
            NowOUT.setOnClickListener(v->onNowINOUT_CLICKED(0));
        }
        else
        {
            Toggle_IO.setTitle(getStringCode(863));//외근
            NowOUT.setTitle(getStringCode(863) + getStringCode(784));//외근퇴근
            NowINI.setTitle(getStringCode(863) + getStringCode(783));//외근출근
            Toggle_IO.setBackgroundColor(Color.parseColor(AppColor));
            Toggle_IO.setTextColor(Color.BLACK);
            NowINI.setOnClickListener(v->onNowINOUT_CLICKED(2));
            NowOUT.setOnClickListener(v->onNowINOUT_CLICKED(3));

        }
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

    @Override
    protected void onStop() {
        isMainActivityAlive = false;//앱 상태를 보고한다.
        super.onStop();
    }

    @Override
    protected void onStart() {
        isMainActivityAlive = true;//앱 상태를 보고한다.
        super.onStart();
    }

    @Override
    protected void onPause() {
        isMainActivityAlive = false;//앱 상태를 보고한다.
        super.onPause();
    }

    @Override
    protected void onResume() {
        isMainActivityAlive = true;//앱 상태를 보고한다.
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        isMainActivityAlive = false;//앱 상태를 보고한다.
        if(mIsBound)
        {
            SendMessagetoMyService(MyService.DISCONNECT);

            unbindService(mConnection);
            mIsBound = false;
        }

        super.onDestroy();
    }

    void UpdateTextview(String BLEState,int BLEStatus)
    {
        //String txt = "";//String.format(getString(R.string.uuid_formatstr), (UUID.length()==0 ? "None" : UUID));
        //textview.setText(txt);


        BLEStateString = BLEState;
        int Imageid = R.drawable.ic_blestate_off;
        switch(BLEStatus)
        {
            case -2: Imageid = R.drawable.ic_error;break; //"NO"
            case -1: Imageid = R.drawable.ic_blestate_off;break; //"Turning ON.."
            case  0: Imageid = R.drawable.ic_blestate_turning;break; //"OFF(fail)"
            case  1: Imageid = R.drawable.ic_blestate_off;break; //"ON"
            case  2: Imageid = R.drawable.ic_blestate_on;break; //"OFF"
            default: return;
        }
        BLEStatusView.setImageDrawable(getResources().getDrawable(Imageid));

        //txt_BLEState.setText(BLEState);
    }
    void UpdateTextClientCode(){
        //Text_Clientcode.setText(getStringCode(134,"ClientCode") + " : " + clientcode);
        ClientCodeCtrl.setsubTitle(getStringCode(134,"ClientCode"));
        ClientCodeCtrl.setTitle(clientcode);
    }

    boolean fileread() {
        UUID = ReadUUID();
        if(UUID == null) return false;
        else return true;
    }


    private final int MY_PERMISSIONS_REQUEST_READ_PHONE_STATE=1001;
    public void permissionCheck() {
        int permissionCheckPHONE = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE);
        int permissionCheckINTERNET = ContextCompat.checkSelfPermission(this, Manifest.permission.INTERNET);
        int permissionCheckEXTSTORAGE = ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permissionCheckPHONE == PackageManager.PERMISSION_GRANTED && permissionCheckINTERNET == PackageManager.PERMISSION_GRANTED && permissionCheckEXTSTORAGE == PackageManager.PERMISSION_GRANTED) {
            CheckBluetooth();
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (shouldShowRequestPermissionRationale(Manifest.permission.READ_PHONE_STATE) || shouldShowRequestPermissionRationale(Manifest.permission.INTERNET) || shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                //"권한 요청 필요""로그인을 위한 전화번호와 인터넷 권한, 외부 저장소 권한이 필요합니다.\n앱을 이용하려면 권한에 모두 동의하시기 바랍니다."
                AtomicBoolean isContinued = new AtomicBoolean(false);
                builder.setTitle(getStringCode(10103)).setMessage(getStringCode(10104))
                        .setPositiveButton("continue", (dialog, which) -> {
                            isContinued.set(true);
                            dialog.dismiss();
                            permissionRequire();
                        })
                        .setOnDismissListener(dialog -> {
                            if(!isContinued.get())
                            terminate();
                        })
                        .create().show();

            } else {
                permissionRequire();
            }
        }
    }
    @RequiresApi(api = Build.VERSION_CODES.M)
    public void permissionRequire(){
        requestPermissions(
                new String[]{Manifest.permission.INTERNET,Manifest.permission.READ_PHONE_STATE,Manifest.permission.WRITE_EXTERNAL_STORAGE},
                MY_PERMISSIONS_REQUEST_READ_PHONE_STATE);
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch(requestCode)
        {
            case MY_PERMISSIONS_REQUEST_READ_PHONE_STATE :
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED
                        && grantResults[1] == PackageManager.PERMISSION_GRANTED
                        && grantResults[2] == PackageManager.PERMISSION_GRANTED) {
                    CheckBluetooth();
                }  else {
                    String err = " (";
                    if(grantResults.length == 0)err = "ERROR";
                    else {
                        if (grantResults[0] != 0) err += "Internet ";
                        if (grantResults[1] != 0) err += "PHONE ";
                        if (grantResults[2] != 0) err += "EXTSTORAGE ";
                        err += ")";
                    }//"Permission Denied""Permission denied.(phone number)"

                    terminate();
                    //TerminateAlert(getStringCode(766),getStringCode(10105) +  err);
                }
                return;
        }
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        switch (requestCode) {
            case REQUEST_ENABLE_BT:
                if (resultCode == RESULT_OK){
                    SendMessagetoMyService(MyService.BLE_ENABLED);
                    MyService.UserBLEDisabled = false;
                    StartTask();
                }
                else {
                    //user denied turning on Bluetooth

                    SendMessagetoMyService(MyService.BLE_DISABLED);
                    MyService.UserBLEDisabled = true;
                    StartTask();
                    //TerminateAlert("Bluetooth fail","Bluetooth fail");
                }
                isCheckingBLE = false;
                break;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }
    void terminate()
    {
        finishAffinity();
        System.runFinalization();
        System.exit(0);
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
    public void MessageAlert(String Title,String Messages)
    {
        new AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
                .setPositiveButton("OK", (dialog, which) -> {//OK
                }).create().show();
    }
    public void MessageSnack(String Title,String Messages)
    {
        mainLayout = findViewById(R.id.imageLogo);
        Snackbar snackbar = Snackbar.make(mainLayout, Title + "\n" + Messages, Snackbar.LENGTH_LONG);
        snackbar.setAction("OK", v->snackbar.dismiss());

        snackbar.show();//OK
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        m_menu = menu;

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if(isBackLocked())return super.onOptionsItemSelected(item);

        if(id == R.id.setting) {
            startActivity(new Intent(this,MenuActivity.class));
            return true;
        }
         /*
         if (id == R.id.btn_register) {//등록 절차를 폐기함.
            if(mIsBound)
                SendMessagetoMyService(MyService.REGISTER);
            return true;
        }else
        */
        /*
         if (id == R.id.btn_unregister) {
            if(mIsBound) new AlertDialog.Builder(this)
                    .setTitle(getStringCode(446)).setMessage(getStringCode(466))
                    .setPositiveButton(getStringCode(446),(dialog, which)->
                            SendMessagetoMyService(MyService.UNREGISTER)).create().show();
            return true;
        }else if (id == R.id.btn_attendlist) {
            startActivity(new Intent(this,MenuActivity.class));
            //startActivity(new Intent(this,Activity_AttendList.class));
            return true;
        }else if(id == R.id.btn_apsetting) {
            startActivity(new Intent(this, APSettingActivity.class));
            return true;
        }else if(id == R.id.person) {
            Intent intent = new Intent(this, AccountActivity.class);
            startActivity(intent);
            return true;
        }
        */

            return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        if(!isBackLocked())
        super.onBackPressed();
        else Toast.makeText(this, "Please Wait..", Toast.LENGTH_SHORT).show();
    }
    boolean isBackLocked()//뒷버튼 사용 제한
    {
        return MyService.isAPIing || !isSetupComplpted;
    }
}
