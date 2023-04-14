package com.example.myfirstkotlin

import android.content.DialogInterface
import android.content.Intent
import android.content.res.ColorStateList
import android.graphics.Color
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.view.Menu
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
import com.example.myfirstkotlin.MyApplication.Companion.SelectedidxEquip
import com.example.myfirstkotlin.MyApplication.Companion.colorStateList
import com.google.android.material.bottomnavigation.BottomNavigationView

class APSettingActivity : AppCompatActivity() {
    lateinit var fragment1detail : FragmentAP_1APdetail
    lateinit var fragment2apip : FragmentAP_2APIP
    lateinit var fragment3apgps : FragmentAP_3APGPS

    lateinit var bottomNavigationView : BottomNavigationView
    lateinit var viewpager : ViewPager2
    lateinit var QRButton : ImageButton
    lateinit var backBtn : ImageButton
    lateinit var viewTitle : TextView

    lateinit var toolbar: Toolbar

    lateinit var controller: MyController

    var pagearray = arrayOf(R.id.apDetailItem,R.id.apIPItem,R.id.apGPSItem)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_apsetting)

        if (intent != null) {
            val idxEquipStr = intent.getStringExtra("idxEquip")
            SelectedidxEquip = Integer.parseInt(idxEquipStr)
        }

        controller = MyController(this, Looper.getMainLooper())
        setToolBar()
        viewpager = findViewById(R.id.viewpager)
        bottomNavigationView = findViewById(R.id.bottomNavigationView)
        viewpager.adapter = PagerAdapter(supportFragmentManager, lifecycle,this)
        viewpager.registerOnPageChangeCallback(PageChangeCallback())

        try{
            val item = intent.getIntExtra("focus",0)
            val menuItem = bottomNavigationView.menu.getItem(item)
            viewpager.currentItem = item
            menuItem.isChecked = true
            viewTitle.text = menuItem.title
        }catch(e:IndexOutOfBoundsException){e.printStackTrace()}

        bottomNavigationView.setOnNavigationItemSelectedListener { item -> navigationSelected(item) }

        backBtn = findViewById(R.id.backButton)
        backBtn.setOnClickListener { finish() }

        setColor()
    }
    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        viewTitle = findViewById(R.id.toolbarText)
        UpdateclientNameTextview()
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

        bottomNavigationView.itemTextColor = colorStateList
        bottomNavigationView.itemRippleColor = colorStateList
        bottomNavigationView.itemIconTintList = colorStateList
    }

    private inner class PagerAdapter(fm: FragmentManager, lc: Lifecycle, m:APSettingActivity): FragmentStateAdapter(fm, lc) {
        val parent = m
        override fun getItemCount() = 3
        override fun createFragment(position: Int): Fragment {
            return when (position) {
                0 -> {fragment1detail = FragmentAP_1APdetail(parent);fragment1detail}
                1 -> {fragment2apip = FragmentAP_2APIP(parent);fragment2apip}
                2 -> {fragment3apgps = FragmentAP_3APGPS(parent);fragment3apgps}
                else -> error("no such position: $position") // TODO : String Conversion
            }
        }
    }
    private fun navigationSelected(item: MenuItem): Boolean {
        item.isChecked = true
        viewTitle.text = item.title//bottomNavigationView.menu.ites.title
        when (item.itemId) {
            R.id.apDetailItem -> viewpager.currentItem = 0
            R.id.apIPItem     -> viewpager.currentItem = 1
            R.id.apGPSItem    -> viewpager.currentItem = 2
            else -> return false
        }
        return true
    }
    private inner class PageChangeCallback: ViewPager2.OnPageChangeCallback() {
        override fun onPageSelected(position: Int) {
            super.onPageSelected(position)
            when (position) {
                0 -> { try{fragment1detail.onFocus()}catch(_:Throwable){}; }
                1 -> { try{fragment2apip.onFocus()}catch(_:Throwable){}; }
                2 -> { try{fragment3apgps.onFocus()}catch(_:Throwable){}; }
                else -> error("no such position: $position") // TODO : String Conversion
            }
            bottomNavigationView.selectedItemId = pagearray[position]
        }
    }
    fun callAPListActivity(focus:Int)
    {
        intent = Intent(this,APSettingListActivity::class.java)
        intent.putExtra("focus",focus)
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
        startActivity(intent)
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