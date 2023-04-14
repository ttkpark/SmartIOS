package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Color
import android.graphics.Typeface
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.ImageView
import android.widget.TextView


class APListAdapter(context: Context?, data: Array<MyApplication.APInfo>) :
    BaseAdapter() {
    var mContext = context
    var layoutInflater = LayoutInflater.from(context)
    var list: Array<MyApplication.APInfo> = data
    override fun getCount(): Int {
        return list.size
    }

    override fun getItem(position: Int): Any? {
        return list[position]
    }

    override fun getItemId(position: Int): Long {
        return position.toLong()
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        @SuppressLint("ViewHolder", "InflateParams")
        var view = convertView

        // "listview_item" Layout을 inflate하여 convertView 참조 획득.
        if (view == null) {
            layoutInflater = LayoutInflater.from(mContext)
            view = layoutInflater!!.inflate(R.layout.ap_list_item, null)
        }
        val editName = view?.findViewById<TextView>(R.id.Text1)
        val distance = view?.findViewById<TextView>(R.id.Distance)
        val flagImage = view?.findViewById<ImageView>(R.id.flagImage)
        val USEImage = view?.findViewById<ImageView>(R.id.imageView)
        var Name = list[position].apname ?: ""


        if (list[position].dist == null)
        {
            //error printing mode

            editName?.text = list[position].apname

            distance!!.visibility = View.GONE
            flagImage!!.visibility = View.GONE
            USEImage!!.visibility = View.GONE
        }
        else {
            //tidy the string
            val limit = 20
            if (Name.length > limit) Name = Name.substring(0, limit) + ".."
            editName?.text = Name

            if ("Y" != list[position].useyn) //Notuse일 때 회색/ 이미지 변경(X로)
            {
                //time.setPaintFlags(time.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
                editName?.setTypeface(Typeface.DEFAULT_BOLD)
                editName?.setTextColor(Color.parseColor("#7f7f7f"))

                //distance.setPaintFlags(time.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
                distance?.setTypeface(Typeface.DEFAULT_BOLD)
                distance?.setTextColor(Color.parseColor("#7f7f7f"))
                USEImage?.setImageResource(R.drawable.api_list_item_notuse)
            }
            distance?.text = java.lang.String.format("%sm", list[position].dist)
            val flag = list[position].flg?.toInt() ?: 3 //if no data
            var flagid = 0
            if (!(flag > 3 || flag < 0)) {
                val flagarray = intArrayOf(
                    R.drawable.ap_list_item_flag0,  // 0 : F(dark) 실패 (AP -> 서버.API로 보낸 IP가 정상 수정 실패하여 기존IP사용)
                    R.drawable.ap_list_item_flag1,  // 1 : W(yellow) 대기중(스마트폰.관리자 수정)
                    R.drawable.ap_list_item_flag2,  // 2 : R(red) 신청중(AP가 가져감)
                    R.drawable.ap_list_item_flag3   // 3 : D(green) 완료 (AP -> 서버.API로 보낸 IP가 정상 수정됨)
                )
                flagid = flagarray[flag]
            }
            if (flagid != 0) flagImage?.setImageResource(flagid)
        }
        return view!!
    }
}
