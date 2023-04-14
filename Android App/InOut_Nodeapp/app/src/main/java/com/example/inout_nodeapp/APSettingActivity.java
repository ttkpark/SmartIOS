package com.example.inout_nodeapp;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.SelectedAPIndex;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static java.lang.String.format;

public class APSettingActivity extends AppCompatActivity {
    View mainLayout;
    boolean mIsBound;
    ListView List_AP;
    TextView Text_AP;
    Button ReloadBtn;
    MyApplication APP = (MyApplication) getApplication();
    Toolbar toolbar;
    ImageButton Home;
    TextView Title;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ap_setting);
        mainLayout = findViewById(R.id.apSettingparent);
        SetToolBar();
        List_AP = findViewById(R.id.List_AP);
        Text_AP = findViewById(R.id.Text_AP);
        List_AP.setOnItemClickListener(this::SelectListView);
        ReloadBtn = findViewById(R.id.reload);
        ReloadBtn.setText("RELOAD");
        ReloadBtn.setOnClickListener(v-> requestAdminAPI());
        ((TextView)findViewById(R.id.textView)).setText(getStringCode(10108));

        setColor();//색 설정 : 툴바, 재시도 버튼

        Intent intent = new Intent(this,MyService.class);
        mIsBound = bindService(intent,mConnection, Context.BIND_AUTO_CREATE);
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
    void setColor()
    {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        Home.setBackgroundColor(Color.parseColor(AppColor));
        Home.setColorFilter(Color.parseColor(TextColor));
        ReloadBtn.setBackgroundColor(Color.parseColor(AppColor));
        ReloadBtn.setTextColor(Color.parseColor(TextColor));
    }
    @Override
    public void finish() {
        try {
            mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_1));

            Intent intent = new Intent(this,MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.putExtra("isNEW",false);
            startActivity(intent);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }
    boolean isNeedIPSet = false;
    @Override
    protected void onPostResume() {
        super.onPostResume();
        if(isNeedIPSet) {
            isNeedIPSet = false;
            requestAdminAPI();
        }
    }

    void requestAdminAPI()
    {
        try {
            mServiceMessenger.send(Message.obtain(null,MyService.GETADMIN));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }
    class ActivityHandler extends Handler {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                default: super.handleMessage(msg); break;
            }
        }
    }
    Messenger mServiceMessenger = null;
    final Messenger mMessenger = new Messenger(new APSettingActivity.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_3));//Sets the "where to send the reply" of the Service : This Activity

                mServiceMessenger.send(Message.obtain(null,MyService.GETADMIN));

                msg = Message.obtain(null,
                        MyService.SEND_VALUE,123456789,0);
                mServiceMessenger.send(msg);
            }catch (RemoteException e){}
        }

        @Override
        public void onServiceDisconnected(ComponentName name) { mServiceMessenger = null; }
    };

    @Override
    protected void onNewIntent(Intent intent) {
        processIntent(intent);
        super.onNewIntent(intent);
    }
    private void processIntent(Intent intent) {
        if(intent != null)
        {
            String reply = intent.getStringExtra("reply");

            if(reply == null)return;
            if(reply.equals("MessageAlert"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageAlert(Title,Message);
            }
            if(reply.equals("MessageSnack"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                MessageSnack(Title,Message);
            }
            if(reply.equals("MessageTerminate"))
            {
                String Title = intent.getStringExtra("Title");
                String Message = intent.getStringExtra("Message");
                TerminateAlert(Title,Message);
            }
            if(reply.equals("GetAdmin"))
            {
                if (APP.APIResult == APIClass.RESULT_OK) {
                    UpdateListView(APP.APInfoList);
                }
            }
        }
    }
    void SelectListView(AdapterView<?> parent, View view, int position, long id)
    {
        SelectDialog dlg = new SelectDialog(this);
        dlg.setClicklistener(mode->{
            dlg.dismiss();
            Intent intent;
            switch(mode)
            {
                case SelectDialogClicklistener.BTN_GENERAL:
                    intent = new Intent(this,APSettingActivity_general.class);
                    startActivity(intent);
                    break;
                case SelectDialogClicklistener.BTN_IPSET:
                    intent = new Intent(this,APSettingActivity_IP.class);
                    startActivity(intent);
                    break;
            }
        });
        SelectedAPIndex = position;
        isNeedIPSet = true;
        String APName = ((MyApplication.APInfo)List_AP.getAdapter().getItem(position)).apname;
        dlg.show(APName);
    }

    void UpdateListView(MyApplication.APInfo[] apInfos)
    {
        APListAdapter adapter = new APListAdapter(this, apInfos);
        List_AP.setAdapter(adapter);
        Text_AP.setText(format("Total : %3d    ",apInfos.length));
    }
    public void TerminateAlert(String Title,String Messages)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Title).setMessage(Messages)
                .setPositiveButton("Exit", (dialog, which) -> finish())
                .setOnDismissListener(dialog -> finish()).create().show();
    }
    public void MessageAlert(String Title,String Messages)
    {
        new AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
                .setPositiveButton("OK", (dialog, which) -> {//OK
                }).create().show();
    }
    public void MessageSnack(String Title,String Messages)
    {
        Snackbar snackbar = Snackbar.make(mainLayout, Title + "\n" + Messages, Snackbar.LENGTH_LONG);

        snackbar.setAction("OK", v->snackbar.dismiss()).show();//OK
    }
}

class APListAdapter extends BaseAdapter
{
    Context mContext = null;
    LayoutInflater layoutInflater = null;
    MyApplication.APInfo[] list;

    public APListAdapter(Context context, MyApplication.APInfo[] data)
    {
        mContext = context;
        list = data;
        layoutInflater = LayoutInflater.from(mContext);
    }

    @Override
    public int getCount() {
        return list.length;
    }

    @Override
    public Object getItem(int position) {
        return list[position];
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        @SuppressLint({"ViewHolder", "InflateParams"})
        View view = layoutInflater.inflate(R.layout.ap_list_item,null);

        TextView time = view.findViewById(R.id.Text1);
        TextView distance = view.findViewById(R.id.Distance);
        ImageView flagImage = view.findViewById(R.id.flagImage);
        ImageView USEImage = view.findViewById(R.id.imageView);

        String Name = list[position].apname;
        final int limit = 20;
        if(Name.length() > limit)Name = Name.substring(0,limit) + "..";
        time.setText(Name);

        if(!("Y".equals(list[position].useyn)))//Notuse일 때 회색/ 이미지 변경(X로)
        {
            //time.setPaintFlags(time.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
            time.setTypeface(Typeface.DEFAULT_BOLD);
            time.setTextColor(Color.parseColor("#7f7f7f"));

            //distance.setPaintFlags(time.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
            distance.setTypeface(Typeface.DEFAULT_BOLD);
            distance.setTextColor(Color.parseColor("#7f7f7f"));

            USEImage.setImageResource(R.drawable.api_list_item_notuse);
        }

        distance.setText(format("%sm",list[position].dist));

        int flag = 0;
        if(list[position].flag.length() == 0)flag = 3;//if no data
        else flag = Integer.parseInt(list[position].flag);

        int flagid = 0;
        if(!(flag>3 || flag<0))
        {
            int[] flagarray = {R.drawable.ap_list_item_flag0,R.drawable.ap_list_item_flag1,R.drawable.ap_list_item_flag2,R.drawable.ap_list_item_flag3};
            flagid = flagarray[flag];
        }
        if(flagid != 0)
        flagImage.setImageResource(flagid);

        return view;
    }
}
