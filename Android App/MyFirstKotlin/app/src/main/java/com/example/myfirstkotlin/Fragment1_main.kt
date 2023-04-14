package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.app.Dialog
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.graphics.Color
import android.graphics.Color.parseColor
import android.os.*
import android.view.*
import android.widget.Button
import android.widget.ImageView
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.bumptech.glide.Glide
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.BLEDuplicateEnabled
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.clientInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.duplicateUserArray
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import org.json.JSONException
import org.json.JSONObject
import com.example.myfirstkotlin.MyApplication.Companion.isAdmin
import org.w3c.dom.Text


class Fragment1_main(var parent: MainActivity) : Fragment(){
    var BackgroundBinder : MyBinder? = null
    var BLE : BLEBroadcast? = null

    var BLEStat : Int = 0

    lateinit var btnQR : MmenuIconContent
    lateinit var ClientCodeCtrl : MmenuIconContent
    lateinit var btnDuplicate30 : MmenuIconContent
    lateinit var imgBLE : ImageView

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragment1_main,container,false)

        parent.bindService(Intent(parent,BackgroundService::class.java),connection, Context.BIND_AUTO_CREATE)
        btnQR = view.findViewById(R.id.QRCode3)
        btnDuplicate30 = view.findViewById(R.id.duplicateBtn)
        imgBLE = view.findViewById(R.id.beaconState)
        ClientCodeCtrl = view.findViewById(R.id.clientCode)

        setColor()

        if(duplicateUserArray.isEmpty()){
            btnDuplicate30.visibility = View.GONE
        }else{
            btnDuplicate30.visibility = View.VISIBLE

            //enable button work.
            btnDuplicate30.setOnClickListener {
                if(!BLEDuplicateEnabled)//if OFF
                {
                    btnDuplicate30.setTitle("ON")
                    BLEDuplicateEnabled = true //make it ON
                }
                else//if ON
                {
                    btnDuplicate30.setTitle("OFF")
                    BLEDuplicateEnabled = false//make it OFF
                }
            }
        }
//        if(clientInfoArray == null)
//        {
//            parent.update_clientInfoArray {  }
//        }

        btnQR.setOnClickListener { _ ->
            val dialog = Dialog(parent)
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.setContentView(R.layout.qrcode_dialog)
            val params = dialog.window!!.attributes
            params.width = WindowManager.LayoutParams.MATCH_PARENT
            params.height = WindowManager.LayoutParams.WRAP_CONTENT
            dialog.window!!.attributes = params

            val OKBtn = dialog.findViewById<Button>(R.id.OKBtn)
            OKBtn.setBackgroundColor(parseColor(AppColor))
            OKBtn.setTextColor(parseColor(TextColor))
            OKBtn.setOnClickListener { v: View? -> dialog.dismiss() }

            val QRCodeImg = dialog.findViewById<ImageView>(R.id.QRCode)
            Glide.with(this).load(getString(R.string.QRCODE)).placeholder(R.drawable.ic_loading)
                .error(R.drawable.ic_error).into(QRCodeImg)
            dialog.show()
        }
        imgBLE.setOnClickListener { _ ->
            try { Toast.makeText(view.context,BLEstate_ment[BLEStat],Toast.LENGTH_SHORT).show()} catch (e:Throwable){e.printStackTrace()}
        }
        changeBLEStat(BLEStat)//intialize the image

        /*
        ClientCodeCtrl.setOnClickListener { parent.onClientCodeTextClick_Dialog({
              ClientCodeCtrl.setTitle(clientcode)
        },null)}
        */

        return view
    }
    fun setColor()
    {
        btnDuplicate30.setTextColor(parseColor(TextColor))
        btnDuplicate30.setBackgroundColor(parseColor(AppColor))
        btnQR.setTextColor(parseColor(TextColor))
        btnQR.setBackgroundColor(parseColor(AppColor))
    }


//    val BEACON_WAITING = 0
//    val BEACON_STOPPED = 1
//    val BEACON_SUCCESS = 2
//    val BEACON_FAILED = 3
    val BLEstate_image = arrayOf(R.drawable.ic_blestate_turning,R.drawable.ic_blestate_off,R.drawable.ic_blestate_on,R.drawable.ic_error)
    val BLEstate_ment  = arrayOf("Turning ON","BLE OFF","BLE ON","Error") // TODO : String Conversion

    @SuppressLint("UseCompatLoadingForDrawables")
    fun changeBLEStat(stat:Int)
    {
        BLEStat = stat
        try {
            imgBLE.setImageDrawable(parent.getDrawable(BLEstate_image[BLEStat]))
        }catch (e:Throwable){e.printStackTrace()}
    }


    /** Defines callbacks for service binding, passed to bindService()  */
    private val connection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName,service: IBinder) {
            // We've bound to LocalService, cast the IBinder and get LocalService instance
            BackgroundBinder = service as MyBinder
            BLE = BackgroundBinder?.GetBLE()

            BLE?.setcallback { stat ->
                val APIHandler = Handler(Looper.getMainLooper())
                APIHandler.sendMessage(
                    Message.obtain(APIHandler) {
                        changeBLEStat(stat)
                    })}
            changeBLEStat(BLE?.blestat ?: 0)
        }

        override fun onServiceDisconnected(arg0: ComponentName) {
            BackgroundBinder = null
            BLE = null
        }
    }
}