package com.example.myfirstkotlin;

import android.app.Dialog;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import static com.example.myfirstkotlin.MyApplication.getStringCode;

interface SelectClientListener
{
    void OnDismiss();
}

public class SelectClientDialog extends Dialog {

    SelectClientListener listener;

    public Dialog dialog;
    TextView Title;
    TextView Infotext;
    Button closebutton;
    RecyclerView recyclerView;
    private RecyclerView.LayoutManager mLayoutManager;
    RecyclerView.Adapter adapter;
    MyApplication.ClientInfo[] clientInfoarray;


    public SelectClientDialog(Context a, RecyclerView.Adapter adapter,MyApplication.ClientInfo[] clientInfoarray) {
        super(a);
        this.adapter = adapter;
        this.clientInfoarray = clientInfoarray;
    }
    public void setListener(SelectClientListener listener) { this.listener = listener; }

    void show(String title,String infoText,Integer buttonColor){
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.select_clientcode_dialog);
        super.setOnDismissListener((dlgint) ->
        {
            this.dismiss();
            listener.OnDismiss();
        });

        Title = findViewById(R.id.Title);
        Infotext = findViewById(R.id.InformationText);
        closebutton = findViewById(R.id.closebutton);
        Title.setText(title);
        Infotext.setText(infoText);
        closebutton.setOnClickListener(v->dismiss());
        closebutton.setTextColor(buttonColor);

        String[] strings = new String[clientInfoarray.length];
        for(int i=0;i<clientInfoarray.length;i++)
            strings[i] = clientInfoarray[i].getClientName();


        recyclerView = findViewById(R.id.recycler_view);
        mLayoutManager = new LinearLayoutManager(getContext());
        recyclerView.setLayoutManager(mLayoutManager);
        recyclerView.setAdapter(adapter);

        super.show();
    }
}