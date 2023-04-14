package com.example.myfirstkotlin

import android.content.DialogInterface
import android.graphics.Color
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.view.MenuItem
import android.view.View
import android.widget.ImageButton
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.widget.Toolbar
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.lifecycle.Lifecycle
import androidx.viewpager2.adapter.FragmentStateAdapter
import androidx.viewpager2.widget.ViewPager2
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.google.android.material.bottomnavigation.BottomNavigationView
import org.json.JSONException
import org.json.JSONObject
import java.lang.Exception

class AdminStatusActivity : AppCompatActivity() {
    lateinit var fragment1ap : FragmentAdminstat_1ap
    lateinit var fragment2user : FragmentAdminstat_2user

    lateinit var bottomNavigationView : BottomNavigationView
    lateinit var viewpager : ViewPager2
    lateinit var backBtn : ImageButton
    lateinit var toolbar: Toolbar

    lateinit var controller: MyController

    var arrayAP: Array<MyApplication.ItemTotalStatus> = emptyArray()
    var arrayUsr: Array<MyApplication.ItemTotalStatus> = emptyArray()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_admin_status)

        controller = MyController(this, Looper.getMainLooper())

        viewpager = findViewById(R.id.viewpager)
        bottomNavigationView = findViewById(R.id.bottomNavigationView)
        setToolBar()

        backBtn = findViewById(R.id.backButton)
        backBtn.setOnClickListener { finish() }

        requestAPI()
    }
    fun startTask()
    {
        viewpager.adapter = PagerAdapter(supportFragmentManager, lifecycle,this)
        viewpager.registerOnPageChangeCallback(PageChangeCallback())
        bottomNavigationView.setOnNavigationItemSelectedListener { item -> navigationSelected(item) }
    }

    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        findViewById<TextView>(R.id.toolbarText).text =
            MyApplication.getStringCode(9999, getString(R.string.app_name))
        UpdateclientNameTextview()
        setColor()
    }
    fun UpdateclientNameTextview()
    {
        val clientName = MyApplication.getClientName(MyApplication.clientcode ?: "") ?: ""
        val clientnameTextView = findViewById<TextView>(R.id.clientname)
        if(!clientName.isNullOrEmpty()) {
            clientnameTextView.visibility = View.VISIBLE
            clientnameTextView.text = clientName
        }else
            clientnameTextView.visibility = View.GONE
    }
    fun setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        toolbar.setTitleTextColor(Color.parseColor(MyApplication.TextColor))

        bottomNavigationView.itemTextColor = MyApplication.colorStateList
        bottomNavigationView.itemRippleColor = MyApplication.colorStateList
        bottomNavigationView.itemIconTintList = MyApplication.colorStateList
    }
    fun requestAPI(){
        controller.aptotstat({isSucceed, reply, replyArray ->
            if(!isSucceed)return@aptotstat

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){1}
            if(rtncode == 1) {

                //JSON 데이터 해석하고 그 결과구조체를 gatepositionArray에 넣기
                val ArrayAP : MutableList<MyApplication.ItemTotalStatus> = ArrayList()
                val ArrayUsr : MutableList<MyApplication.ItemTotalStatus> = ArrayList()

                for(i in 0 until replyArray.length())
                {
                    val obj: JSONObject = replyArray.getJSONObject(i)

                    // {"ty":"ap", , "idx":"1", "nm":"AP명", "lastdt":"2021-06-09 12:23:44", "st":"a", "temp":"35.2"}
                    try{
                        val input = MyApplication.ItemTotalStatus()
                        input.type = try{obj.getString("ty")}catch(e: JSONException){null}
                        input.idx = try{obj.getString("idx").toIntOrNull()}catch(e: JSONException){null}
                        input.nm = try{obj.getString("nm")}catch(e: JSONException){null}
                        input.lastdt = try{obj.getString("lastdt")}catch(e: JSONException){null}
                        input.st = try{obj.getString("st")}catch(e: JSONException){null}
                        input.temp = try{obj.getString("temp")}catch(e: JSONException){null}
                        if(input.type=="ap")ArrayAP.add(input)
                        else ArrayUsr.add(input)
                    }catch(e: Exception){}
                }
                arrayAP = ArrayAP.toTypedArray()
                arrayUsr = ArrayUsr.toTypedArray()

            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {
                    MyApplication.getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                }
                if(errorstr.toIntOrNull() != null)
                    errorstr = MyApplication.getStringCode(errorstr.toInt())

                val errorRow = MyApplication.SimpleGatePosition()
                errorRow.time = errorstr
                errorRow.gatename = null
                MyApplication.gatepositionArray = arrayOf(errorRow)

                //parent.MessageAlert("admin",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
            
            startTask()
        },idxUser.toString(), clientcode)
    }

    private inner class PagerAdapter(fm: FragmentManager, lc: Lifecycle, m:AdminStatusActivity): FragmentStateAdapter(fm, lc) {
        val parent = m
        override fun getItemCount() = 2
        override fun createFragment(position: Int): Fragment {
            return when (position) {
                0 -> {fragment1ap = FragmentAdminstat_1ap(parent);fragment1ap}
                1 -> {fragment2user = FragmentAdminstat_2user(parent);fragment2user}
                else -> error("no such position: $position") // TODO : String Conversion
            }
        }
    }
    private fun navigationSelected(item: MenuItem): Boolean {
        item.setChecked(true)
        when (item.itemId) {
            R.id.apStatItem     -> viewpager.currentItem = 0
            R.id.clientStatItem -> viewpager.currentItem = 1
            else -> return false
        }
        return true
    }
    private inner class PageChangeCallback: ViewPager2.OnPageChangeCallback() {
        override fun onPageSelected(position: Int) {
            super.onPageSelected(position)
            bottomNavigationView.selectedItemId = when (position) {
                0 -> R.id.apStatItem
                1 -> R.id.clientStatItem
                else -> error("no such position: $position") // TODO : String Conversion
            }
        }
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