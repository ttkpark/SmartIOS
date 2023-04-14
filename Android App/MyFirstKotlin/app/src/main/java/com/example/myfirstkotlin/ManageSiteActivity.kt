package com.example.myfirstkotlin

import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.Color.parseColor
import android.os.Bundle
import android.os.Looper
import android.util.Log
import android.view.KeyEvent
import android.view.View
import android.view.animation.Animation
import android.view.animation.AnimationUtils
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.ImageButton
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.idxUser

class ManageSiteActivity : AppCompatActivity() {
    lateinit var controller: MyController
    lateinit var webView : WebView
    lateinit var backButton : ImageButton
    lateinit var loadingPicture : View
    lateinit var loadingTextUpper : View
    lateinit var loadICONAnimation: Animation
    lateinit var loadTextAnimation :Animation
    lateinit var viewTitle : TextView
    lateinit var toolbar: Toolbar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_manage_site)

        webView = findViewById(R.id.WebView)
        backButton = findViewById(R.id.backButton)
        loadingPicture = findViewById(R.id.loadingImage)
        loadingTextUpper = findViewById(R.id.LoadingTextUp)
        loadICONAnimation = AnimationUtils.loadAnimation(this,R.anim.load_icon_animation)
        loadTextAnimation = AnimationUtils.loadAnimation(this,R.anim.loading_text)
        controller = MyController(this, Looper.getMainLooper())


        webView.settings.javaScriptEnabled = true
        webView.webChromeClient = WebChromeClient()
        webView.webViewClient = WebViewClientClass(this)

        backButton.setOnClickListener { finish() }
        pageReset()

        setToolBar()
    }

    fun setToolBar()
    {
        toolbar = findViewById(R.id.toolbar) //툴바 설정
        setSupportActionBar(toolbar)
        title = ""
        viewTitle = findViewById(R.id.toolbarText)
        viewTitle.text = "ManageSite" // TODO : String Conversion
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

    fun animationON()
    {
        loadingPicture.startAnimation(loadICONAnimation)
        loadingTextUpper.startAnimation(loadTextAnimation)
    }
    fun animationOFF()
    {
        loadingPicture.clearAnimation()
        loadingTextUpper.clearAnimation()

        loadingPicture.visibility = View.INVISIBLE
        loadingTextUpper.visibility = View.INVISIBLE
    }

    fun pageReset()
    {
        controller.GetnerateAESKey300()

        val valEncoded = String(CryptUtil.Base64Encode(CryptUtil.Base64Encode(controller.Crypt.AESEncode("${clientcode}^${idxUser}", controller.AESNo),0),0))

        val url: String = getStringCode(9996)
        val postdata ="val1=${valEncoded}&val2=${controller.DValue}${controller.CValue}&val3=atologin"
        webView.postUrl(url, postdata.toByteArray())
    }


    var isFirst = true

    private class WebViewClientClass(m:ManageSiteActivity) : WebViewClient() {
        val parent = m
        override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?)
        {
            if(url != null) {
                Log.d("check URL", url)
                if (url.contains("Signin")) {
                    if (parent.isFirst) {
                        parent.isFirst = false
                        parent.pageReset()
                    } else parent.finish();//parent.pageReset()
                }
            }
            parent.animationON()

            super.onPageStarted(view, url, favicon)
        }
        override fun onPageFinished(view: WebView, url: String) {

            parent.animationOFF()
            super.onPageFinished(view, url)
        } /*

        @Override
        public void onPageFinished(WebView view, String url) {
            if(isFirst)
            {
                String postdata = "hp=" + phoneNumber.replace("-","");//new String(Base64Encode(phoneNumber.replace("-","").getBytes(),0));
                //Log.d("postdata",postdata);
                //view.postUrl(getStringCode(9996), postdata.getBytes());
                //Log.i("postdata2",postdata);
                isFirst = false;
            }
            super.onPageFinished(view, url);
        }
        */
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        return if (keyCode == KeyEvent.KEYCODE_BACK && webView.canGoBack()) {
            webView.goBack()
            true
        } else {
            finish()
            //pageReset()
            false
        }
        return super.onKeyDown(keyCode, event);
    }
}