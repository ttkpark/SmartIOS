package com.example.inout_nodeapp;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import org.json.JSONException;
import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.Objects;
import java.util.Optional;
import java.util.Random;

import static com.example.inout_nodeapp.APIClass.RESULT_OK;
import static com.example.inout_nodeapp.APIClass.TaskName;
import static com.example.inout_nodeapp.APIClass.privTaskName;
import static com.example.inout_nodeapp.CryptUtil.Base64Encode;
import static com.example.inout_nodeapp.MyApplication.APIResult;
import static com.example.inout_nodeapp.MyApplication.APIRtnErrorCode;
import static com.example.inout_nodeapp.MyApplication.GatePositionToRemark;
import static com.example.inout_nodeapp.MyApplication.SimpleGateTimeList;
import static com.example.inout_nodeapp.MyApplication.accountData;
import static com.example.inout_nodeapp.MyApplication.clientcode;
import static com.example.inout_nodeapp.MyApplication.companyList;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.getStringCodeHash;
import static com.example.inout_nodeapp.MyApplication.idxUser;
import static com.example.inout_nodeapp.MyApplication.loginType;

public class MyService extends Service {
    View mainView;
    String UUID = "";
    static boolean logined = false;

    Class<?> SendClass = null;
    String phone = "";

    APIClass APIsender;
    CryptUtil crypt;

    int BLEStatus = 0;
    boolean isNEW = true;

    boolean isFirst = false;
    Messenger mMessenger = null;

    static final int CONNECT = 0;
    static final int DISCONNECT = 1;
    static final int SEND_VALUE = 2;

    static final int START = 6;
    static final int LANGUAGE = 13;
    static final int STARTTASK = 3;
    static final int REGISTER = 4;
    static final int UNREGISTER = 5;
    static final int QRCODE = 12;
    static final int LISTING = 7;
    static final int GETADMIN = 8;
    static final int APSetting_General = 9;
    static final int APSetting_IP = 10;
    static final int APSetting_Domain = 11;
    static final int DoorOpen = 14;
    static final int person_check = 15;
    static final int person_modify = 16;
    static final int person_useYN = 17;
    static final int GetDomain = 18;
    static final int LOGIN = 19;
    static final int COMLST = 20;
    static final int LISTING_SIMPLE = 21;
    static final int LISTING_REMARK = 22;
    static final int NEW_ACCOUNT = 23;
    static final int LISTING_REMARK_CANCEL = 24;

    static final int BLE_DISABLED = 30;
    static final int BLE_ENABLED = 31;

    static final int ACTIVITY_NULL = 119;
    static final int ACTIVITY_1 = 120;
    static final int ACTIVITY_2 = 121;
    static final int ACTIVITY_3 = 122;
    static final int ACTIVITY_APSettingActivity_general = 123;
    static final int ACTIVITY_APSettingActivity_IP = 124;
    static final int ACTIVITY_APSettingActivity_domain = 125;
    static final int ACTIVITY_ACCOUNT = 126;
    static final int ACTIVITY_ACCOUNTSetting = 127;
    static final int ACTIVITY_MENU = 128;
    static final int ACTIVITY_LISTING_SIMPLE = 129;
    static final int ACTIVITY_NEW_ACCOUNT = 130;


    static boolean UserBLEDisabled = false;

    public Messenger mBLEMessenger;
    final Messenger mMessengerBLE = new Messenger(new ServiceHandler());
    private ServiceConnection mConnectionBLE = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mBLEMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,BackgroundService.CONNECT);
                msg.replyTo = mMessengerBLE;
                mBLEMessenger.send(msg);

                msg = Message.obtain(null,
                        BackgroundService.SEND_VALUE,123456789,0);
                mBLEMessenger.send(msg);

                mBLEMessenger.send(Message.obtain(null,BackgroundService.BEACON_RESET,UUID));
                mBLEMessenger.send(Message.obtain(null,BackgroundService.BEACON_Status));
                UpdateTextview();
            }catch (RemoteException e){}
        }

        @Override
        public void onServiceDisconnected(ComponentName name) { mBLEMessenger = null; }
    };

    class ServiceHandler extends Handler {
        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                case BackgroundService.SEND_VALUE:
                    //MessageAlert("BackgroundService로부터","메시지 받아옴." + msg.obj);
                    break;
                case BackgroundService.BEACON_Status:
                    BLEStatus = msg.arg1;
                    break;
                case BackgroundService.BEACON_RESET:
                    UpdateAfterReply();
                    isAPIing = false;

                    APIResult = RESULT_OK;
                    if(Objects.equals(privTaskName, LoginName))
                        LoginSucceed();
                    break;
                case BackgroundService.UPDATETEXT:
                    UpdateTextview();
                    break;


                case CONNECT:
                    mMessenger = msg.replyTo;
                    ClearisAPIing();
                    break;

                case DISCONNECT:
                    mMessenger = null;
                    break;

                case SEND_VALUE:
                    int val = msg.arg1;

                    Optional.ofNullable(mMessenger)
                            .ifPresent(messenger -> {
                                try {
                                    messenger.send(Message.obtain(null,SEND_VALUE,val,0));
                                } catch (RemoteException e) {
                                    e.printStackTrace();
                                }
                            });
                    break;

                case GetDomain:API_getDomain();break;
                case STARTTASK:startTask();break;
                case REGISTER:API_register();break;
                case UNREGISTER:API_unregister();break;
                case START:Start(msg);break;
                case LOGIN:API_login();break;
                case COMLST:API_COMLST();break;
                case LISTING:API_Listing((String) msg.obj);break;
                case LISTING_SIMPLE:API_SimpleListing((String) msg.obj);break;
                case LISTING_REMARK:API_Listing_Remark(msg.arg1);break;
                case LISTING_REMARK_CANCEL:API_Listing_RemarkCancel(msg.arg1);break;
                case GETADMIN:API_GetAdmin();break;
                case APSetting_General:API_SetAPGeneral((MyApplication.InformationSettingData)msg.obj);break;
                case APSetting_IP:API_SetAPIP((MyApplication.IPSettingData)msg.obj);break;
                case APSetting_Domain:API_SetAPDomain((MyApplication.DomainSettingData)msg.obj);break;
                case QRCODE:MakeQRCodeUrl();break;
                case LANGUAGE:API_language();break;
                case DoorOpen:API_DoorOpen();break;
                case person_check: API_person_check(); break;
                case person_modify: API_person_modify(); break;
                case person_useYN: API_person_useYN((String)msg.obj); break;
                case NEW_ACCOUNT: API_New_Account((MyApplication.NewAccountData)msg.obj); break;

                case ACTIVITY_1:SendClass = MainActivity.class;break;
                case ACTIVITY_2:SendClass = Activity_AttendList.class;break;
                case ACTIVITY_3:SendClass = APSettingActivity.class;break;
                case ACTIVITY_APSettingActivity_general:SendClass = APSettingActivity_general.class;break;
                case ACTIVITY_APSettingActivity_IP:SendClass = APSettingActivity_IP.class;break;
                case ACTIVITY_ACCOUNT:SendClass = AccountActivity.class;break;
                case ACTIVITY_ACCOUNTSetting:SendClass = AccountSettingActivity.class;break;
                case ACTIVITY_MENU:SendClass = MenuActivity.class;break;
                case ACTIVITY_LISTING_SIMPLE:SendClass = Activity_simpleAttendList.class;break;
                case ACTIVITY_NEW_ACCOUNT:SendClass = NewAccountActivity.class;break;
                case ACTIVITY_NULL:SendClass = null;break;

                case BLE_DISABLED: BLEDisabled(); break;
                case BLE_ENABLED: BLEEnabled(); break;

                default: super.handleMessage(msg); break;
            }
        }

    }

    void LoginSucceed() {
        privTaskName = "";
        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","LoginSucceed");
        startActivity(sendIntent);//Login 성공 -> startTask를 시작한다.
    }

    private void BLEEnabled() {
        Log.i("BLEEnabled()","in MyService");
        UserBLEDisabled = false;

        if( mBLEMessenger != null)
            try {
                mBLEMessenger.send(Message.obtain(null,BackgroundService.BLE_ENABLED));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
    }

    private void BLEDisabled() {
        Log.i("BLEDisabled()","in MyService");
        UserBLEDisabled = true;

        if( mBLEMessenger != null)
            try {
                mBLEMessenger.send(Message.obtain(null,BackgroundService.BLE_DISABLED));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
    }


    final Messenger mServiceMessenger = new Messenger(new ServiceHandler());

    @Override
    public IBinder onBind(Intent intent) {
        return mServiceMessenger.getBinder();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        //Toast.makeText(this,"service starting",Toast.LENGTH_SHORT).show();

        return START_STICKY;
    }

    void Start(Message msg) {

        UUID = (String)msg.obj;
        isFirst = (msg.arg1 == 1);
        isNEW = msg.arg2 == 1;
        Login();
    }
    @Override
    public void onCreate() {
        super.onCreate();
    }
    @Override
    public void onDestroy() {
        try {
            if(mBLEMessenger != null)
                mBLEMessenger.send(Message.obtain(null,BackgroundService.DISCONNECT));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        super.onDestroy();
    }

    public void Login() {
        crypt = new CryptUtil();
        APIsender = new APIClass(this);

        idxUser = Integer.parseInt(getStringCode(10904));
        if (phone == null)phone = " ";

        if(!isNEW){startTask();return;}//중복 로그인 방지

        if (clientcode == null || Objects.equals(clientcode, ""))
        {
            Log.w("MyServicelogin","No Clientcode");
            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply","NoClientCode");
            startActivity(sendIntent);
        }
        else API_language();//API_login();
    }
    boolean isstarted = false;
    public void startTask()
    {
        if(!isstarted) {
            Intent serviceintent = new Intent(MyService.this, BackgroundService.class);
            boolean isBind = bindService(serviceintent, mConnectionBLE, Context.BIND_AUTO_CREATE);
            if (!isBind) MessageAlert("", "Bind Error");

            if (!isFirst) StartBLE();
        }
        if(!isNEW)
        {
            StartBLE();
        }
        isstarted = true;
    }
    void MakeQRCodeUrl() {
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String QIs = new String(Base64Encode(Base64Encode(crypt.AESEncode("QI",AESNo),0),0));
        String QRData = String.format("%s/%s/%d",UUIDs,QIs,Numaes);

        String URL = String.format(getString(R.string.QRCODE),QRData);

        Intent sendintent = new Intent(this,SendClass);
        sendintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendintent.putExtra("reply","QRCode");
        sendintent.putExtra("URL",URL);
        startActivity(sendintent);
    }

    public void MessageSnack(String Title, String Message) {
        if(SendClass == null)return;

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","MessageSnack");
        sendIntent.putExtra("Title",Title);
        sendIntent.putExtra("Message",Message);
        startActivity(sendIntent);
    }
    public void MessageAlert(String Title,String Message) {
        if(SendClass == null)return;

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","MessageAlert");
        sendIntent.putExtra("Title",Title);
        sendIntent.putExtra("Message",Message);
        startActivity(sendIntent);
    }
    public void MessageTerminate(String Title,String Message) {
        if(SendClass == null)return;

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","MessageTerminate");
        sendIntent.putExtra("Title",Title);
        sendIntent.putExtra("Message",Message);
        startActivity(sendIntent);
    }
    public void MessageToast(String Title,String Message) {
        Toast.makeText(this, Title + " : " + Message, Toast.LENGTH_SHORT).show();
    }
    public void UpdateTextview() {
        String txt = "";//String.format(getString(R.string.uuid_formatstr), (UUID.length()==0 ? "None" : UUID));

        Log.e("BEACON","BLEStatus : " + BLEStatus);
        txt = String.format(getString(R.string.BLE_STAT), getBeaconStatus());

        if(SendClass == null)return;
        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","UpdateTextview");
        sendIntent.putExtra("BLEState",txt);
        sendIntent.putExtra("BLEStatus",BLEStatus);
        startActivity(sendIntent);
    }

    void filewrite(String NewUUID) {
        getStringCodeHash.put("10901",NewUUID);//UUID
        ((MyApplication)getApplication()).saveCodeData();
        /*
        FileOutputStream fo;

        try {
            fo = openFileOutput(MainActivity.filename,Context.MODE_PRIVATE);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return;
        }
        try {
            fo.write(NewUUID.getBytes());
            fo.close();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
        Log.i("filewrite","line");

        return;
        */
    }
    //String LoginURL;
    //String RegisterURL;
    //String LanguageURL;
    //String UnregisterURL;
    //String APIListingURL;
    //String APIDoorOpenURL;
    //String APIGetAdminURL;
    //String APISetAPURL;
    //String APISetAPDomainURL;

    static String LoginName = "Login";
    static String ComlstName = getStringCode(824);//"회사 선택"
    static String LanguageName = "Language";//"언어 데이터 가져오기";
    static String registerName = getStringCode(303);//"등록";
    static String unregisterName = getStringCode(446);//"탈퇴";
    static String listingName = getStringCode(773);//"입자별 입출입";
    static String simplelistingName = "simpleListing";//"simpleListing";
    static String listingRemarkName = "simpleRemark";//"simpleListing";
    static String RemarkCancelName = "simpleRemarkCancel";//"simpleListing";
    static String GetAdminName = getStringCode(10108);//"[관리자] 등록 및 삭제";
    static String SetAPName = "AP Setting";
    static String DoorOpenName = getStringCode(806);//"문 열기"
    static String getDomainName = "Get Domain";
    static String UsrCheckName = "Get Account";
    static String UsrModifyName = "Modify Account";
    static String UsruseYNName = "Account useYN";
    static String NewAccountName = "New Account";

    void RefreshText() {
        ComlstName = getStringCode(824);//"회사 선택"
        registerName = getStringCode(303);//"등록";
        unregisterName = getStringCode(446);//"탈퇴";
        listingName = getStringCode(773);//"입자별 입출입";
        GetAdminName = getStringCode(10108);//"[관리자] 등록 및 삭제";
        DoorOpenName = getStringCode(806);
    }

    Message reqMSG;
    int AESNo;

    static boolean isAPIing = false;
    void ClearisAPIing()
    {
        isAPIing = false;
    }

    private void API_getDomain() {//URLGetDomain_Host
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(getDomainName);return;}
        TaskName = getDomainName;
        restartAPISender();
        int Numaes = GetnerateAESKey();
        Random random = new Random();
        random.setSeed(System.currentTimeMillis());

        String id = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(clientcode,AESNo),0),0)));
        String val = String.format("%d%d",random.nextInt(9000)+1000,Numaes);
        String val2 = "dm";

        APIsender.URLPage = getString(R.string.URLcdm);
        APIsender.Data = String.format(getString(R.string.str_param3), id,val,val2);
        //APISetAPURL = URLGet;
        //APISetAPDomainURL = URLGet;

        AESLog(TaskName,AESNo);
        APIsender.URLHost = getStringCode(10111);
        APIsender.start();
    }
    void API_COMLST() {
        if(clientcode == null) clientcode = "0";
        APIregister("comlst",ComlstName);
        if(clientcode.equals("0"))clientcode = null;
    }
    void API_login() {
        APIregister("loginhp",LoginName);
    }
    void API_language() {
        APIregister("langv",LanguageName);
    }
    void API_register() {
        APIregister("in",registerName);
    }
    void API_unregister() {
        APIregister("out",unregisterName);
    }
    void API_DoorOpen() {
        /*
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(DoorOpenName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String date = new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(datestring,AESNo),0),0));
        String UUIDs = new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        String URLGet = String.format(getString(R.string.URLDoorOpen), TextUtils.htmlEncode(date), TextUtils.htmlEncode(UUIDs),presuffix);
        APIsender.URLGet = URLGet;
        APIDoorOpenURL = URLGet;

        AESLog(DoorOpenName,AESNo);
        TaskName = DoorOpenName;
        APIsender.URLHost = getStringCode(9995);
        APIsender.start();
        */
    }
    void API_Listing(String datestring) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(listingName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        if(datestring == null || datestring.length() != 8) {
            @SuppressLint("SimpleDateFormat")
            SimpleDateFormat mFormat = new SimpleDateFormat("yyyyMMdd");
            datestring = mFormat.format(new Date(System.currentTimeMillis()));
        }

        String date = new String(Base64Encode(Base64Encode(crypt.AESEncode(datestring,AESNo),0),0));
        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = "list";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param4), TextUtils.htmlEncode(date), TextUtils.htmlEncode(UUIDs),presuffix,suffix);

        //APIListingURL = URLGet;

        AESLog(listingName,AESNo);
        TaskName = listingName;
        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_SimpleListing(String datestring) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(simplelistingName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        if(datestring == null || datestring.length() != 8) {
            @SuppressLint("SimpleDateFormat")
            SimpleDateFormat mFormat = new SimpleDateFormat("yyyyMMdd");
            datestring = mFormat.format(new Date(System.currentTimeMillis()));
        }
        Random random = new Random();
        random.setSeed(Calendar.getInstance().getTimeInMillis());

        //String date = new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(datestring,AESNo),0),0));
        String APName = new String(Base64Encode(Base64Encode(crypt.AESEncode("2013",AESNo),0),0));
        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String randomNum = String.valueOf(random.nextInt(90) + 10);
        String suffiix = "smlst";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param6), TextUtils.htmlEncode(UUIDs),TextUtils.htmlEncode(APName),datestring,randomNum,presuffix,suffiix);
                //APIListingURL = URLGet;

        AESLog(simplelistingName,AESNo);
        TaskName = simplelistingName;
        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_Listing_Remark(int Yesone) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(listingRemarkName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String date = new String(Base64Encode(Base64Encode(crypt.AESEncode(GatePositionToRemark.time,AESNo),0),0));
        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = (Yesone==3)?"mkoo":(Yesone==2)?"mkoi":(Yesone==1)?"mkii":"mkio";
        //state 3=외근퇴근 2=외근출근 1=내근 0=퇴근

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param4), TextUtils.htmlEncode(date),TextUtils.htmlEncode(UUIDs),presuffix,suffix);

        AESLog(listingRemarkName,AESNo);
        TaskName = listingRemarkName;
        APIsender.URLHost = getStringCode(10110);
        APIsender.start();
    }
    void API_Listing_RemarkCancel(int Yesone) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(RemarkCancelName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String date = new String(Base64Encode(Base64Encode(crypt.AESEncode(GatePositionToRemark.time,AESNo),0),0));
        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = (Yesone==1)?"mkci":"mkco";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param4), TextUtils.htmlEncode(date),TextUtils.htmlEncode(UUIDs),presuffix,suffix);

        AESLog(RemarkCancelName,AESNo);
        TaskName = RemarkCancelName;
        APIsender.URLHost = getStringCode(10110);
        APIsender.start();
    }
    void API_GetAdmin() {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(GetAdminName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String UUIDs = new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = "adm";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param3), TextUtils.htmlEncode(UUIDs),presuffix,suffix);
        //APIGetAdminURL = URLGet;

        AESLog(GetAdminName,AESNo);
        TaskName = GetAdminName;

        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_SetAPGeneral(MyApplication.InformationSettingData data) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(SetAPName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String UUIDs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(UUID,AESNo),0),0)));
        String APIDs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(data.str_ModuleName,AESNo),0),0)));
        String SensType = data.SensorType;//TextUtils.htmlEncode(new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(data.SensorType,AESNo),0),0)));
        String Dist = data.str_ModuleDistance;//TextUtils.htmlEncode(new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(data.str_ModuleDistance,AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        String suffix = data.str_suffix;

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param6), UUIDs,APIDs,SensType,Dist,presuffix,suffix);
        //APISetAPURL = URLGet;

        AESLog(SetAPName,AESNo);
        TaskName = SetAPName;

        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_SetAPIP(MyApplication.IPSettingData data) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(SetAPName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        byte[] IPArray = new byte[data.IPARRAYSize];
        System.arraycopy(data.IP,0,IPArray,0,4);
        System.arraycopy(data.DefGate,0,IPArray,4,4);
        System.arraycopy(data.SubMask,0,IPArray,8,4);
        System.arraycopy(data.DNS1,0,IPArray,12,4);
        System.arraycopy(data.DNS2,0,IPArray,16,4);
        String IPsBase64 = new String(Base64Encode(IPArray,0));

        String UUIDs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode("10"/*UUID*/,AESNo),0),0)));
        String APIDs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(data.APCode,AESNo),0),0)));
        String IPData = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(IPsBase64,AESNo),0),0)));
        String SensType = data.SensorType;//TextUtils.htmlEncode(new String(crypt.Base64Encode(crypt.Base64Encode(crypt.AESEncode(data.SensorType,AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        String suffix = "ipc";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param6), UUIDs,APIDs,SensType,IPData,presuffix,suffix);
        //APISetAPURL = URLGet;

        AESLog(SetAPName,AESNo);
        TaskName = SetAPName;

        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_SetAPDomain(MyApplication.DomainSettingData data) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(SetAPName);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String APIDs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(data.APCode,AESNo),0),0)));
        String DNSs = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(data.domain,AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        String suffix = "dmc";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param4), APIDs,DNSs,presuffix,suffix);
        //APISetAPURL = URLGet;
        //APISetAPDomainURL = URLGet;

        AESLog(SetAPName,AESNo);
        TaskName = SetAPName;

        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_person_check() {
        APIregister("pers",UsrCheckName);
    }
    void API_person_useYN(String suffix) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(UsruseYNName);return;}
        TaskName = UsruseYNName;
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String id = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(phone,AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param3), id,presuffix,suffix);
        //APIsender.URLPage = URLGet;

        AESLog(TaskName,AESNo);
        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_person_modify() {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(UsrModifyName);return;}
        TaskName = UsrModifyName;
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String id = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(phone,AESNo),0),0)));
        String val = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(accountData.Name,AESNo),0),0)));
        String id2 = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(accountData.ID,AESNo),0),0)));
        String val2 = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode("1111",AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = "pers";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param6), id,val,id2,val2,presuffix,suffix);
        //APIsender.URLPage = URLGet;

        AESLog(TaskName,AESNo);
        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }
    void API_New_Account(MyApplication.NewAccountData obj) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(NewAccountName);return;}
        TaskName = NewAccountName;
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String id = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(obj.phone,AESNo),0),0)));
        String val = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(obj.Name,AESNo),0),0)));
        String id2 = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(obj.ID,AESNo),0),0)));
        String val2 = obj.Type;
        String id3 = obj.Language;
        String val3 = TextUtils.htmlEncode(new String(Base64Encode(Base64Encode(crypt.AESEncode(obj.Password,AESNo),0),0)));
        String presuffix = MakePreSuffix(clientcode,Numaes);
        String suffix = "join";

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param8), id,val,id2,val2,id3,val3,presuffix,suffix);
        //APIsender.URLPage = URLGet;

        AESLog(TaskName,AESNo);
        APIsender.URLHost = getStringCode(10112);
        APIsender.start();
    }


    void busyAPIMessage(String Title){MessageSnack(Title,"명령이 실행중입니다");}

    void APIregister(String suffix,String MessageTitle) {
        if(!isAPIing)isAPIing = true; else {busyAPIMessage(MessageTitle);return;}
        restartAPISender();
        int Numaes = GetnerateAESKey();

        String hp = new String(Base64Encode(Base64Encode(crypt.AESEncode(phone,AESNo),0),0));
        String presuffix = MakePreSuffix(clientcode,Numaes);

        APIsender.URLPage = getString(R.string.URLatt);
        APIsender.Data = String.format(getString(R.string.str_param3), TextUtils.htmlEncode(hp),presuffix,suffix);
        //APIsender.URLPage = URLGet;

        AESLog(MessageTitle,AESNo);
        APIsender.URLHost = getStringCode(10112);
        TaskName = MessageTitle;
        APIsender.start();
        //return URLGet;
    }
    int GetnerateAESKey(){
        Calendar cal = Calendar.getInstance();
        int month = cal.get(Calendar.MONTH) + 1;
        int day = cal.get(Calendar.DAY_OF_MONTH);

        Random random = new Random();
        random.setSeed(cal.getTimeInMillis());

        int randomVal = random.nextInt(9000)+1000;//1000~9999
        AESNo = ((month + day)/2) + randomVal%10;//1~30
        return randomVal%10;
    }
    void restartAPISender(){
        if(APIsender != null)
        if(APIsender.isAlive())
            APIsender.interrupt();
        APIsender = new APIClass(this);
    }
    void AESLog(String TAG,int AESNum) {
        Log.i(TAG,"AESNo : " + AESNum);
        Log.i(TAG,"AESIV : " + CryptUtil.BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum-1][0],0)));
        Log.i(TAG,"AESKey : " + CryptUtil.BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum-1][1],0)));
    }
    String MakePreSuffix(String clientcode,int AESNo) {
        try {
            int i = Integer.parseInt(clientcode);
        }catch (NumberFormatException e)
        {
            clientcode = "0";
        }
        
        return String.format(Locale.KOREA,"%s%d",clientcode,AESNo);
    }

    void Requests() {

        if(APIsender.replyOBJ == null)
                Log.e("Request received","JSON : NULL");
        else {
            Log.i("Request received", APIsender.reply);
            Log.i(TaskName, "JSON : " + APIsender.replyOBJ.toString());
        }

        if(TaskName.equals(LoginName)) {
            APIResult = APIreply_Login();
            if(APIResult == 9)
            {
                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply","ComlstNoAccount");
                startActivity(sendIntent);
            }
        }

        else if(TaskName.equals(ComlstName)) {
            APIResult = APIreply_Comlst();

            if(!APIClass.isInternetFail(APIResult)) {
                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                if(APIResult == 9)//No account
                    sendIntent.putExtra("reply","ComlstNoAccount");
                else if(APIResult != RESULT_OK)
                    sendIntent.putExtra("reply","ComlstFail");
                else sendIntent.putExtra("reply","ComlstSucceed");
                startActivity(sendIntent);
            }
        }

        else if(TaskName.equals(getDomainName))
            APIResult = APIreply_getDomain();

        else if(TaskName.equals(LanguageName)) {
            APIResult = APIreply_Language();

            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            if(APIResult != RESULT_OK)
                 sendIntent.putExtra("reply","LanguageFail");
            else sendIntent.putExtra("reply","Language");
            startActivity(sendIntent);
        }

        else if(TaskName.equals(registerName))
            APIResult = APIreply_register();//->Waiting for request.. BEACON_RESET and Update textview

        else if(TaskName.equals(unregisterName))
            APIResult = APIreply_unregister();

        else if(TaskName.equals(DoorOpenName))
            APIResult = APIreply_doorOpen();

        else if(TaskName.equals(listingName))
            APIResult = APIreply_listing();

        else if(TaskName.equals(simplelistingName))
            APIResult = APIreply_simplelisting();

        else if(TaskName.equals(listingRemarkName))
            APIResult = APIreply_listingRemark();

        else if(TaskName.equals(RemarkCancelName))
            APIResult = APIreply_listingRemark();

        else if(TaskName.equals(NewAccountName))
            APIResult = APIreply_NewAccount();

        else if(TaskName.equals(SetAPName)) {
            APIResult = APIreply_SetAP();


            if(APIResult == RESULT_OK) {
                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply", "Succeed");
                startActivity(sendIntent);
            }
        }

        else if(TaskName.equals(GetAdminName)) {
            APIResult = APIreply_GetAdmin();
            if(APIResult == RESULT_OK) {
                Intent sendIntent = new Intent(this, SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply", "GetAdmin");
                startActivity(sendIntent);
            }
        }
        else if(TaskName.equals(UsrCheckName))
            APIResult = APIreply_UsrCheck();

        else if(TaskName.equals(UsrModifyName))
            APIResult = APIreply_UsrModify();

        else if(TaskName.equals(UsruseYNName))
            APIResult = APIreply_UsruseYN();
        else
            Log.i("error","ee");

        isAPIing = false;
    }


    int APIreply_Comlst() {
        if(reqMSG.what == 1)
        {
            if(APIClass.isInternetFail(reqMSG.arg1))
            {
                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply","LoginInternetFail");
                startActivity(sendIntent);

            }else MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));//error
            return reqMSG.arg1;
        }

        try {
            int rtncode = Integer.parseInt(APIsender.replyOBJ.getString("code"));

            if(rtncode==9)// No Accounts in our smartios.
            {
                return rtncode;
            }

            String errorCode = APIRtnErrorCode(rtncode);
            if (rtncode != 1) {
                //fail
                MessageAlert(TaskName,errorCode);
                return rtncode;
            }
        } catch (JSONException e) { }

        // 리스트가 넘어온다면

        final int countArr = APIsender.replyArr.length();
        final int companyListCount = countArr-1;//회사 개수

        companyList = new MyApplication.CompanyList[companyListCount];
        try {
            for(int i=0;i<companyListCount;i++)
            {
                JSONObject obj = APIsender.replyArr.getJSONObject(i+1);

                companyList[i] = new MyApplication.CompanyList();
                companyList[i].code = obj.getString("code");
                companyList[i].CompanyName = obj.getString("cn");
            }
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        /*
        int len = 12;
        companyList = new MyApplication.CompanyList[len];
        for(int i=0;i<len;i++)
        {
            companyList[i] = new MyApplication.CompanyList();
            companyList[i].code = String.format("%d",1001+i);
            companyList[i].CompanyName = String.format("회사%d",i+1);
        }
        */


        return RESULT_OK;
    }


    int APIreply_UsrCheck() {
        if(reqMSG.what == 1)
        {
            MessageTerminate(TaskName,APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        int rtncode;
        try {
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        String errorCode = APIRtnErrorCode(rtncode);
        if (rtncode != 1) {
            //fail
            MessageTerminate(TaskName,errorCode);
            return rtncode;
        }
        accountData = new MyApplication.AccountData();
        try {
            accountData.Name = APIsender.replyOBJ.getString("name");
            accountData.ID = APIsender.replyOBJ.getString("id");
            accountData.lang = APIsender.replyOBJ.getString("langv");
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageTerminate(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        //MessageSnack(TaskName,errorCode);

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","GetPersSucceed");
        startActivity(sendIntent);

        return RESULT_OK;
    }

    int APIreply_UsrModify() {
        int res = APIreply_EndIFfailed();

        if(res == RESULT_OK) {
            Intent sendIntent = new Intent(this, SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply", "UsrModifySucceed");
            startActivity(sendIntent);
        }
        return res;
    }
    int APIreply_UsruseYN() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);
        if (rtn.rtncode != 1) {
            //fail
            MessageAlert(TaskName,rtn.Errormsg);
            return rtn.rtncode;
        }

        //MessageSnack(TaskName,"OK");



        stopService(new Intent(getApplicationContext(),BackgroundService.class));

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","AppTerminate");
        sendIntent.putExtra("Title",TaskName);
        sendIntent.putExtra("Message",getStringCode(790));//"앱을 종료합니다."
        startActivity(sendIntent);

        return RESULT_OK;
    }

    int APIreply_Login() {
        if(reqMSG.what == 1)
        {
            if(APIClass.isInternetFail(reqMSG.arg1))
            {
                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply","LoginInternetFail");
                startActivity(sendIntent);

            }else MessageTerminate(TaskName, APIClass.EndedinFail(reqMSG.arg1));//error
            return reqMSG.arg1;
        }

        int rtncode;
        try {
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageTerminate(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        //comlst에서 하는 일로 곂쳐서 삭제한다.
        /*
        if(rtncode == 9) // rtncode = 9일 때 -
        {
            String clientcode;
            try {
                clientcode = APIsender.replyOBJ.getString("clientcode");
            } catch (JSONException e) {
                e.printStackTrace();//    "JSON Error\n"
                MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
                return APIClass.RESULT_JSON_FAIL;
            }
            String[] clientcodes = clientcode.split(",");

            if(clientcodes.length == 0)
            {
                MessageTerminate(TaskName,getStringCode(762,"등록되지 않았습니다."));
                return rtncode;
            }
            companyList = new MyApplication.CompanyList[clientcodes.length];
            for(int i=0;i<clientcodes.length;i++)
            {
                companyList[i] = new MyApplication.CompanyList();
                companyList[i].code = clientcodes[i];
                companyList[i].CompanyName = clientcodes[i];
            }

            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply","LoginNeedClientCode");
            startActivity(sendIntent);//Login 성공 -> startTask를 시작한다.
            return rtncode;
        }
        */
        String errorCode = APIRtnErrorCode(rtncode);
        if (rtncode != 1) {
            //fail
            if(rtncode != 9)
            MessageTerminate(TaskName,errorCode);
            return rtncode;
        }

        /* //login시 clientcode를 받았던 코드
        String clientCode;
        try {
             clientCode = APIsender.replyOBJ.getString("clientcode");
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageTerminate(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        //MessageSnack(TaskName,errorCode);

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","LoginSucceed");
        //sendIntent.putExtra("clientcode",clientCode);
        startActivity(sendIntent);//Login 성공 -> startTask를 시작한다.
        */
        privTaskName = TaskName;
        String EncodedUUID = "";
        String UserType = "";
        try{
            EncodedUUID = APIsender.replyOBJ.getString("nodecode");
            UserType = APIsender.replyOBJ.getString("utype");
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageTerminate(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        for(int i=0;i< loginType.length;i++)
        {
            if(UserType.equals(loginType[i][1])){//employee
                UserType = loginType[i][0];//->E
                break;
            }
            if(i==loginType.length-1)
                UserType = "E";
        }

        String strUUID = new String(crypt.AESDecode(Base64.decode(Base64.decode(EncodedUUID.getBytes(),0),0)));

        getStringCodeHash.put("10903",UserType);
        if(strUUID.length() == 0) {
            //fail
            MessageTerminate(TaskName,getStringCode(10107));//"Decoding UUID Error"
            return MyApplication.UUID_DECODE_ERROR;
        }
        UUID = strUUID;
        filewrite(UUID);

        //MessageSnack(TaskName,errorCode);

        Update_BLEReset();

        if(mBLEMessenger == null)
            LoginSucceed();

        return APIClass.RESULT_NONE;// 텍스트 업데이트를 기다린다.
    }

    int APIreply_Language() {
        if(reqMSG.what == 1)
        {
            //error
            //MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        try {
            int rtncode;
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));
            String errorCode = APIRtnErrorCode(rtncode);
            if (rtncode != 1) {
                //fail
                MessageAlert(TaskName,errorCode);
                return rtncode;
            }
        } catch (JSONException ignored) { }


        try{
            for(int i=0;i<APIsender.replyArr.length();i++)
            {
                JSONObject obj = APIsender.replyArr.getJSONObject(i);
                getStringCodeHash.put(String.valueOf(Integer.parseInt(obj.getString("code"))),obj.getString("val"));
                //Log.i(LanguageName,obj.toString());
            }
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        ((MyApplication)getApplication()).LangvSucceed();
        RefreshText();

        return RESULT_OK;
    }

    int APIreply_register() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        String EncodedUUID = "";
        int rtncode;
        try {
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106)+ APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        String errorCode = APIRtnErrorCode(rtncode);
        if (rtncode != 1) {
            //fail
            MessageAlert(TaskName,errorCode);
            return rtncode;
        }

        try{
            EncodedUUID = APIsender.replyOBJ.getString("nodecode");
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }

        String strUUID = new String(crypt.AESDecode(Base64.decode(Base64.decode(EncodedUUID.getBytes(),0),0)));
        if(strUUID.length() == 0) {
            //fail
            MessageAlert(TaskName,getStringCode(10107));//"Decoding UUID Error"
            return MyApplication.UUID_DECODE_ERROR;
        }
        UUID = strUUID;
        filewrite(UUID);

        //MessageSnack(TaskName,errorCode);

        privTaskName = TaskName;
        Update_BLEReset();

        return APIClass.RESULT_NONE;
    }
    int APIreply_unregister() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);
        if (rtn.rtncode != 1) {
            //fail
            MessageAlert(TaskName,rtn.Errormsg);
            return rtn.rtncode;
        }

        UUID = "";
        filewrite(UUID);

        //MessageSnack(TaskName,"OK");

        requestBeaconReset();
        UpdateAfterReply();
        ((MyApplication)getApplication()).ResetFile();

        //stopService(new Intent(getApplicationContext(),BackgroundService.class));

        //MessageTerminate(TaskName,getStringCode(790));//"앱을 종료합니다."

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","unregister");
        startActivity(sendIntent);//Login 성공 -> startTask를 시작한다.

        return RESULT_OK;
    }
    int APIreply_listing() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);

        if(rtn.rtncode != 01) {
            //{"rtncode" : "15" , "time" : "" , "gatename" : "출입 데이터가 없습니다."}
            String errorName = null;
            try {
                errorName = APIsender.replyOBJ.getString("gatename");
            } catch (JSONException ignored) { }// It throws Exception when "gatename" isn't there.

            if (errorName != null) {
                //Show Message in the Dialog.
                MessageAlert(TaskName, errorName);

                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply","ListFailed");
                startActivity(sendIntent);

                return rtn.rtncode;
            }

            if (rtn.rtncode != 1) {
                //fail
                MessageAlert(TaskName,rtn.Errormsg);
                return rtn.rtncode;
            }
        }


        try {
            int NumRecorded = APIsender.replyArr.length();
            GateTimeList = new MyApplication.GatePosition[NumRecorded];

            for(int i=0;i<NumRecorded;i++)
            {
                JSONObject obj  = APIsender.replyArr.getJSONObject(i);
                GateTimeList[i] = new MyApplication.GatePosition();
                GateTimeList[i].time = obj.getString("time");
                GateTimeList[i].gatename = obj.getString("gatename");

                //Log.i("List[" + i + "/" + NumRecorded + "]","time : " + GateTimeList[i].time + " gatename :" + GateTimeList[i].gatename);
            }

            //MessageSnack(TaskName,"OK");

            MyApplication.GateTimeList = GateTimeList;

            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply","List");
            startActivity(sendIntent);
        }
        catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }

        return RESULT_OK;
    }
    int APIreply_simplelisting() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);

        if(rtn.rtncode != 01) {
            //{"rtncode" : "15" , "time" : "" , "gatename" : "출입 데이터가 없습니다."}
            String errorName = null;
            try {
                errorName = APIsender.replyOBJ.getString("gatename");
            } catch (JSONException ignored) { }// It throws Exception when "gatename" isn't there.

            if (errorName != null) { //Show Message in the Dialog.
                MessageAlert(TaskName, errorName);

                Intent sendIntent = new Intent(this,SendClass);
                sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                sendIntent.putExtra("reply","ListFailed");
                startActivity(sendIntent);

                return rtn.rtncode;
            }

            if (rtn.rtncode != 1) {
                //fail
                MessageAlert(TaskName,rtn.Errormsg);
                return rtn.rtncode;
            }
        }
//  {  "seq":"0","io_dt":"2021-03-26 03:25:56.559", "io_type":"io_min_o", "gatename" : "출입문", "io":"" }
//, {  "seq":"1","io_dt":"2021-03-26 08:55:30"    , "io_type":"io_min"  , "gatename" : "출입문", "io":"i" }
//, {  "seq":"2","io_dt":"2021-03-26 20:10:10"    , "io_type":"io_max"  , "gatename" : "출입문", "io":"" }
//, {  "seq":"3","io_dt":"", "io_type":"io_max_o", "gatename" : "" , "io":"o"}

        try {
            int NumRecorded = APIsender.replyArr.length();
            SimpleGateTimeList = new MyApplication.SimpleGatePosition[NumRecorded];

            //String[] ioType = {"io_min_o","io_min","io_max","io_max_o"};
            // JSON으로 넘어오는 4가지 타입들을
            // 각각 GateTileList의 0,1,2,3에 해당시켜 저장한다.
            for(int i=0;i<NumRecorded;i++)
            {
                JSONObject obj  = APIsender.replyArr.getJSONObject(i);
                int focus = 0;
                /*
                for(int q=0;q<ioType.length;q++)
                {
                    if(ioType[q].equals(obj.getString("io_type")))// 같다면
                        focus = q;
                }
                */
                focus = obj.getInt("seq");

                SimpleGateTimeList[focus] = new MyApplication.SimpleGatePosition();
                SimpleGateTimeList[focus].io_type = obj.getString("io_type");
                SimpleGateTimeList[focus].time = obj.getString("io_dt");
                SimpleGateTimeList[focus].gatename = obj.getString("gatename");
                SimpleGateTimeList[focus].io = obj.getString("io");

                //Log.d("List[" + i + "/" + NumRecorded + "]","time : " + GateTimeList[i].time + " gatename :" + GateTimeList[i].gatename);
            }
            for(MyApplication.SimpleGatePosition g : SimpleGateTimeList)
            {
                if(g == null)
                {
                    MessageAlert(TaskName,getStringCode(99));//시스템 오류. -> NULL인 데이터가 있다.
                    return 99;
                }
            }

            //MessageSnack(TaskName,"OK");

            //MyApplication.GateTimeList = GateTimeList;

            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply","List");
            startActivity(sendIntent);
        }
        catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }

        return RESULT_OK;
    }
    int APIreply_GetAdmin() {
        if(reqMSG.what == 1)
        {
            //error
            MessageTerminate(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);
        if (rtn.rtncode != 1) {
            //fail
            MessageTerminate(TaskName,rtn.Errormsg);
            return rtn.rtncode;
        }

        try {
            MyApplication.APInfo[] APInfoList;
            int NumRecorded = APIsender.replyArr.length();
            int NumAP = NumRecorded-1;
            APInfoList = new MyApplication.APInfo[NumAP];

            for(int i=0;i<NumAP;i++)
            {
                JSONObject obj  = APIsender.replyArr.getJSONObject(i+1);
                APInfoList[i] = new MyApplication.APInfo();
                APInfoList[i].apname = obj.getString("apname");
                APInfoList[i].apcode = obj.getString("apcode");
                APInfoList[i].dist = obj.getString("dist");
                APInfoList[i].useyn = obj.getString("useyn");
                APInfoList[i].flag = obj.getString("flg");
                APInfoList[i].IPAddr = obj.getString("ip");
                APInfoList[i].sensType = obj.getString("sty");
                Log.i(TaskName,obj.toString());
            }

            getStringCodeHash.put("10112",APIsender.replyOBJ.getString("apname"));//AP Domain Update!!

            //MessageSnack(TaskName,"OK");

            MyApplication.APInfoList = APInfoList;
        }
        catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }

        return RESULT_OK;
    }
    int APIreply_listingRemark() {
        int res = APIreply_EndIFfailed();
        if(res == RESULT_OK)
        {
            Intent sendIntent = new Intent(this,SendClass);
            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            sendIntent.putExtra("reply","remarkSucceed");
            startActivity(sendIntent);
        }
        return res;
    }
    int APIreply_SetAP() {
        return APIreply_EndIFfailed();
    }
    int APIreply_doorOpen() {
        return APIreply_EndIFfailed();
    }
    //중계서버를 통해 도메인을 가져오는 API.
    int APIreply_getDomain() {
        String Errorment =  "\n" + getStringCode(790,"The app is to shut down.");//에러가 발생하면 앱이 종료된다. 그 때 출력하는 멘트.
        if(reqMSG.what == 1)
        {
            MessageTerminate(TaskName,APIClass.EndedinFail(reqMSG.arg1) + Errorment);
            return reqMSG.arg1;
        }

        int rtncode;
        try {
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + Errorment);
            return APIClass.RESULT_JSON_FAIL;
        }
        String errorCode = APIRtnErrorCode(rtncode);
        if (rtncode != 1) {
            //fail
            MessageTerminate(TaskName,errorCode + Errorment);
            return rtncode;
        }

        String Enc_domain;
        try {
            Enc_domain = APIsender.replyOBJ.getString("dm");
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageTerminate(TaskName,getStringCode(10106) + Errorment);
            return APIClass.RESULT_JSON_FAIL;
        }
        //MessageSnack(TaskName,errorCode);
        String domain = new String(crypt.AESDecode(Base64.decode(Base64.decode(Enc_domain,0),0)));

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply","getDomainSucceed");
        sendIntent.putExtra("domain",domain);
        startActivity(sendIntent);

        return RESULT_OK;
    }
    int APIreply_NewAccount() {
        if(reqMSG.what == 1)
        {
            MessageTerminate(TaskName,APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }
        int rtncode;
        try {
            rtncode = Integer.parseInt(APIsender.replyOBJ.getString("rtncode"));

            String errorCode = APIRtnErrorCode(rtncode);
            if (rtncode != 1) {
                //fail
                MessageAlert(TaskName,errorCode);
                return rtncode;
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        try{
            for(int i=0;i<APIsender.replyArr.length();i++)
            {
                JSONObject obj = APIsender.replyArr.getJSONObject(i);
                getStringCodeHash.put(String.valueOf(Integer.parseInt(obj.getString("code"))),obj.getString("val"));
                //Log.i(LanguageName,obj.toString());
            }
        } catch (JSONException e) {
            e.printStackTrace();//    "JSON Error\n"
            MessageAlert(TaskName,getStringCode(10106) + APIsender.replyOBJ.toString());
            return APIClass.RESULT_JSON_FAIL;
        }
        ((MyApplication)getApplication()).LangvSucceed();
        RefreshText();
        //language code

        Intent sendIntent = new Intent(this,SendClass);
        sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        sendIntent.putExtra("reply", "NewAccountSucceed");
        startActivity(sendIntent);

        return RESULT_OK;
    }

    int APIreply_EndIFfailed() {
        if(reqMSG.what == 1)
        {
            //error
            MessageAlert(TaskName, APIClass.EndedinFail(reqMSG.arg1));
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);
        if (rtn.rtncode != 1) {
            //fail
            if(rtn.Errormsg == null || rtn.Errormsg.length() > 0)
                rtn.Errormsg = getStringCode(rtn.rtncode);
            MessageAlert(TaskName,rtn.Errormsg);
            return rtn.rtncode;
        }

        //MessageSnack(TaskName,"OK");
        return RESULT_OK;
    }
    int APIreply_simple() {
        if(reqMSG.what == 1)
        {
            return reqMSG.arg1;
        }

        RtnError rtn = ErrorMessageWith_rtnmsg(APIsender.replyOBJ);
        if (rtn.rtncode != 1) {
            return rtn.rtncode;
        }
        return RESULT_OK;
    }
    int APIreply_onlyMessage(String SucceedMessage,String FailedMessage) {//API 응답 후 리턴 형식. 메시지만 보냄.
        int res = APIreply_simple();
        Intent intent = new Intent(this,SendClass);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        if(res != RESULT_OK)
            intent.putExtra("reply",FailedMessage);
        else intent.putExtra("reply",SucceedMessage);
        return res;
    }

    RtnError ErrorMessageWith_rtnmsg(JSONObject obj) {
        RtnError rtn = new RtnError();

        try {
            rtn.rtncode = Integer.parseInt(obj.getString("rtncode"));
        } catch (JSONException e) {
            e.printStackTrace();
            rtn.rtncode = 1;
            rtn.Errormsg = "OK";
            return rtn;
        }

        try {
            rtn.Errormsg = obj.getString("rtnmsg");
        } catch (JSONException e) {
            e.printStackTrace();
            if(rtn.rtncode == 1) {
                rtn.Errormsg = "OK";
            }
            return rtn;
        }

        return rtn;
    }
    static class RtnError {
        int rtncode;
        String Errormsg;
    }
    void UpdateAfterReply() {
        UpdateTextview();
    }

    void StartBLE() {
        requestBeaconReset();
    }
    void Update_BLEReset() {
        requestBeaconReset();
    }

    String getBeaconStatus()
    {
        return BLEBeacon.strBLEStatus(BLEStatus);
    }
    void requestBeaconReset() {
        if(mBLEMessenger == null){Log.i("myService","mBLEMessenger == null");return;}
        try {
            mBLEMessenger.send(Message.obtain(null,BackgroundService.BEACON_RESET,UUID));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }


    Handler mHandler = new Handler(){
        public void handleMessage(Message msg)
        {
            switch (msg.what){
                case 1: /*EndedinFail(msg.arg1);*/
                case 2: reqMSG = msg; Requests();
            }
        }
    };

    MyApplication.GatePosition[] GateTimeList;
}
