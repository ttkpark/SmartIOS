package com.example.inout_nodeapp;

import android.annotation.SuppressLint;
import android.app.DatePickerDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;

import java.util.Calendar;
import java.util.GregorianCalendar;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static java.lang.String.format;

public class Activity_AttendList extends AppCompatActivity {
    View mainLayout;
    boolean mIsBound = false;
    ListView attendlist;
    EditText editDate;
    String DateString;
    Button RetryBtn;

    int setYear;
    int setMonth;
    int setDay;

    Toolbar toolbar;
    ImageButton Home;
    TextView Title;
    GregorianCalendar dateSetting;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_attend_list);
        SetToolBar();
        mainLayout = findViewById(R.id.list_Layout);
        editDate = findViewById(R.id.editDate);
        attendlist = findViewById(R.id.attendlist);
        RetryBtn = findViewById(R.id.button);

        editDate.setOnClickListener(v -> editDateClick());
        RetryBtn.setOnClickListener(v -> RequestListing());
        RetryBtn.setText(getStringCode(5999));//조회

        Intent intent = getIntent();
        if(intent != null) {
            GregorianCalendar post = (GregorianCalendar)intent.getSerializableExtra("date");
            if (post != null) { // 시간 업데이트
                dateSetting = post;
                setDateAndDateString(dateSetting.get(Calendar.YEAR),dateSetting.get(Calendar.MONTH)+1,dateSetting.get(Calendar.DATE));
            }
            else
            {
                dateSetting = new GregorianCalendar();
                setDateAndDateString(dateSetting.get(Calendar.YEAR),dateSetting.get(Calendar.MONTH)+1,dateSetting.get(Calendar.DATE));
            }
        }

        setColor();//색 설정 : 조회 버튼, 툴바

        editDate.setText(DateString);

        if(mServiceMessenger == null) {
            intent = new Intent(this, MyService.class);
            mIsBound = bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
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
        RetryBtn.setBackgroundColor(Color.parseColor(AppColor));
        RetryBtn.setTextColor(Color.parseColor(TextColor));
    }
    void setDateAndDateString(int Y,int M,int d)
    {
        setYear = Y;
        setMonth = M;
        setDay = d;
        DateString = format("%04d%02d%02d", Y, M, d);
    }
    void RequestListing()
    {
        if(mIsBound){
            try {
                mServiceMessenger.send(Message.obtain(null,MyService.LISTING,DateString));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    void editDateClick()
    {
        DatePickerDialog dlg = new DatePickerDialog(this, new DatePickerDialog.OnDateSetListener() {
            @Override
            public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                setDateAndDateString(year,month+1,dayOfMonth);
                editDate.setText(DateString);
                RequestListing();
            }
        }, setYear, setMonth-1, setDay);
        dlg.show();
    }

    @Override
    public void finish() {
        try {
            mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_LISTING_SIMPLE));
            super.finish();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    class ActivityHandler extends Handler
    {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what)
            {
                default: super.handleMessage(msg); break;
            }
        }
    }
    Messenger mServiceMessenger = null;
    final Messenger mMessenger = new Messenger(new Activity_AttendList.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_2));

                mServiceMessenger.send(Message.obtain(null,MyService.LISTING,DateString));

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
    private void processIntent(Intent intent)
    {
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
            if(reply.equals("ListFailed")) {

                UpdateListView(null);
            }
            if(reply.equals("List"))
            {
                MyApplication.GatePosition[] gatePositions;

                MyApplication APP = (MyApplication) getApplication();
                gatePositions = APP.GateTimeList;

                for(int i=0;i<gatePositions.length;i++)
                {
                    //Log.d("List[" + i + "/" + gatePositions.length + "]","time : " + gatePositions[i].time + " gatename :" + gatePositions[i].gatename);
                }

                UpdateListView(gatePositions);
            }
        }
    }
    void UpdateListView(MyApplication.GatePosition[] gatePositions)
    {
        if(gatePositions == null)gatePositions = new MyApplication.GatePosition[0];

        AttendListAdapter adapter = new AttendListAdapter(this, gatePositions);
        attendlist.setAdapter(adapter);
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
    /*
    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        switch(item.getItemId())
        {
            case android.R.id.home:
                Intent intent = new Intent(this,MainActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                startActivity(intent);

                return true;
        }
        return super.onOptionsItemSelected(item);
    }
    */
}

class AttendListAdapter extends BaseAdapter
{
    Context mContext = null;
    LayoutInflater layoutInflater = null;
    MyApplication.GatePosition[] list;

    public AttendListAdapter(Context context, MyApplication.GatePosition[] data)
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
        View view = layoutInflater.inflate(R.layout.attend_list_item,null);

        TextView time = view.findViewById(R.id.attend_list_Time);
        TextView door = view.findViewById(R.id.attend_list_door);

        time.setText(list[position].time);
        door.setText(list[position].gatename);

        return view;
    }
}