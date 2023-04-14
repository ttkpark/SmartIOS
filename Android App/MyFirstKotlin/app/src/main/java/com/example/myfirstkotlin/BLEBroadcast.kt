package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.le.AdvertiseCallback
import android.bluetooth.le.AdvertiseSettings
import android.content.Context
import android.content.pm.PackageManager
import android.os.Looper
import android.util.Base64
import android.util.Log
import com.example.myfirstkotlin.CryptUtil.BytesToString
import com.example.myfirstkotlin.MyApplication.Companion.BLEDataCount
import com.example.myfirstkotlin.MyApplication.Companion.BLEDuplicateEnabled
import com.example.myfirstkotlin.MyApplication.Companion.changeSaveCode
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.duplicateUserArray
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.makeNotification
import org.altbeacon.beacon.Beacon
import org.altbeacon.beacon.BeaconParser
import org.altbeacon.beacon.BeaconTransmitter
import org.json.JSONException
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.*


class BLEBroadcast(m:BackgroundService) : Thread() {
    val parent = m
    var callback: ((Int) -> Unit)? = null
    var context = m as Context
    var isRun = true

    var BLEUUID: String = "F0"
    var major: Int = 0
    var minor: Int = 0

    var beacon: Beacon? = null
    var beaconParser: BeaconParser? = null
    var beaconTransmitter: BeaconTransmitter? = null
    var isBLEEnabled = false
    var blestat = 0

    var loopcount = 0
    var beaconTimer = 0L
    val beaconTimerInterval = 20000L

    var attendCountTimer = 0L
    val attendCountInterval = 60000L * 30


    var countInterval = 500L

    val controller = MyController(parent, Looper.getMainLooper())
    var crypt = controller.Crypt

    fun count2Millsec(t: Int) = t * countInterval
    fun millsec2Count(ms: Int) = ms / countInterval

    init {
        Log.i("Thread", "Start${System.currentTimeMillis()}")
        isRun = true
    }

    fun setcallback(call: (Int) -> Unit) {
        callback = call
    }


    fun stopForever() {
        synchronized(this) {
            isRun = false
            Log.i(
                "Thread",
                " End : ${System.currentTimeMillis()}"
            )
        }
    }

    override fun run() {
        BLEInit()

        if (isBLEEnabled) BLEDuplicate10()

        beaconTimer = System.currentTimeMillis()
        while (isRun) {
            if (isBeaconTime(beaconTimer, beaconTimerInterval)) {
                beaconTimer = System.currentTimeMillis()
                processBLETask()
            }

            if (isBeaconTime(attendCountTimer, attendCountInterval)) {
                attendCountTimer = System.currentTimeMillis()
                processAttendanceCount()
            }


            // 블루투스를 3연속 껐다 켜면 비콘 신호가 끊어진다.
            // 10초마다 검사하며 블루투스 여부를 알아낸다.
            parent.CheckBLEAlive()

            try {
                sleep(countInterval)
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }
        }
        BLEStop()
        if (isBLEEnabled) BLEDuplicate10_TurnOff()

        Log.i("Thread", String.format(" Out : %d", System.currentTimeMillis()))
    }

    fun isBeaconTime(timer: Long, interval: Long) = System.currentTimeMillis() - timer >= interval

    fun processBLETask() {
        //Log.d("Thread", String.format(" run : %d", System.currentTimeMillis()))
        if (isBLEEnabled) {
            if (idxUser == 0) {
                stopForever()
                return
            }

            val strUUID = makedata(idxUser)
            updateData(strUUID, AESNo - 1, 0xFFEF)

            BLEConfigure()
            BLEChange()

            BLEDuplicate10_TurnOff()
            if (BLEDuplicateEnabled) {
                BLEDuplicate10_TurnON()
            }
        } else BLEStop()
    }

    fun BLEInit() {
        val flag = BeaconTransmitter.checkTransmissionSupported(context)
        if (flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER || flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS) {
            Log.e("BEACON", "LE Adapter in not available!!")
            callOnChangeCallback(BEACON_FAILED)
        } else if (!context.packageManager.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            isBLEEnabled = false
            Log.e("BEACON", "LE Adapter CAN'T be used!!")
            callOnChangeCallback(BEACON_FAILED)
        } else if (!BluetoothAdapter.getDefaultAdapter().isEnabled) {
            isBLEEnabled = false
            Log.e("BEACON", "LE is closed.")
            callOnChangeCallback(BEACON_FAILED)
        } else isBLEEnabled = true
        Log.e("BEACON", "isBLEEnabled : $isBLEEnabled")
        callOnChangeCallback(BEACON_WAITING)

        isRun = true
        beaconParser = BeaconParser()
        beaconParser!!.setBeaconLayout("m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24") //if null throws error
        beaconTransmitter = BeaconTransmitter(context, beaconParser)

    }

    fun BLEChange() {
        beaconTransmitter?.stopAdvertising()
        callOnChangeCallback(BEACON_STOPPED)

        beaconTransmitter ?: Log.e("Beacon", "beaconTransmitter is NULL")

        callOnChangeCallback(BEACON_WAITING)
        beaconTransmitter!!.startAdvertising(beacon, object : AdvertiseCallback() {
            override fun onStartFailure(errorCode: Int) {
                Log.e("TAG", "Advertisement start failed with code: $errorCode")
                callOnChangeCallback(BEACON_FAILED)
                beaconTransmitter?.stopAdvertising()//if null just return null
            }

            override fun onStartSuccess(settingsInEffect: AdvertiseSettings) {
                Log.i("TAG", "Advertisement start succeeded.")
                callOnChangeCallback(BEACON_SUCCESS)
            }
        })
    }

    var duplicateTransmitter = emptyArray<BeaconTransmitter>()
    fun BLEDuplicate10() {
        duplicateUserArray =
            when (idxUser) {
                11 -> arrayOf(101, 102, 103, 104, 105, 106, 107, 129, 130)
                16 -> arrayOf(108, 109, 110, 111, 112, 113, 114)
                17 -> arrayOf(115, 116, 117, 118, 119, 120, 121)
                13, 12 -> arrayOf(122, 123, 124, 125, 126, 127, 128)
                0 -> emptyArray()
                else -> emptyArray()
            }
        duplicateTransmitter = Array(10) { BeaconTransmitter(context, beaconParser) }
    }

    fun BLEDuplicate10_TurnOff() {
        for (i in duplicateUserArray.indices) {
            val sendidx = duplicateUserArray[i]
            if (sendidx == 0) break

            duplicateTransmitter[i].stopAdvertising()//if null just return null
        }
    }

    fun BLEDuplicate10_TurnON() {
        for (i in duplicateUserArray.indices) {
            val sendidx = duplicateUserArray[i]
            if (sendidx == 0) break

            val strUUID = makedata(sendidx)
            updateData(strUUID, AESNo - 1, 0xFFEF)
            BLEConfigure()

            beacon ?: Log.e("Beacon", "beacon is NULL")
            beaconTransmitter ?: Log.e("Beacon", "beaconTransmitter is NULL")
            beacon ?: continue
            beaconTransmitter ?: continue

            duplicateTransmitter[i].startAdvertising(beacon, object : AdvertiseCallback() {
                override fun onStartFailure(errorCode: Int) {
                    Log.e("TAG", "Advertisement[$i] start failed with code: $errorCode")
                    duplicateTransmitter[i].stopAdvertising()//if null just return null
                }

                override fun onStartSuccess(settingsInEffect: AdvertiseSettings) {
                    Log.i("TAG", "Advertisement[$i] start succeeded.")
                }
            })
        }
    }

    fun BLEStop() {
        beaconTransmitter?.stopAdvertising()

        callOnChangeCallback(BEACON_STOPPED)
    }

    fun updateData(BLEUUid: String, maj: Int, min: Int) {
        synchronized(this) {
            BLEUUID = BLEUUid
            major = maj
            minor = min
        }
    }

    fun BLEConfigure() {
        beacon = Beacon.Builder()
            .setId1(BLEUUID)
            .setId2(major.toString())
            .setId3(minor.toString())
            .setManufacturer(0x004C)
            .setTxPower(-66)
            .build()
    }

    fun GetnerateAESNo(): Int {
        val cal = Calendar.getInstance()
        val month = cal[Calendar.MONTH] + 1
        val day = cal[Calendar.DAY_OF_MONTH]
        val random = Random()
        random.setSeed(cal.timeInMillis)
        val randomVal = random.nextInt(10) //0~9
        return (month + day) / 2 + randomVal //1~30
    }

    var AESNo = 0
    fun makedata(idxUser: Int): String {
        AESNo = GetnerateAESNo()
        val dateformat = SimpleDateFormat("yyyyMMdd", Locale.getDefault())
        val now = Date()

        if (BLEDataCount > 60000) BLEDataCount = 0 else BLEDataCount++
        changeSaveCode("10905", BLEDataCount.toString())

        val string_x = dateformat.format(now) + String.format(
            "%04X",
            BLEDataCount
        ) + java.lang.String.format("%08X", idxUser)
        Log.i("Send", string_x)

        val iterator = (string_x.length + 1) / 2
        val x_encoded = ByteArray(iterator)
        for (i in 0 until iterator) {
            var sub = string_x.substring(i * 2)
            sub = if (sub.length == 1) sub[0].toString() + "?" else sub.substring(0, 2)

            x_encoded[i] = Str2Hex(sub).toByte()
        }
        Log.i("Send", BytesToString(x_encoded))
        AESLog("Beacon(1~30)", AESNo)

        //> (x) Base64(AES암호화.aes_no(yyyyMMdd+난수 값(0001 ~ 9999))
        //         난수 값은 매번 변경, 단 일일이내 난수값이 중복 될 수는 없다는 전제하...
        //> (y) Base64(AES암호화.aes_no(hp))
        //> (z) manufacture : 0xEFFF (AJUAttender 고유 코드) + aes_no

        // yyyyMMddNNNNiiiiiiii (i:idxUser,y:year,M:month,d:date,N:count)
        //12Bytes
        val binUUID = crypt.AESEncode(x_encoded, AESNo)

        val out = StringBuilder()
        for (i in binUUID!!.indices) {
            out.append(String.format("%02x", binUUID[i]))
        }
        Log.i("Send", "out : $out")
        return out.toString()
    }

    fun Str2Hex(str: String): Int {
        var result = 0
        for (ch in str.iterator())
            result = result * 16 + when (ch) {
                in '0'..'9' -> ch - '0'
                in 'A'..'F' -> ch - 'A' + 10
                '?' -> 15
                else -> 0
            }
        return result
    }

    fun AESLog(TAG: String?, AESNum: Int) {
        Log.i(TAG, "AESNo : $AESNum")
        Log.i(TAG, "AESIV : " + BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum - 1][0], 0)))
        Log.i(TAG, "AESKey : " + BytesToString(Base64.decode(crypt.AESKeyIVs[AESNum - 1][1], 0)))
    }

    val BEACON_WAITING = 0
    val BEACON_STOPPED = 1
    val BEACON_SUCCESS = 2
    val BEACON_FAILED = 3
    fun callOnChangeCallback(bleStat: Int) {
        blestat = bleStat
        callback?.invoke(bleStat)
    }

    fun processAttendanceCount() {
        controller.iodtlist(
            { isSucceed, reply, replyArray ->
                if (!isSucceed) return@iodtlist

                val rtncode = try {
                    replyArray.getJSONObject(0).getString("rtncode").toInt()
                } catch (e: Throwable) {
                    1
                }
                if (rtncode == 1 || rtncode == 15) {//rtn = 15 : 출입 데이터가 없습니다

                    //JSON 데이터 해석
                    var attendanceCount = 0


                    for (i in 0 until replyArray.length()) {
                        val obj: JSONObject = replyArray.getJSONObject(i)

                        //, "time" : "08:55:30", "cc" : "1000", "ccname" : "회사명", "gatename" : "출입문"
                        // "rtncode":"01","time":"22:37:23","idxc":1,"ccname":"(주)아주소프트","gatename":"출입문2"
                        try {
                            if (obj.getString("rtncode") != "comlist") {
                                attendanceCount++ // counts only the Attendance data (keep out clients)
                            }
                        } catch (e: JSONException) {

                        }
                    }
                    //re-get the value of clientcode.
                    if (clientcode == null)clientcode = try{
                        MyApplication.getStringCode(10900)
                    } catch(_:Throwable){null}

                    val clientStr = MyApplication.getClientName(clientcode)
                    val notiText =
                        if (clientStr != null) {
                            String.format("\"%s\"이/가 선택되었습니다", clientStr)
                        } else {
                            String.format("")
                        }

                    makeNotification(String.format("총 %d건의 입출입 발생", attendanceCount), notiText)

                } else {
                    var errorstr = try {
                        replyArray.getJSONObject(0).getString("msg")
                    } catch (e: Throwable) {

                    }
                }
            },
            idxUser.toString(),
            SimpleDateFormat("yyyyMMdd", Locale.US).format(Calendar.getInstance().time),
            ""
        )
    }
}

/*
 from the main remained source about BLE Advertise.

    fun initalizeBLE() {
        if (!this.packageManager.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            isBLEEnabled = false
            Log.e("BEACON", "LE Adapter CAN'T be used!!")
        }

        val flag = BeaconTransmitter.checkTransmissionSupported(this)
        if (flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER || flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS) {
            Log.e("BEACON", "LE Adapter in not available!!")
        } else isBLEEnabled = true


        Log.e("BEACON", "isBLEEnabled : $isBLEEnabled")

        beaconParser = BeaconParser()
        beaconParser!!.setBeaconLayout("m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24") //if null throws error
        beaconTransmitter = BeaconTransmitter(this,beaconParser)
    }
    fun BLEturnON()
    {
        beaconTransmitter ?: Log.e("Beacon","beaconTransmitter is NULL")
        beaconTransmitter!!.startAdvertising(beacon, object : AdvertiseCallback() {
            override fun onStartFailure(errorCode: Int) {
                Log.e(
                    "TAG",
                    "Advertisement start failed with code: $errorCode"
                )
                blestat.setText("BLE FAILED")

                beaconTransmitter?.stopAdvertising()//if null just return null
            }

            override fun onStartSuccess(settingsInEffect: AdvertiseSettings) {
                Log.i("TAG", "Advertisement start succeeded.")
                blestat.setText("BLE ON")

            }
        })
    }
    fun BLEturnOFF(){
        blestat.setText("BLE OFF")
        beaconTransmitter ?: Log.e("Beacon","beaconTransmitter is NULL")
        beaconTransmitter?.stopAdvertising()//if null just return null
    }
    fun BLEConfigureData(uuid: String,AesNo:Int,manufacture: Int)
    {
        beacon = Beacon.Builder()
            .setId1(uuid)
            .setId2(AesNo.toString())
            .setId3(manufacture.toString())
            .setManufacturer(0x004C)
            .setTxPower(-66)
            .build()
        Log.d("BEACON",uuid)
    }
 */