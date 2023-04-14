package com.example.myfirstkotlin

import android.content.DialogInterface
import android.graphics.Color
import android.graphics.Color.*
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.view.MenuItem
import android.view.View
import android.widget.Button
import android.widget.ImageButton
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.widget.Toolbar
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.google.android.material.bottomnavigation.BottomNavigationView

class SettingsActivity : AppCompatActivity() {
    lateinit var controller:MyController
    lateinit var backBtn : ImageButton
    lateinit var viewTitle : TextView
    lateinit var toolbar: Toolbar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_settings)

        controller = MyController(this, Looper.getMainLooper())

        val fragment = Fragment_account(this)

        supportFragmentManager.beginTransaction()
            .add(R.id.framelayout,fragment).commit()

        backBtn = findViewById(R.id.backButton)
        backBtn.setOnClickListener { finish() }

        setToolBar()
    }
    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        viewTitle = findViewById(R.id.toolbarText)
        viewTitle.text = "Account" // TODO : String Conversion
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
    fun setColor() {
        toolbar.setBackgroundColor(parseColor(AppColor))
        toolbar.setTitleTextColor(parseColor(TextColor))
    }
    fun MessageAlert(Title: String?, Messages: String?) {
        AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
            .setPositiveButton("OK") { _: DialogInterface?, _: Int -> }.create().show()
    } // TODO : String Conversion

}