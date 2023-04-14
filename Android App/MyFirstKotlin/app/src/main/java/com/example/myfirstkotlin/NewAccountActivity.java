package com.example.myfirstkotlin;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;

import static com.example.myfirstkotlin.MyApplication.AppColor;
import static com.example.myfirstkotlin.MyApplication.TextColor;
import static com.example.myfirstkotlin.MyApplication.getStringCode;
import static com.example.myfirstkotlin.MyApplication.getStringCodeHash;
import static com.example.myfirstkotlin.MyApplication.languageCode;
import static com.example.myfirstkotlin.MyApplication.loginType;
import static com.example.myfirstkotlin.MyApplication.saveCodeData;


public class NewAccountActivity extends AppCompatActivity {
    MyController controller;

    Toolbar toolbar;
    ImageButton Home;
    Button OK,Cancel,btnGetHP;
    TextView Title;
    EditText ID,Name,Password,Password2,editPhone;//ClientCode
    Spinner UserType,Language;
    CheckBox Check;

    boolean mIsBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_new_account);
        SetToolBar();

        controller = new MyController(this, Looper.getMainLooper());

        ID = findViewById(R.id.editText101);
        Name = findViewById(R.id.editText102);
        //Clientcode = findViewById(R.id.editText103);
        UserType = findViewById(R.id.Sensor_spinner104);
        Language = findViewById(R.id.Sensor_spinner105);
        Password = findViewById(R.id.editText106);
        Password2 = findViewById(R.id.editText107);
        Check = findViewById(R.id.checkBox);
        editPhone = findViewById(R.id.editTexthp);
        btnGetHP = findViewById(R.id.btnGethp);

        OK = findViewById(R.id.loginbtn);
        Cancel = findViewById(R.id.oncancel);
        OK.setOnClickListener(v->onOK());
        Cancel.setOnClickListener(v->finish());

        SpinnerSetting();

        SetText();
        setColor();//색 설정 : 조회 버튼, 툴바
    }
    boolean isValidText(@NotNull String in)
    {
        return in.length() > 0;
    }
    String ConvertStringInTable(String source, @NotNull String[][] Table) throws ArrayIndexOutOfBoundsException  {
        for(String[] row : Table)
        {
            if(source.equals(row[1]))
                return row[0];
        }
        throw new ArrayIndexOutOfBoundsException();
    }
    void onOK()//now sending
    {
        if(!Check.isChecked())
        {
            Toast.makeText(this, "Please Check and Agree.", Toast.LENGTH_SHORT).show();
            return; // TODO : String Conversion
        }
        if(!Objects.equals(Password.getText().toString(), Password2.getText().toString()))
        {
            Toast.makeText(this, "Check your Password again.", Toast.LENGTH_SHORT).show();
            return; // TODO : String Conversion
        }


        String strID = ID.getText().toString();
        String strName = Name.getText().toString();
        //String strclientcode = Clientcode.getText().toString();
        int Type = UserType.getSelectedItemPosition();
        int languagePos = Language.getSelectedItemPosition();
        String strPassword = Password.getText().toString();
        String strphone = editPhone.getText().toString();

        if (!isValidText(strID) || !isValidText(strName) || /*!isValidText(strclientcode) || */!isValidText(strPassword)) {
            Toast.makeText(this, "Check Text again.", Toast.LENGTH_SHORT).show();
            return;
        }

        String strType,strLanguage;
        try{
            strType = loginType[Type][0];
            strLanguage = languageCode[languagePos][0];
        }catch (ArrayIndexOutOfBoundsException e){
            Toast.makeText(this, "Select Type or Language.", Toast.LENGTH_SHORT).show();
            return;
        }
        getStringCodeHash.put("10903",strType);//선택한 Type
        getStringCodeHash.put("10904",strLanguage);//선택한 language
        saveCodeData();

        controller.join((succeed, reply, replyArray) ->{
            Toast.makeText(this, "join" + reply, Toast.LENGTH_SHORT).show();
        },strphone,strName,strID,strType,strLanguage,strPassword);
    }
    void SpinnerSetting()
    {
        String[] TypeList = new String[loginType.length];
        for(int i=0;i<TypeList.length;i++)
            TypeList[i] = loginType[i][1];
        final ArrayAdapter<String> adapter = new ArrayAdapter<>(this, R.layout.spinner_item, TypeList);
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        UserType.setAdapter(adapter);

        String[] LanguageList = new String[languageCode.length];
        for(int i=0;i<LanguageList.length;i++)
            LanguageList[i] = languageCode[i][1];
        final ArrayAdapter<String> adapter1 = new ArrayAdapter<>(this, R.layout.spinner_item, LanguageList);
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        Language.setAdapter(adapter1);
    }

    @Override
    public void finish() {
        super.finish();

//        Intent intent = new Intent(this, MainActivity.class);
//        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
//        intent.putExtra("isNEW",false);
//        startActivity(intent);
    }

    void SetText() {
        Intent intent = getIntent();
        if(intent != null)
        {
            String email = intent.getStringExtra("email");
            String name  = intent.getStringExtra("name");
            String lang  = intent.getStringExtra("lang");

            ID.setText(email);
            Name.setText(name);

            for(int i=0;i<languageCode.length;i++)
            {
                if(Objects.equals(languageCode[i][0], lang)) {
                    Language.setSelection(i);
                    break;
                }
            }

        }
        //((TextView)findViewById(R.id.textView101)).setText();
    }

    void SetToolBar()
    {
        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        Title = findViewById(R.id.toolbarText);
        Title.setText(getStringCode(9998,getString(R.string.app_name)));
        actionBar.setDisplayShowTitleEnabled(false);
        //Home = findViewById(R.id.imageBtn);
        //Home.setOnClickListener((v)->{
       //     Intent intent = new Intent(this,MainActivity.class);
       //     intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
       //     startActivity(intent);
       // });
    }
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        OK.setBackgroundColor(Color.parseColor(AppColor));
        OK.setTextColor(Color.parseColor(TextColor));
        Cancel.setBackgroundColor(Color.parseColor(AppColor));
        Cancel.setTextColor(Color.parseColor(TextColor));
        btnGetHP.setBackgroundColor(Color.parseColor(AppColor));
        btnGetHP.setTextColor(Color.parseColor(TextColor));
    }


    public void TerminateAlert(String Title,String Messages)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Title).setMessage(Messages)
                .setPositiveButton("Exit", (dialog, which) -> finish()) // TODO : String Conversion
                .setOnDismissListener(dialog -> finish()).create().show();
    }
    public void MessageAlert(String Title,String Messages)
    {
        new AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
                .setPositiveButton("OK", (dialog, which) -> { // TODO : String Conversion
                }).create().show();
    }
}