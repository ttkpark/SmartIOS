package com.example.myfirstkotlin

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.content.ContentValues
import android.content.DialogInterface
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.ColorStateList
import android.graphics.Color
import android.graphics.Color.parseColor
import android.os.Build
import android.os.Bundle
import android.os.Looper
import android.util.Log
import android.view.*
import android.widget.ImageButton
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.view.iterator
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.lifecycle.Lifecycle
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.adapter.FragmentStateAdapter
import androidx.viewpager2.widget.ViewPager2
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.InitHashes
import com.example.myfirstkotlin.MyApplication.Companion.ResetFile
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.clientInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.colorStateList
import com.example.myfirstkotlin.MyApplication.Companion.getClientName
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.getStringCodeHash
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.isAdmin
import com.example.myfirstkotlin.MyApplication.Companion.myNotification
import com.example.myfirstkotlin.MyApplication.Companion.privClientcode
import com.example.myfirstkotlin.MyApplication.Companion.saveCodeData
import com.google.android.gms.tasks.OnCompleteListener
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.firebase.messaging.FirebaseMessaging
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.system.exitProcess

class MainActivity : AppCompatActivity() {
    lateinit var bottomNavigationView :BottomNavigationView
    lateinit var viewpager : ViewPager2
//    lateinit var settingsButton : ImageButton

    lateinit var fragment1Main : Fragment1_main
    lateinit var fragment3List : Fragment3_list
    lateinit var fragment5Setting : Fragment5_setting

    lateinit var toolbar: Toolbar

    lateinit var controller: MyController

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        controller = MyController(this, Looper.getMainLooper())

        setContentView(R.layout.activity_main)

        clientcode = try{getStringCode(10900)} catch(_:Throwable){null}

        bottomNavigationView = findViewById(R.id.bottomNavigationView)
        viewpager = findViewById(R.id.viewpager)
//        settingsButton = findViewById(R.id.settingsButton)
        bottomNavigationView.inflateMenu(if (!isAdmin)R.menu.menu_main_bottomnav else R.menu.menu_main_bottomnav_admin)

        setToolBar()
        checkPermission()
    }
    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        findViewById<TextView>(R.id.toolbarText).text = getStringCode(9999,getString(R.string.app_name))
        UpdateclientNameTextview()
        setColor()
    }
    fun UpdateclientNameTextview()
    {
        val clientName = getClientName(clientcode?:"") ?: ""
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

        //bottomNavigationView.setBackgroundColor(parseColor(MyApplication.AppColor))
        colorStateList = ColorStateList(
            arrayOf(arrayOf(android.R.attr.state_checked).toIntArray(),
                    arrayOf(-android.R.attr.state_checked).toIntArray()),
            arrayOf(parseColor(AppColor),
                Color.GRAY).toIntArray())
        bottomNavigationView.itemTextColor = colorStateList
        bottomNavigationView.itemRippleColor = colorStateList
        bottomNavigationView.itemIconTintList = colorStateList
    }

//        supportActionBar!!.setDisplayHomeAsUpEnabled(false)
//        supportActionBar!!.(R.drawable.ic_baseline_settings_24)
//        supportActionBar!!.setDisplayShowTitleEnabled(false)

    private inner class PagerAdapter(fm:FragmentManager,lc:Lifecycle,m:MainActivity): FragmentStateAdapter(fm, lc) {
        val parent = m
        override fun getItemCount() = if(isAdmin) 3 else 2
        override fun createFragment(position: Int): Fragment {
            return when (position) {
                0 -> {fragment1Main = Fragment1_main(parent);fragment1Main}
                1 -> {fragment3List = Fragment3_list(parent);fragment3List}
                2 -> {fragment5Setting = Fragment5_setting(parent);fragment5Setting}
                else -> error("no such position: $position") // TODO : String Conversion
            }
        }
    }
    private fun navigationSelected(item: MenuItem): Boolean {
        val checked = item.setChecked(true)
        when (item.itemId) {
            R.id.mainItem    -> viewpager.currentItem = 0
            R.id.listItem    -> viewpager.currentItem = 1
            R.id.settingItem -> viewpager.currentItem = 2
            else -> return false
        }
        return true
    }
    private inner class PageChangeCallback: ViewPager2.OnPageChangeCallback() {
        override fun onPageSelected(position: Int) {
            super.onPageSelected(position)
            bottomNavigationView.selectedItemId = when (position) {
                0 -> {try{fragment5Setting.isUnFocused(); fragment3List.isUnFocused()}catch(_:Throwable){};R.id.mainItem}
                1 -> {try{fragment5Setting.isUnFocused(); fragment3List.isFocused()}catch(_:Throwable){};R.id.listItem}
                2 -> {try{fragment5Setting.isFocused(); fragment3List.isUnFocused()}catch(_:Throwable){};R.id.settingItem}
                else -> error("no such position: $position") // TODO : String Conversion
            }
        }
    }

    fun startTask() {
        StartBackground()

        initalizeControls()

        if(!BackgroundService.isBluetoothON)
        {
            registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { activityResult ->
                if(activityResult.resultCode != RESULT_OK){ // TODO : String Conversion
                    MessageAlert("블루투스 경고",getString(R.string.bluetooth_down_message))
                    BackgroundService.isBluetoothON = true;//메시지 재생성.
                }
            }.launch(Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE))
        }

    }


    fun StartBackground() {
        // 포그라운드 서비스 호출
        val serviceintent = Intent(this, BackgroundService::class.java)
        startService(serviceintent)
    }

    fun initalizeControls() {
//        settingsButton.setOnClickListener {
//
//        }

        viewpager.adapter = PagerAdapter(supportFragmentManager, lifecycle,this)
        viewpager.registerOnPageChangeCallback(PageChangeCallback())
        bottomNavigationView.setOnNavigationItemSelectedListener { item -> navigationSelected(item) }
    }

    fun checkPermission() {
        val permissionInternet = ContextCompat.checkSelfPermission(
            this,
            Manifest.permission.INTERNET
        ) == PackageManager.PERMISSION_GRANTED
        val permissionEXTSTORAGE = ContextCompat.checkSelfPermission(
            this,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
        ) == PackageManager.PERMISSION_GRANTED

        if (permissionInternet && permissionEXTSTORAGE)
            startTask()
        else {
            if (ActivityCompat.shouldShowRequestPermissionRationale(
                    this,
                    Manifest.permission.INTERNET
                ) or ActivityCompat.shouldShowRequestPermissionRationale(
                    this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                )
            )
                showPermissionDialog()
            else
                requestPermission()
        }
    }

    fun showPermissionDialog() {
        val builder = AlertDialog.Builder(this)

        val isContinued = AtomicBoolean(false)
        builder.setTitle("권한 요청 필요") // TODO : String Conversion
            .setMessage("로그인을 위한 전화번호와 인터넷 권한, 외부 저장소 권한이 필요합니다.\n앱을 이용하려면 권한에 모두 동의하시기 바랍니다.")
            .setPositiveButton("continue") { dialog: DialogInterface, _: Int ->
                isContinued.set(true) // TODO : String Conversion
                dialog.dismiss()
                requestPermission()
            }
            .setOnDismissListener { if (!isContinued.get()) appTerminate() }
            .create().show()
    }

    fun requestPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(
                arrayOf(
                    Manifest.permission.INTERNET,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                ),
                1000
            )

        }
//        registerForActivityResult(ActivityResultContracts.RequestPermission()) {Result->
//            if(!Result)
//                TerminateAlert("Permission Denied", "READ_PHONE_STATE");
//        }.launch(Manifest.permission.READ_PHONE_STATE)
//
//        registerForActivityResult(ActivityResultContracts.RequestPermission()) {Result->
//            if(!Result)
//                TerminateAlert("Permission Denied", "WRITE_EXTERNAL_STORAGE");
//        }.launch(Manifest.permission.WRITE_EXTERNAL_STORAGE)
    }

    override fun onRequestPermissionsResult( requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            1000 -> {
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {

                    startTask()

                } else {
                    var err = " ("
                    if (grantResults.isEmpty()) err = "ERROR" else { // TODO : String Conversion
                        if (grantResults[0] != PackageManager.PERMISSION_GRANTED) err += "Internet "
                        if (grantResults[1] != PackageManager.PERMISSION_GRANTED) err += "EXTSTORAGE "
                        err += ")"
                    }
                    //appTerminate()
                    TerminateAlert("Permission Denied", "Permission denied.($err)") // TODO : String Conversion
                }
                return
            }
        }
    }

    fun TerminateAlert(Title: String?, Messages: String?) {
        val builder = AlertDialog.Builder(this)
        builder.setTitle(Title).setMessage(Messages) // TODO : String Conversion
            .setPositiveButton("Exit") { _: DialogInterface?, _: Int -> }
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

    fun onClientCodeTextClick_Dialog(array:Array<String>,title: String?,infoText: String?,callback: ((SelectClientDialog,Int) -> Unit)?, dismiss:((SelectClientDialog)->Unit)? ) {
        clientInfoArray ?: return
        lateinit var dlg : SelectClientDialog


        val OnSelected = AtomicBoolean(false)

        val dataAdapter = DataAdapter(array,object : DataAdapter.RecyclerViewItemClickListener {
            override fun clickOnItem(position : Int){
                callback?.invoke(dlg,position)
                OnSelected.set(true)
            }

        })
        dlg = SelectClientDialog(this, dataAdapter,clientInfoArray)
        dlg.setListener {
            dismiss?.invoke(dlg)
        }

        dlg.show(title,infoText, parseColor(AppColor))
    }

    fun setclientCode(position: Int) {
        clientcode = clientInfoArray!!.get(position)?.clientCode ?: "0"
        privClientcode = clientcode
        Log.e("SetClientCode", "clientcode -> $clientcode")
        getStringCodeHash.put("10900", clientcode) //clientcode
        ResetFile()
        saveCodeData()
        setToolBar()

        myNotification.sendEmptyMessage(2)// update Notification Title Cycle
    }

    fun update_clientInfoArray(callback: (() -> Unit)?){
        controller.comlist({isSucceed, reply, replyArray ->
            if(!isSucceed)return@comlist

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){1}
            if(rtncode == 1) {

                val count = replyArray.length()-1
                val array : Array<MyApplication.ClientInfo?> = arrayOfNulls(count)

                for(i in 0 until array.size)
                {
                    val obj = replyArray.getJSONObject(i+1)
                    array[i] = MyApplication.ClientInfo()

                    try{
                        //"cc" : "1000", "cn" : "client명1"
                        array[i]?.clientCode = obj.getString("code")
                        array[i]?.clientName = obj.getString("cn")
                        array[i]?.clientLocation = obj.getString("addr")
                    }catch(e: JSONException){}
                }
                clientInfoArray = array
                callback?.invoke()
            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {
                    getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                MessageAlert("comlist",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }

        }, idxUser.toString())
    }
    fun callLanguageCode(callback: ((succeed:Boolean) -> Unit)?)
    {
        controller.langv({ isSucceed, reply, replyArray -> if(isSucceed) {
            val rtncode = try { replyArray.getJSONObject(0).getString("rtncode").toInt() } catch (e: Throwable) { 1 }
            if (rtncode == 1) {
                processLanguageCode(replyArray)
                saveCodeData()
                InitHashes()

            } else {
                var errorstr = try {
                    replyArray.getJSONObject(0).getString("msg")
                } catch (e: Throwable) {
                    getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                }
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                Throwable("Unknown Error.($rtncode,$errorstr)").printStackTrace() // TODO : String Conversion
                callback?.invoke(true)
                //Languagecode를 호출할 때는 에러가 나도 계속한다.
            }
        }}, idxUser.toString())
        idxUser = try { getStringCode(9988).toInt() } catch(e:Throwable){0}
    }
    fun processLanguageCode(replyArray : JSONArray)
    {
        for (i in 0 until replyArray.length()) {
            val obj: JSONObject = replyArray.getJSONObject(i)
            getStringCodeHash.put(
                obj.getString("code").toInt().toString(),
                obj.getString("val")
            )
            Log.i("Language",obj.toString())
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.menu_settings, menu)       // main_menu 메뉴를 toolbar 메뉴 버튼으로 설정
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when(item.itemId){
            R.id.menu_account -> startActivity(Intent(this,SettingsActivity::class.java))
            R.id.menu_logout -> { ResetFile();InitHashes();startActivity(Intent(this,LoginActivity::class.java)) }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun finish() {
        super.finish()
        //exitProcess(0)
    }
}

class DataAdapter(myDataset: Array<String>, listener: RecyclerViewItemClickListener ) : RecyclerView.Adapter<DataAdapter.FruitViewHolder>()  {
    var mDataset = myDataset
    val recyclerViewItemClickListener = listener

    override fun onCreateViewHolder(parent: ViewGroup, i: Int): FruitViewHolder {
        val v = LayoutInflater.from(parent.context)
            .inflate(R.layout.clientdialog_content, parent, false)
        return FruitViewHolder(v,this)
    }

    override fun onBindViewHolder(fruitViewHolder: FruitViewHolder, i: Int) {
        fruitViewHolder.mTextView.setText(mDataset[i])
    }

    override fun getItemCount(): Int {
        return mDataset.size
    }


    class FruitViewHolder(v: View,p:DataAdapter) : RecyclerView.ViewHolder(v),
        View.OnClickListener {
        val parent = p
        var mTextView = v.findViewById<TextView>(R.id.clientName)
        override fun onClick(v: View) {
            parent.recyclerViewItemClickListener.clickOnItem(this.adapterPosition)
        }

        init {
            v.setOnClickListener(this)
        }
    }

    interface RecyclerViewItemClickListener {
        fun clickOnItem(position: Int)
    }
}
