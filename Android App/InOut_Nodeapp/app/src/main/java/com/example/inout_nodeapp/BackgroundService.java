package com.example.inout_nodeapp;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import org.altbeacon.beacon.BeaconTransmitter;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.Random;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.idxUser;
import static com.example.inout_nodeapp.MyApplication.isMainActivityAlive;

public class BackgroundService extends Service {
    CryptUtil crypt;
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        Log.d("Background","onBind");
        return mServiceMessenger.getBinder();
    }

    @Override
    public void onCreate() {
        crypt = new CryptUtil();
        if(beacon == null)
            InitBLE();
        else Log.e("Error","Beacon is already alive.");
        idxUser = Integer.parseInt(getStringCode(10904));

        beacon.makedata(makedata());
        beacon.onCreate(UUID);

        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("Background","onStartCommand");
        Notifi_M = (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);
        start();

        return START_STICKY;
    }

    //서비스가 종료될 때 할 작업
    @Override
    public void onDestroy() {
        Log.d("Background","onDestory");
        stop();
        beacon.onDestory();
        super.onDestroy();
    }
    void RestartThread()
    {
        //start();
    }
    public void start() {
        myNotificationHandler handler = new myNotificationHandler();
        handler.initalizeNotification(this);
        handler.sendMessage(Message.obtain(null,0,"App Start"));

        if(thread != null) thread.stopForever();
        thread = new ServiceThread(handler, this);
        thread.start();
    }
    public void stop() {
        myNotificationHandler handler = new myNotificationHandler();
        if(thread != null) thread.stopForever();
        thread = new ServiceThread( handler,this );
        thread.stopForever();
    }

    //출처: https://shihis123.tistory.com/entry/Android-Background-ServiceThread-백그라운드-서비스-유지하기Notification [Gomdori]

    NotificationManager Notifi_M;
    ServiceThread thread;
    int BLEStatus = 0;
    static BLEBeacon beacon;
    static final int CONNECT = 100;
    static final int DISCONNECT = 101;
    static final int SEND_VALUE = 102;
    static final int BEACON_RESET = 105;
    static final int BEACON_Status = 106;
    static final int BLE_DISABLED = 107;
    static final int BLE_ENABLED = 108;
    static final int UPDATETEXT = 110;
    private class myMessageHandler extends Handler {
        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what) {
                case CONNECT:
                    mMessenger = msg.replyTo;
                    Log.w("CONNECT","Restread");
                    break;
                case DISCONNECT:
                    mMessenger = null;
                    Log.w("DISCONNECT","RestartThread");
                    break;

                case BEACON_RESET:
                    ThreadBeaconReset((String)msg.obj);
                    break;
                case BEACON_Status:
                    SendBLEStatus();
                    break;
                case UPDATETEXT:
                    RequireUpdateTextview();
                    break;
                case BLE_DISABLED:
                    beacon.BLEstatus = -1;
                    beacon.onDestory();
                    RequireUpdateTextview();
                    break;
                case BLE_ENABLED:
                    ThreadBeaconReset(UUID);
                    RequireUpdateTextview();
                    break;
            }
        }
    }

    final Messenger mServiceMessenger = new Messenger(new myMessageHandler());
    Messenger mMessenger = null;
    boolean isFirst = false;
    void InitBLE() {
        isFirst = false;
        //if (!fileread() || UUID.length() < 5) isFirst = true;

        Log.w("BEACON", "Init");
        beacon = new BLEBeacon(this,(s)->{ BLEStatus = s; SendBLEStatus();});//It sets BLEStatus change callback.

        //Log.w("BEACON",String.format("isFirst:%b",isFirst));
        //if(!isFirst)
        //    beaconON(UUID);
    }
    private void CallMainActivity() {
        Intent intent = new Intent(BackgroundService.this,MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        intent.putExtra("reply","BLERequire");
        startActivity(intent);
        MainActivity.isCheckingBLE = true;
    }

    public void RequireUpdateTextview() {
        if(!isMainActivityAlive || mMessenger == null)return;
        try {
            mMessenger.send(Message.obtain(null,UPDATETEXT));
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }
    int GetnerateAESNo(){
        Calendar cal = Calendar.getInstance();
        int month = cal.get(Calendar.MONTH) + 1;
        int day = cal.get(Calendar.DAY_OF_MONTH);

        Random random = new Random();
        random.setSeed(cal.getTimeInMillis());

        int randomVal = random.nextInt(10);//0~9
        return ((month + day)/2) + randomVal;//1~30
    }
    int BLEDataCount = 0;
    byte[] makedata()
    {
        int AESNo = GetnerateAESNo();

        SimpleDateFormat dateformat = new SimpleDateFormat("yyyyMMdd", Locale.getDefault());
        Date now = new Date();


//        String seedphone = phoneNumber.replace("-","");
//        if(seedphone.length()%2 == 1)seedphone += '?';

//        String Date = dateformat.format(now);

        if(BLEDataCount > 60000)BLEDataCount = 0;
        String string_x = dateformat.format(now) + String.format("%04X",BLEDataCount++) + String.format("%08X",idxUser);
        Log.i("Send",string_x);


        int iterator = (string_x.length()+1)/2;
        byte[] x_encoded = new byte[iterator];
        for(int i=0;i<iterator;i++)
        {
            String sub = string_x.substring(i*2);
            if(sub.length() == 1)sub = sub.charAt(0) + "?";
            else sub = sub.substring(0,2);

            x_encoded[i] = (byte) ((byte)(sub.charAt(0)-'0')<<4 | (byte)(sub.charAt(1)-'0'));
        }
        Log.i("Send",crypt.BytesToString(x_encoded));

        beacon.aesNo = AESNo;

        AESLog("Beacon(1~30)",AESNo);

        //> (x) Base64(AES암호화.aes_no(yyyyMMdd+난수 값(0001 ~ 9999))
        //         난수 값은 매번 변경, 단 일일이내 난수값이 중복 될 수는 없다는 전제하...
        //> (y) Base64(AES암호화.aes_no(hp))
        //> (z) manufacture : 0xEFFF (AJUAttender 고유 코드) + aes_no

        // yyyyMMddNNNNiiiiiiii (i:idxUser,y:year,M:month,d:date,N:count)
        //12Bytes

        return crypt.AESEncode(x_encoded,AESNo);
    }
    void AESLog(String TAG,int AESNum) {
        Log.i(TAG,"AESNo : " + AESNum);
        Log.i(TAG,"AESIV : " + CryptUtil.BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum-1][0],0)));
        Log.i(TAG,"AESKey : " + CryptUtil.BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum-1][1],0)));
    }
    void BeaconReset(String UUID) {
        if(MyService.UserBLEDisabled)
        {
            Log.e("BEACON","Disabled by USER.");
        }else
        this.UUID = UUID;

        Log.w("BEACON", "DeInit");
        beacon.onDestory();

        beacon.makedata(makedata());
        beacon.onCreate(UUID);
        Log.w("BEACON", "ON");

        BLEStatus = beacon.BLEstatus;

        if(!isMainActivityAlive || mMessenger == null)return;
        try {
            mMessenger.send(Message.obtain(null,BEACON_RESET,BLEStatus,0));
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }
    void SendBLEStatus() {
        if(!isMainActivityAlive || mMessenger == null)return;
        try {
            mMessenger.send(Message.obtain(null,BEACON_Status,BLEStatus,0));
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }
    void ThreadBeaconReset(String UUID) {
        this.UUID = UUID;
        thread.setUUID(UUID);
    }


    static String UUID = "";

    public class myNotificationHandler extends Handler {
        BackgroundService parent;
        Intent intent;
        PendingIntent pendingIntent;
        //Uri soundUri;
        NotificationManager notificationManager;
        Notification.Builder notificationBuilder;

        void initalizeNotification(BackgroundService m_parent)
        {
            parent = m_parent;
            if(getStringCode(9996).equals("~"))
            {
                //Log.e("Notification","NO URL for site!");
                //intent = new Intent(BackgroundService.this,MainActivity.class);
                //intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            }
            else {
                //Log.e("Notification","Site setting succeed!");
                //String url = getStringCode(9996) + "?hp=" + telephone.replace("-","");
                //intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            }
            intent = new Intent(BackgroundService.this, WebViewActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);


            pendingIntent = PendingIntent.getActivity(BackgroundService.this,0,intent, 0);

            //soundUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);

            notificationManager = (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);

            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
            {
                notificationManager.createNotificationChannel(
                        new NotificationChannel("my_notification","n_channel",NotificationManager.IMPORTANCE_LOW));
            }

            if (android.os.Build.VERSION.SDK_INT >= 28)
                notificationBuilder = new Notification.Builder(BackgroundService.this,"my_notification");
            else notificationBuilder = new Notification.Builder(BackgroundService.this);

            notificationBuilder.setSmallIcon(R.mipmap.ic_launcher)
                    .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_launcher))
                    .setSmallIcon(R.drawable.ic_notification)
                    .setContentTitle(getStringCode(9998,getString(R.string.app_name)))
                    .setAutoCancel(false)//알림 클릭 시 자동 삭제
                    //.setSound(soundUri)
                    .setContentTitle(getStringCode(9998,"We are waiting for you to login!"))
                    .setContentText(getStringCode(9997,""))
                    .setContentIntent(pendingIntent)
                    .setDefaults(Notification.DEFAULT_ALL)
                    .setOnlyAlertOnce(true)
                    .setColor(Color.parseColor(AppColor));

            startForeground(1,notificationBuilder.build());
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            if(notificationBuilder == null) return;//initalizeNotification();


            if(!privInited && MainActivity.isSetupComplpted)
            {
                privInited = true;
                initalizeNotification(parent);
            }
            else
            notificationBuilder.setContentTitle(getStringCode(9998))
                    .setContentText(getStringCode(9997,""));

            startForeground(1,notificationBuilder.build());
            //notificationManager.notify(, notificationBuilder.build());
        }
    }
    static boolean privInited = false;
    void CheckBLEAlive()
    {

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            int flag = BeaconTransmitter.checkTransmissionSupported(this);
            if (flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS || flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER) {
                if (!MyService.UserBLEDisabled && !MainActivity.isCheckingBLE) {
                    Log.e("BLE", "NOT_SUPPORTED_CANNOT_GET_ADVERTISER");
                    Log.e("BLE", "MyService.UserBLEDisabled : " + MyService.UserBLEDisabled);
                    //LE가 비활성화됨.->Main을 부른다.
                    CallMainActivity();
                }
            } /*else {

                //꺼져있는지 확인.
                //블루투스가 꺼지면 비콘을 다시 켠다.

                BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                if (!mBluetoothAdapter.isEnabled()) {
                    if (isBluetoothON) {
                        // 블루투스를 켠다.
                        thread.setUUID(UUID);
                        Log.e("BLE", "Restarting BLE.");

                        isBluetoothON = false;
                    }
                } else isBluetoothON = true;

            } */ // 30초마다 획인하는걸로 바뀌었다.
        }
    }

}

class ServiceThread extends Thread
{
    BackgroundService main;
    Handler handler;
    boolean isRun = true;
    public ServiceThread(Handler h,BackgroundService m) {
        main = m;
        handler = h;
        //main.ThreadInitBLE();
        handler.sendMessage(Message.obtain(null,0,String.format("Start:%b",main.isFirst)));
        Log.i("Thread",String.format(" Start:%b",main.isFirst));
    }
    public void stopForever(){
        synchronized (this)
        {
            isRun = false;
            handler.sendMessage(Message.obtain(null,0,"End"));
            Log.i("Thread",String.format(" End : %d",System.currentTimeMillis()));
            //main.ThreadDeInitBLE();
        }
    }
    int loopcount = 0;
    public void run()
    {
        while(isRun /*&& !isShut*/)
        {
            loopcount++;
            if(loopcount%12 ==3) {
                handler.sendMessage(Message.obtain(null, 0, ((int) System.currentTimeMillis() / 1000) % 2, 0, String.format(" run : %d %b", loopcount, isRun)));
            }
            if(loopcount%20 ==1) {
                main.CheckBLEAlive();
                setUUID(UUID); //10초마다 블루투스 재시도
                // 블루투스를 3연속 껐다 켜면 비콘 신호가 끊어진다.
                // 10초마다 강제로 블루투스를 재시도한다.
            }
            try{
                Thread.sleep(500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            if(BLERebootFlag == 1)
            {
                UUID = "";

                BLERebootFlag = 0;
                if(UUID != null) {
                    main.BeaconReset(UUID);
                    /*
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    */
                }
            }
        }
        Log.i("Thread",String.format(" Out : %d",System.currentTimeMillis()));
    }
    void setUUID(String UUID) {
        this.UUID = UUID;
        BLERebootFlag = 1;
    }
    int BLERebootFlag = 0;
    String UUID;
}