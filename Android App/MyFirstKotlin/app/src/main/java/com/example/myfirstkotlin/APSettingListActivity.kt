package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.content.DialogInterface
import android.content.Intent
import android.graphics.Color
import android.graphics.Color.parseColor
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.ImageButton
import android.widget.ListView
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.widget.Toolbar
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.apInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import org.json.JSONException

class APSettingListActivity : AppCompatActivity() {
    lateinit var controller:MyController
    lateinit var List_AP: ListView
    lateinit var Text_AP: TextView
    lateinit var backBtn: ImageButton
    lateinit var viewTitle : TextView
    lateinit var toolbar: Toolbar
    lateinit var arrayAdapter : APListAdapter
    lateinit var swipe : SwipeRefreshLayout

    var focus = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_apsetting_list)

        Text_AP = findViewById(R.id.text_ap)
        List_AP = findViewById(R.id.List_ap)

        backBtn = findViewById(R.id.backButton)
        backBtn.setOnClickListener { finish() }

        swipe = findViewById(R.id.swipe_listap)
        swipe.setOnRefreshListener { requestAPI(); swipe.isRefreshing = false }

        controller = MyController(this, Looper.getMainLooper())

        if(clientcode == null || Integer.parseInt(clientcode!!) == 0)
        {
            TerminateAlert("admin","No clientcode. app internal error.")
            // TODO : String Conversion
            //parent.onClientCodeTextClick_Dialog( {dlg-> dlg.dismiss();startTask() },null)
        }else
        {
            //List 초기 세팅 : adapter를 만들고 나중에 arrayAdapter.notifyDataSetChanged()를 호출하여 업데이트한다.
            arrayAdapter = APListAdapter(this, emptyArray())
            List_AP.adapter = arrayAdapter

            List_AP.setOnItemClickListener { parent, view, position, id ->
                if(arrayAdapter.list[position].dist == null) return@setOnItemClickListener

                val row = arrayAdapter.list[position]
                callSettingsActivity(row.idx)
            }

            //requestAPI()
        }
        setToolBar()

        if(intent != null)
            focus = intent.getIntExtra("focus",0)
    }
    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        viewTitle = findViewById(R.id.toolbarText)
        viewTitle.text = "APList"
        UpdateclientNameTextview()
        setColor()
    }
    fun UpdateclientNameTextview()
    {
        val clientName = MyApplication.getClientName(clientcode ?: "") ?: ""
        val clientnameTextView = findViewById<TextView>(R.id.clientname)
        if(!clientName.isNullOrEmpty()) {
            clientnameTextView.visibility = View.VISIBLE
            clientnameTextView.text = clientName
        }else
            clientnameTextView.visibility = View.GONE
    }
    fun setColor()
    {
        toolbar.setBackgroundColor(parseColor(AppColor))
        toolbar.setTitleTextColor(parseColor(TextColor))
    }


    override fun onResume() {
        super.onResume()

        if(clientcode == null || Integer.parseInt(clientcode!!) == 0);
        else requestAPI()
    }
    fun callSettingsActivity(idxEquip : Int?)
    {
        if(idxEquip==null){
            MessageAlert("aplist","Invalid idxEquip.") // TODO : String Conversion
            return
        }
        val intent = Intent(this,APSettingActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
        intent.putExtra("idxEquip",idxEquip.toString())
        intent.putExtra("focus",focus)
        startActivity(intent)
        finish()
    }
    fun requestAPI()
    {
        //clientcode = "1"
        controller.aplist({isSucceed, reply, replyArray ->
            if(!isSucceed)return@aplist

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){1}
            if(rtncode == 1) {

                val count = replyArray.length()
                val array : MutableList<MyApplication.APInfo> = emptyArray<MyApplication.APInfo>().toMutableList()

                for(i in 0 until count)
                {
                    val obj = replyArray.getJSONObject(i)
                    //{"rtncode" : "01"
                    // , "apname" : "AP명" ,"idx" : "1", "dist" : "","useyn" : "","flg" : ""
                    // ,"ip" : "","sty" : ","gps" : "","useio" : "","openlock" : "","tz" : "50"}
                    val arrayItem = MyApplication.APInfo()

                    try{
                        //"cc" : "1000", "cn" : "client명1"
                        arrayItem.apname = obj.getString("apname")
                        arrayItem.idx = obj.getInt("idx")
                        arrayItem.dist = obj.getString("dist")
                        arrayItem.useyn = obj.getString("useyn")
                        arrayItem.flg = obj.getString("flg")
                        arrayItem.ip = obj.getString("ip")
                        arrayItem.sty = obj.getString("sty")//Sensor style
                        arrayItem.useio = obj.getString("useio")
                        arrayItem.openlock = obj.getInt("openlock")
                        arrayItem.gps = obj.getString("gps")
                        arrayItem.tz = obj.getInt("tz")//timeZone
                    }catch(e: JSONException){}
                    array.add(arrayItem)
                }
                apInfoArray = array.toTypedArray()
            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {null}
                if(errorstr.isNullOrEmpty())errorstr = getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                MessageAlert("aplist",errorstr) // TODO : String Conversion
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                val v = MyApplication.APInfo()
                v.apname = errorstr
                apInfoArray = arrayOf(v)
            }
            listSetting()

        }, clientcode)
    }
    @SuppressLint("DefaultLocale")
    fun listSetting()
    {
        arrayAdapter.list = apInfoArray
        arrayAdapter.notifyDataSetChanged()

        Text_AP.text = java.lang.String.format("Total : %d", apInfoArray.size) // TODO : String Conversion
    }


    fun TerminateAlert(Title: String?, Messages: String?) {
        val builder = AlertDialog.Builder(this)
        builder.setTitle(Title).setMessage(Messages)
            .setPositiveButton("Exit") { _: DialogInterface?, _: Int -> } // TODO : String Conversion
            .setOnDismissListener { appTerminate() }
            .create().show()
    }

    fun MessageAlert(Title: String?, Messages: String?) {
        AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
            .setPositiveButton("OK") { _: DialogInterface?, _: Int -> }.create().show()
    } // TODO : String Conversion
    fun appTerminate() {
        finishAffinity()
        System.runFinalization()
        System.exit(0)
    }
}