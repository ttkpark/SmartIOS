package com.example.inout_nodeapp;

import android.app.Dialog;
import android.content.Context;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;

import static com.example.inout_nodeapp.MyApplication.getStringCode;

public class AttendRemarkDialog extends Dialog {
    AttendRemarkDialogEvents listener;
    ImageButton loginBtn,logoutBtn, deleteBtn;
    Button closeBtn;
    TextView text_Login,text_Logout,subTitle,text_Title;
    String strsubTitle;
    String strTitle;
    boolean isDeleteMode = false;

    public AttendRemarkDialog(Context context) {
        super(context);
    }
    public AttendRemarkDialog setListener(AttendRemarkDialogEvents Listener)
    {
        listener = Listener;
        return this;
    }
    public AttendRemarkDialog setsubTitle(String StrsubTitle)
    {
        strsubTitle = StrsubTitle;
        return this;
    }
    public AttendRemarkDialog setTitle(String strTitle)
    {
        this.strTitle = strTitle;
        return this;
    }

    void setDeleteMode(boolean isDelete)
    {
        isDeleteMode = isDelete;
    }

    @Override
    public void dismiss() {
        super.dismiss();
        listener.onDismiss();
    }

    @Override
    public void show() {
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        if(isDeleteMode) {
            setContentView(R.layout.attend_remark_delete_dialog);
            deleteBtn = findViewById(R.id.Btn_Login);
            text_Login = findViewById(R.id.text_Login);
            text_Login.setText(getStringCode(81));//삭제
        }else {
            setContentView(R.layout.attend_remark_dialog);
            loginBtn = findViewById(R.id.Btn_Login);
            logoutBtn = findViewById(R.id.Btn_Logout);
            text_Login = findViewById(R.id.text_Login);
            text_Logout = findViewById(R.id.text_Logout);
            text_Login.setText(getStringCode(783));//출근
            text_Logout.setText(getStringCode(784));//퇴근
        }

        subTitle = findViewById(R.id.InformationText);
        closeBtn = findViewById(R.id.closebutton);
        text_Title = findViewById(R.id.Title);
        subTitle.setText(strsubTitle);
        text_Title.setText(strTitle);

        if(listener != null)
        {
            if(isDeleteMode)
                deleteBtn.setOnClickListener(v->listener.onDelete());
            else {
                loginBtn.setOnClickListener(v->listener.onLoginPressed());
                logoutBtn.setOnClickListener(v->listener.onLogoutPressed());
            }
            closeBtn.setOnClickListener(v->dismiss());
        }
        super.show();
    }
}

interface AttendRemarkDialogEvents{
    void onLoginPressed();//퇴근 눌렸을 때
    void onLogoutPressed();//출근 눌렸을 때
    void onDismiss();
    void onDelete();//삭제 눌렀을 때
}
