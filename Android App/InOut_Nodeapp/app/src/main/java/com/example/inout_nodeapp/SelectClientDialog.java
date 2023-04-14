package com.example.inout_nodeapp;

import android.app.Dialog;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import static com.example.inout_nodeapp.MyApplication.companyList;
import static com.example.inout_nodeapp.MyApplication.getStringCode;

/*
public class SelectClientDialog extends Dialog {
    SelectClientListener listener;

    public SelectClientDialog(@NonNull Context context) { super(context); }
    public void setListener(SelectClientListener listener) { this.listener = listener; }
    @Override
    public void show() {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.select_clientcode_dialog);
        super.show();

        super.setOnDismissListener((dlgint) ->
        {
            this.dismiss();
            listener.OnDismiss();
        });
        //ListView clientcodeList = findViewById(R.id.ClientCodeList);
        TextView Title = findViewById(R.id.Title);
        TextView Infotext = findViewById(R.id.InformationText);
        Title.setText(getStringCode(824,"회사 선택"));
        Infotext.setText(getStringCode(823,"출입할 회사를 선택해 주세요!"));

        String[] strings = new String[companyList.length];
        for(int i=0;i<companyList.length;i++)
            strings[i] = companyList[i].CompanyName;


        /*
        final ArrayAdapter<String> adapter = new ArrayAdapter<>(getContext(),R.layout.clientdialog_content,strings);
        clientcodeList.setAdapter(adapter);
        clientcodeList.setOnItemClickListener((parent, view, position, id) -> {
            this.dismiss();
            listener.OnSelect(position);
        });
        SelectDialogListAdapter listAdapter = new SelectDialogListAdapter(getContext(),companyList);
        listAdapter.setListener((parent, view, position, id) -> {
            this.dismiss();
            listener.OnSelect(position);
        });
        clientcodeList.setAdapter(listAdapter);
        */
 /*

    }
}
*/
interface SelectClientListener
{
    void OnDismiss();
}
/*
class SelectDialogListAdapter extends BaseAdapter
{
    AdapterView.OnItemClickListener listener;
    Context mContext = null;
    LayoutInflater layoutInflater = null;
    MyApplication.CompanyList[] list;
    public SelectDialogListAdapter(Context context, MyApplication.CompanyList[] list)
    {
        mContext = context;
        this.list = list;
        layoutInflater = LayoutInflater.from(context);
    }
    public void setListener(AdapterView.OnItemClickListener listener) { this.listener = listener; }
    @Override
    public int getCount() { return list.length; }

    @Override
    public Object getItem(int position) { return list[position]; }

    @Override
    public long getItemId(int position) { return position; }

    TextView clientName;
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(convertView == null)
        {
            convertView = layoutInflater.inflate(R.layout.clientdialog_content,null);
            clientName = convertView.findViewById(R.id.clientName);
        }

        clientName.setText(list[position].CompanyName);

        //리스트 아이템을 터치 했을 때 이벤트 발생
        convertView.setOnClickListener(v->{
            if(listener != null)
                listener.onItemClick(null, null, position, getItemId(position));
            //Toast.makeText(this.mContext, "선택한 이름:" + list[position].CompanyName, Toast.LENGTH_SHORT).show();
        });
        return convertView;
    }
}
*/
public class SelectClientDialog extends Dialog {

    SelectClientListener listener;


    public Dialog dialog;
    TextView Title;
    TextView Infotext;
    Button closebutton;
    RecyclerView recyclerView;
    private RecyclerView.LayoutManager mLayoutManager;
    RecyclerView.Adapter adapter;


    public SelectClientDialog(Context a, RecyclerView.Adapter adapter) {
        super(a);
        this.adapter = adapter;
    }
    public void setListener(SelectClientListener listener) { this.listener = listener; }

    @Override
    public void show(){
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
        Title.setText(getStringCode(824,"회사 선택"));
        Infotext.setText(getStringCode(823,"출입할 회사를 선택해 주세요!"));
        closebutton.setOnClickListener(v->dismiss());

        String[] strings = new String[companyList.length];
        for(int i=0;i<companyList.length;i++)
            strings[i] = companyList[i].CompanyName;


        recyclerView = findViewById(R.id.recycler_view);
        mLayoutManager = new LinearLayoutManager(getContext());
        recyclerView.setLayoutManager(mLayoutManager);
        recyclerView.setAdapter(adapter);

        super.show();
    }
}

class DataAdapter extends RecyclerView.Adapter<DataAdapter.FruitViewHolder>  {
    private String[] mDataset;
    RecyclerViewItemClickListener recyclerViewItemClickListener;

    public DataAdapter(String[] myDataset, RecyclerViewItemClickListener listener) {
        mDataset = myDataset;
        this.recyclerViewItemClickListener = listener;
    }

    @NonNull
    @Override
    public FruitViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int i) {

        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.clientdialog_content, parent, false);

        FruitViewHolder vh = new FruitViewHolder(v);
        return vh;

    }

    @Override
    public void onBindViewHolder(@NonNull FruitViewHolder fruitViewHolder, int i) {
        fruitViewHolder.mTextView.setText(mDataset[i]);
    }

    @Override
    public int getItemCount() {
        return mDataset.length;
    }



    public  class FruitViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

        public TextView mTextView;

        public FruitViewHolder(View v) {
            super(v);
            mTextView = (TextView) v.findViewById(R.id.clientName);
            v.setOnClickListener(this);
        }

        @Override
        public void onClick(View v) {
            recyclerViewItemClickListener.clickOnItem(this.getAdapterPosition());

        }
    }

    public interface RecyclerViewItemClickListener {
        void clickOnItem(int position);
    }
}