package com.example.inout_nodeapp;


import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import static com.example.inout_nodeapp.MyApplication.AppColor;

class SelectDialog extends Dialog
{
    Context context;
    SelectDialog(Context m_context){
        super(m_context);
    }
    TextView APName;
    static Button btnGeneral;
    static Button btnIPSetting;
    static Button btndomainSet;
    SelectDialogClicklistener selectDialogClicklistener;

    public void setClicklistener(SelectDialogClicklistener clicklistener){
        selectDialogClicklistener = clicklistener;
    }

    public void show(String apname){
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.ap_mode_select_dialog);
        super.show();

        APName = findViewById(R.id.Dialog_APname);
        btnGeneral = findViewById(R.id.Dialog_Btn_General);
        btnIPSetting = findViewById(R.id.Dialog_Btn_IP);
        //btndomainSet = findViewById(R.id.Dialog_Btn_Doamin);

        APName.setText(apname);
        btnGeneral.setOnClickListener(v->{selectDialogClicklistener.onClick(0);});
        btnIPSetting.setOnClickListener(v->{selectDialogClicklistener.onClick(1);});
        //btndomainSet.setOnClickListener(v->{selectDialogClicklistener.onClick(2);});

        setColor();// 버튼 2개 색 설정
    }
    void setColor()
    {
        btnGeneral.setTextColor(Color.parseColor(AppColor));
        btnIPSetting.setTextColor(Color.parseColor(AppColor));
    }
}
interface SelectDialogClicklistener
{
    final int BTN_GENERAL = 0;
    final int BTN_IPSET = 1;
    //final int BTN_DOMAIN = 2;
    void onClick(int mode);
}