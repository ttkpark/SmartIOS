package com.example.myfirstkotlin

import android.app.*
import android.bluetooth.BluetoothAdapter
import android.content.ContentValues
import android.content.ContentValues.TAG
import android.content.Context
import android.content.Intent
import android.graphics.BitmapFactory
import android.graphics.Color
import android.os.*
import android.util.Log
import android.widget.Toast
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.BLEDataCount
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.myNotification
import com.google.android.gms.tasks.OnCompleteListener
import com.google.firebase.messaging.FirebaseMessaging

class BackgroundService : Service() {

    override fun onBind(intent: Intent?): IBinder = MyBinder(this)

    var crypt = CryptUtil()

    override fun onCreate() {
        idxUser = try{getStringCode(9988).toInt()}catch(_:Throwable){0}
        BLEDataCount = try{getStringCode(10905).toInt()}catch(_:Throwable){0}

        thread = BLEBroadcast(this)
        super.onCreate()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.d("Background", "onStartCommand")
        if(idxUser == 0) { onDestroy();return START_NOT_STICKY }

        start()
        return START_STICKY
    }

    //서비스가 종료될 때 할 작업
    override fun onDestroy() {
        Log.d("Background", "onDestory")
        stop()
        //startActivity(Intent(this,LoadingActivity::class.java))
        super.onDestroy()
    }

    fun start() {
        myNotification.initalizeNotification(this)
        myNotification.sendMessage(Message.obtain(null, 0, "App Start")) // TODO : String Conversion
        Initfirebase()
        if(!thread.isAlive) {
            //thread.stopForever()
            thread.start()
        }
        thread.processAttendanceCount()
    }

    fun Initfirebase(){
        FirebaseMessaging.getInstance().token.addOnCompleteListener(OnCompleteListener { task ->
            if (!task.isSuccessful) {
                Log.w(ContentValues.TAG, "Fetching FCM registration token failed", task.exception)
                return@OnCompleteListener
            }

            // Get new FCM registration token
            val token = task.result

            // Log and toast
            val msg = getString(R.string.msg_token_fmt, token)
            Log.d(ContentValues.TAG, msg)
            Toast.makeText(baseContext, msg, Toast.LENGTH_SHORT).show()
        })
    }
    fun stop() {
        Log.w("BEACON", "stopForever")
        thread.stopForever()
    }

//    private fun CallMainActivity() {
//        val intent = Intent(this@BackgroundService, MainActivity::class.java)
//        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_SINGLE_TOP)
//        intent.putExtra("reply", "BLERequire")
//        startActivity(intent)
//    }

    lateinit var thread : BLEBroadcast

    val Notification_Foreground_ID = 1
    val Notification_Alert_ID = 2
    var privInited = false

    //출처: https://shihis123.tistory.com/entry/Android-Background-ServiceThread-백그라운드-서비스-유지하기Notification [Gomdori]
    class myNotificationHandler : Handler(Looper.getMainLooper()) {
        lateinit var parent: BackgroundService
        var intent: Intent? = null
        var pendingIntent: PendingIntent? = null

        //Uri soundUri;
        lateinit var notificationManager: NotificationManager
        lateinit var notificationBuilder: Notification.Builder
        fun initalizeNotification(m_parent: BackgroundService?) {
            parent = m_parent ?: return
            if(idxUser == 0) return

            // Intent for clicking
            if (getStringCode(9996) == "~") {
                //Log.e("Notification","NO URL for site!");
                //intent = new Intent(BackgroundService.this,MainActivity.class);
                //intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            } else {
                //Log.e("Notification","Site setting succeed!");
                //String url = getStringCode(9996) + "?hp=" + telephone.replace("-","");
                //intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            }
            intent = Intent(parent, LoadingActivity::class.java)
            intent!!.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
            pendingIntent = PendingIntent.getActivity(parent, 0, intent, 0)

            //soundUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
            notificationManager = parent.getSystemService(NOTIFICATION_SERVICE) as NotificationManager
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                notificationManager.createNotificationChannel(
                    NotificationChannel(
                        "InOut_Check_Notification",
                        "InOut Check",
                        NotificationManager.IMPORTANCE_LOW
                    )
                )
            }
            notificationBuilder =
                if (Build.VERSION.SDK_INT >= 28)
                    Notification.Builder(parent,"InOut_Check_Notification")
                else
                    Notification.Builder(parent)
            notificationBuilder.setSmallIcon(R.mipmap.ic_launcher)
                .setLargeIcon(BitmapFactory.decodeResource(parent.resources, R.mipmap.ic_launcher))
                .setSmallIcon(R.drawable.ic_notification)
                //.setContentTitle(getStringCode(9998, parent.getString(R.string.app_name)))
                .setAutoCancel(false) //알림 클릭 시 자동 삭제
                .setContentIntent(pendingIntent)
                //.setSound(soundUri)
                .setOnlyAlertOnce(true)
                .setColor(Color.parseColor(AppColor))
            parent.startForeground(parent.Notification_Foreground_ID, notificationBuilder.build())
        }

        override fun handleMessage(msg: Message) {
            if (!parent.privInited) {
                parent.privInited = true
                initalizeNotification(parent)
            } else {
                if(msg.what == 3)
                {
                    val messages = msg.obj as Array<*>
                    if(messages.size != 2) return

                    notificationBuilder.setContentTitle(messages[0].toString())
                        .setContentText(messages[1].toString())

                    parent.startForeground(parent.Notification_Foreground_ID, notificationBuilder.build())
                    //notificationManager.notify(parent.Notification_Foreground_ID, notificationBuilder.build())
                }
                else if(msg.what == 2){
                    parent.thread.processAttendanceCount()
                }
            }
        }
    }


    companion object
    {
        @JvmStatic
        var isBluetoothON = true
    }

    fun CheckBLEAlive() {
        if (!BluetoothAdapter.getDefaultAdapter().isEnabled) {//is bluetooth off?
            if (isBluetoothON) //check our ONOFF inspection value
                onBluetoothOff()
        }
        else {//is bluetooth on?
            if (!isBluetoothON) //check our ONOFF inspection value
                onBluetoothOn()
        }
    }
    fun onBluetoothOff()
    {
        Log.e("BEACON", "User BLE OFF")
        thread.isBLEEnabled = false
        thread.callOnChangeCallback(thread.BEACON_FAILED)

        BLUETOOTHAlert()

        thread.BLEStop()
        isBluetoothON = false
    }
    fun onBluetoothOn()
    {
        thread.BLEInit()
        val notificationManager = this.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        notificationManager.cancel(Notification_Alert_ID)

        isBluetoothON = true
    }
    fun BLUETOOTHAlert()
    {

        val intent = Intent(this,LoginActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_SINGLE_TOP)
        val pendingIntent = PendingIntent.getActivity(this, 0, intent, 0)

        val notificationManager = getSystemService(NOTIFICATION_SERVICE) as NotificationManager
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
            notificationManager.createNotificationChannel(
                NotificationChannel("my_notification","n_channel",NotificationManager.IMPORTANCE_HIGH))

        val notificationBuilder =
            if (Build.VERSION.SDK_INT >= 28)
                Notification.Builder(this,"my_notification")
            else
                Notification.Builder(this)

        notificationBuilder.setSmallIcon(R.mipmap.ic_launcher)
            .setContentIntent(pendingIntent)
            .setLargeIcon(BitmapFactory.decodeResource(resources, R.mipmap.ic_launcher))
            .setSmallIcon(R.drawable.ic_error)
            .setContentTitle(getStringCode(9998, getString(R.string.app_name)))
            .setContentText(getString(R.string.bluetooth_down_message))
            .setColor(Color.RED)
            .setAutoCancel(false) //알림 클릭 시 자동 삭제

        notificationManager.notify(Notification_Alert_ID, notificationBuilder.build())
    }

}
class MyBinder(m:BackgroundService) : Binder(){
    var parent = m
    fun GetBLE():BLEBroadcast{
        return parent.thread
    }
}