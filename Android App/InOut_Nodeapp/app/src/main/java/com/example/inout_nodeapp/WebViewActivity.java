package com.example.inout_nodeapp;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ImageButton;
import android.widget.TextView;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static com.example.inout_nodeapp.MyApplication.idxUser;

public class WebViewActivity extends AppCompatActivity {
    Toolbar toolbar;
    ImageButton Home;
    TextView Title;
    WebView webView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_web_view);
        webView = findViewById(R.id.WebView);

        SetToolBar();
        setText();
        setColor();//색 설정. : 툴바 OK버튼

        webView.getSettings().setJavaScriptEnabled(true);
        webView.setWebChromeClient(new WebChromeClient());
        webView.setWebViewClient(new WebViewClientClass());
        //String url = getStringCode(9996) + "?hp=" + phoneNumber.replace("-","");
        //webView.loadUrl(url);
        String url = getStringCode(9996);
        String postdata = "idxuser=" + idxUser;//new String(Base64Encode(phoneNumber.replace("-","").getBytes(),0));
        webView.postUrl(url, postdata.getBytes());
    }

    void SetToolBar()
    {
        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        Title = findViewById(R.id.toolbarText);
        Title.setText(getStringCode(9998,getString(R.string.app_name)));
        actionBar.setDisplayShowTitleEnabled(false);
        Home = findViewById(R.id.imageBtn);
        Home.setOnClickListener((v)->{
            Intent intent = new Intent(this,MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.putExtra("isNEW",false);
            startActivity(intent);
        });
    }
    void setText()
    {

    }
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        Home.setBackgroundColor(Color.parseColor(AppColor));
        Home.setColorFilter(Color.parseColor(TextColor));
    }
    static boolean isFirst = true;
    private class WebViewClientClass extends WebViewClient {//페이지 이동
        @Override
        public void onPageFinished(WebView view, String url) {
            Log.d("check URL", url);
            if (url.contains("Signin"))
            {
                if(isFirst) {
                    isFirst = false;
                    String urls = getStringCode(9996);
                    String postdata = "idxuser=" + idxUser;//new String(Base64Encode(phoneNumber.replace("-","").getBytes(),0));
                    webView.postUrl(urls, postdata.getBytes());
                }else finish();
            }

            super.onPageFinished(view, url);
        }
        /*

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

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if((keyCode == KeyEvent.KEYCODE_BACK) && webView.canGoBack())
        {
            webView.goBack();
            return true;
        }else{
            finish();
            return true;
        }
        //return super.onKeyDown(keyCode, event);
    }
}