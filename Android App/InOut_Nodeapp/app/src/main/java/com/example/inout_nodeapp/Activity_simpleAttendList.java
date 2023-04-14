package com.example.inout_nodeapp;

import android.annotation.SuppressLint;
import android.app.DatePickerDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;

import static com.example.inout_nodeapp.MyApplication.AppColor;
import static com.example.inout_nodeapp.MyApplication.GatePositionToRemark;
import static com.example.inout_nodeapp.MyApplication.TextColor;
import static com.example.inout_nodeapp.MyApplication.getStringCode;
import static java.lang.String.format;

public class Activity_simpleAttendList extends AppCompatActivity {
    View mainLayout;
    boolean mIsBound = false;
    Button btn;
    EditText editDate;
    String DateString;
    Button RetryBtn;
    TextView text;
    GridView gridView;

    int setYear;
    int setMonth;
    int setDay;

    Toolbar toolbar;
    ImageButton Home;
    TextView Title;

    // 1. smlst 불러와 GridView에 바둑판식으로 배열한다.
    //    { "seq":"0","io_dt":"2021-03-26 03:25:56.559", "io_type":"io_min_o", "gatename" : "출입문", "io":"" }
    // 2. 입출입 부분을 쿨릭-> 출근퇴근 대화상자가 뜬다.-> (출퇴근)선택에 따라 설정
    // 츨퇴근이 이미 마킹되면 할 필요 없다.
    //  조건
    // > 출석(근) 처리 : 두번째 값(io_min)  중 io=i가 아니면 팝업창 띄운다.
    // > 퇴근(실) 처리 : 세번재 값(io_max)이 있으면 팝업창을 띄운다.

    GregorianCalendar dateSetting;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_actitvity_simple_attend_list);
        SetToolBar();
        mainLayout = findViewById(R.id.list_Layout);
        editDate = findViewById(R.id.editDate);
        RetryBtn = findViewById(R.id.button);
        gridView = findViewById(R.id.Gridview);
        text = findViewById(R.id.textbox);
        btn = findViewById(R.id.Next);

        editDate.setOnClickListener(v -> editDateClick());
        RetryBtn.setOnClickListener(v -> RequestListing());
        btn.setOnClickListener(v->{
                    Intent intent = new Intent(this,Activity_AttendList.class);
                    intent.putExtra("date",dateSetting);
                    startActivity(intent);
                });
        RetryBtn.setText(getStringCode(5999));//조회
        btn.setText(getStringCode(169));//상세내역 보기
        text.setText("상세보기");

        setColor();//색 설정 : 조회 버튼, 툴바

        if(dateSetting == null) {
            dateSetting = new GregorianCalendar();
            setDateAndDateString(dateSetting.get(Calendar.YEAR), dateSetting.get(Calendar.MONTH) + 1, dateSetting.get(Calendar.DATE));
            editDate.setText(DateString);
        }
        gridView.setOnItemClickListener((parent, view, position, id)->onGridViewSelected(position));

        UpdateListView(MyApplication.SimpleGateTimeList);

        if(mServiceMessenger == null) {
            Intent intent = new Intent(this, MyService.class);
            mIsBound = bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
    }
    void SetToolBar() {
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
    void setColor() {
        toolbar.setBackgroundColor(Color.parseColor(AppColor));
        Title.setBackgroundColor(Color.parseColor(AppColor));//색 설정
        Title.setTextColor(Color.parseColor(TextColor));
        Home.setBackgroundColor(Color.parseColor(AppColor));
        Home.setColorFilter(Color.parseColor(TextColor));
        RetryBtn.setBackgroundColor(Color.parseColor(AppColor));
        RetryBtn.setTextColor(Color.parseColor(TextColor));
    }
    void setDateAndDateString(int Y,int M,int d) {
        setYear = Y;
        setMonth = M;
        setDay = d;
        DateString = format("%04d%02d%02d", Y, M, d);
    }
    void RequestListing() {
        Log.d("RequestListing","clicked");
        Log.d("RequestListing",DateString);
        if(mIsBound){
            try {
                mServiceMessenger.send(Message.obtain(null,MyService.LISTING_SIMPLE,DateString));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    void editDateClick() {
        Log.d("editDateClick","clicked");
        DatePickerDialog dlg = new DatePickerDialog(this, new DatePickerDialog.OnDateSetListener() {
            @Override
            public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                setDateAndDateString(year,month+1,dayOfMonth);
                editDate.setText(DateString);
                dateSetting.set(year,month,dayOfMonth);
                RequestListing();
            }
        }, setYear, setMonth-1, setDay);
        dlg.show();
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
    final Messenger mMessenger = new Messenger(new Activity_simpleAttendList.ActivityHandler());
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mServiceMessenger = new Messenger(service);
            try{
                Message msg = Message.obtain(null,MyService.CONNECT);
                msg.replyTo = mMessenger;
                mServiceMessenger.send(msg);

                mServiceMessenger.send(Message.obtain(null,MyService.ACTIVITY_LISTING_SIMPLE));

                mServiceMessenger.send(Message.obtain(null,MyService.LISTING_SIMPLE));

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
                MyApplication.SimpleGatePosition[] gatePositions;

                gatePositions = MyApplication.SimpleGateTimeList;

                for(int i=0;i<gatePositions.length;i++)
                {
                    //Log.d("List[" + i + "/" + gatePositions.length + "]","time : " + gatePositions[i].time + " gatename :" + gatePositions[i].gatename);
                }

                UpdateListView(gatePositions);
            }
            if(reply.equals("remarkSucceed"))
            {
                RequestListing();
            }
        }
    }
    void UpdateListView(MyApplication.SimpleGatePosition[] gatePositions)
    {
        Log.i("UpdateGridView","GridView Updating start!");
        if(gatePositions == null)gatePositions = new MyApplication.SimpleGatePosition[0];

        StringBuilder builder = new StringBuilder();
        for(int i=0;i<gatePositions.length;i++)
        {
            if(gatePositions[i] == null)continue;
            builder.append(String.format("%s : %s(%s)\n",gatePositions[i].io_type,gatePositions[i].gatename,gatePositions[i].time));
        }
        Log.i("UpdateGridView",builder.toString());

        text.setText(builder.toString());
        MyGridViewAdapter adapter = new MyGridViewAdapter(new ArrayList<>(Arrays.asList(gatePositions)),this);
        gridView.setAdapter(adapter);
    }
    void onGridViewSelected(int position) {
        int timediff = 1;//Integer.parseInt(getStringCode(9990));
        GregorianCalendar today_minus_diff = new GregorianCalendar();
        GregorianCalendar value_dateSetting = (GregorianCalendar) dateSetting.clone();

        today_minus_diff.add(Calendar.DAY_OF_MONTH,-timediff + 1);
        today_minus_diff.set(Calendar.HOUR_OF_DAY,0);
        today_minus_diff.set(Calendar.MINUTE,0);
        today_minus_diff.set(Calendar.SECOND,0);
        today_minus_diff.set(Calendar.MILLISECOND,0);

        value_dateSetting.set(Calendar.HOUR_OF_DAY,0);
        value_dateSetting.set(Calendar.MINUTE,0);
        value_dateSetting.set(Calendar.SECOND,0);
        today_minus_diff.set(Calendar.SECOND,0);
        today_minus_diff.set(Calendar.MILLISECOND,0);

        if(value_dateSetting.getTime().compareTo(today_minus_diff.getTime()) < 0) {
            Toast.makeText(this, "Date is over.", Toast.LENGTH_SHORT).show();
            return;
        }


        GatePositionToRemark = new MyApplication.GatePosition();
        try {
            GatePositionToRemark.gatename = MyApplication.SimpleGateTimeList[position].gatename;
            GatePositionToRemark.time = MyApplication.SimpleGateTimeList[position].time;
        }catch (ArrayIndexOutOfBoundsException e){return;}
        if(MyApplication.SimpleGateTimeList[position].time.length() < 2)return;//filter the invalid data

        String io_Type = MyApplication.SimpleGateTimeList[position].io_type;
        String io = MyApplication.SimpleGateTimeList[position].io;

        if( io_Type.equals("io_max") || io_Type.equals("io_min") && !io.equals("i"))
        {
            return;//입출입을 눌러 마킹하는 기능은 제공하지 않는다.
        }

        AttendRemarkDialog dialog = new AttendRemarkDialog(this);

        //2번째이고 io고 i라면(출근 처리된 내용을 눌렀다면) 삭제버튼을 출력한다.
        if(!(io_Type.equals("io_min") || io_Type.equals("io_max")) ||  // 2,3번째가 아니면 삭제 버튼을 띄워준다.
                io_Type.equals("io_min") && io.equals("i")) //'2번째' '출근'을 눌렀을 때
        {
            dialog.setDeleteMode(true);

            if(io_Type.equals("io_min_o") || io_Type.equals("io_max_o"))//퇴근
                 dialog.setTitle(getStringCode(784)); //퇴근 (취소)
            else dialog.setTitle(getStringCode(783)); //출근 (취소)

            //Title : getStringCode(783) + "/" + getStringCode(784);
        }
        dialog.setsubTitle(GatePositionToRemark.time);
        // 상태      제목
        // io_min_o  퇴근   취소
        // io_min (i)출근   취소
        // io_max    X
        // io_max_o  퇴근   취소

        //입출입일때 나오지 않는다.


        dialog.setListener(new AttendRemarkDialogEvents() {
            @Override
            public void onLoginPressed() {if(mIsBound)
                try {
                    mServiceMessenger.send(Message.obtain(null,MyService.LISTING_REMARK,1,0));
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                dialog.dismiss();
            }

            @Override
            public void onLogoutPressed() {if(mIsBound)
                try {
                    mServiceMessenger.send(Message.obtain(null,MyService.LISTING_REMARK,0,0));
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                dialog.dismiss();
            }
            @Override
            public void onDelete() {if(mIsBound)
                try {
                    mServiceMessenger.send(Message.obtain(null,MyService.LISTING_REMARK_CANCEL
                            ,io_Type.equals("io_min") && io.equals("i") ? 1 : 0 // 두번째 눌린 상황이 아니면(출근을 취소한다면 1, 퇴근을 취소하면 0)
                            ,0));
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                dialog.dismiss();
            }

            @Override
            public void onDismiss() {}
        });
        dialog.show();

        WindowManager.LayoutParams params = dialog.getWindow().getAttributes();// 다이얼로그 창을 화면에 맞게 채워준다.
        params.width = WindowManager.LayoutParams.MATCH_PARENT;
        params.height = WindowManager.LayoutParams.WRAP_CONTENT;
        dialog.getWindow().setAttributes(params);

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

class MyGridViewAdapter extends BaseAdapter {

    ArrayList<MyApplication.SimpleGatePosition> items;
    Context context;
    Drawable backborder,GreenFilteredborder;

    @SuppressLint("UseCompatLoadingForDrawables")
    public MyGridViewAdapter(ArrayList<MyApplication.SimpleGatePosition> items, Context context){
        this.items = items;
        this.context = context;
        backborder = context.getResources().getDrawable(R.drawable.gridview_corners);

        GreenFilteredborder = context.getResources().getDrawable(R.drawable.gridview_corners_green);
        //GreenFilteredborder.setColorFilter(Color.GREEN, PorterDuff.Mode.SRC_ATOP);
        //GreenFilteredborder = context.getResources().getDrawable(R.drawable.gridview_corners);
        //GreenFilteredborder.setColorFilter(Color.GREEN, PorterDuff.Mode.DST_OUT);
    }

    @Override
    public int getCount() {
        return items.size();
    }

    @Override
    public Object getItem(int i) {
        return items.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    String[] ioType =    {"io_min_o"        ,"io_min","io_max"          ,"io_max_o"};
    String[] ioTypestr = {getStringCode(784),"io_min",getStringCode(839),getStringCode(784)};

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {

        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        view = inflater.inflate(R.layout.gridview_element, viewGroup, false);

        LinearLayout linearLayout = view.findViewById(R.id.BorderLayout);

        TextView Title = view.findViewById(R.id.Title);
        TextView Contents = view.findViewById(R.id.Contents);

        if(items.get(i) != null) {

            //글자 io_min_o,io_max_o,io_min,io_max를 제목으로 바꾼다.
            for(int j=0;j<ioType.length;j++)
                if(ioType[j].equals(items.get(i).io_type)){
                    Title.setText(ioTypestr[j]);
                    break;
                }

            //io_min이면서 io=i라면 "출근" 표시
            if(items.get(i).io_type.equals(ioType[1]))
            {
                if(items.get(i).io.equals("i"))
                    Title.setText(getStringCode(783));//출근
                else //아니면 "입출입"
                    Title.setText(getStringCode(839));//입출입
            }

            // 출근이라면 초록색 테두리 적용
            if(items.get(i).io_type.equals(ioType[1]) && items.get(i).io.equals("i"))
                linearLayout.setBackground(GreenFilteredborder);
            else linearLayout.setBackground(backborder);

            // 시간 데이터 해석 (날짜) (시간).밀리초
            // (시간)만 가져온다.
            String[] times = items.get(i).time.split(" ");
            if(times.length!=2)
                Contents.setText(items.get(i).time);
            else
                Contents.setText(times[1].substring(0, times[1].length() - 4));
        }

        return view;
    }

}