package com.example.myfirstkotlin

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.ImageView
import android.widget.ListView
import android.widget.TextView
import androidx.fragment.app.Fragment

class FragmentAdminstat_1ap(var parent: AdminStatusActivity) : Fragment(){
    lateinit var list : ListView
    lateinit var adapter : APUsrListViewAdapter

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragmentadminstat_1ap,container,false)
        list = view.findViewById(R.id.APListView)

        list.refreshDrawableState()
        adapter = APUsrListViewAdapter(parent.arrayAP,parent)
        list.adapter = adapter
        return view
    }
    fun setColor()
    {

    }
}

class APUsrListViewAdapter(val list : Array<MyApplication.ItemTotalStatus>,val context : Context) : BaseAdapter(){
    override fun getCount(): Int {
        return list.size
    }

    override fun getItem(position: Int): MyApplication.ItemTotalStatus {
        return list[position]
    }

    override fun getItemId(position: Int): Long {
        return position.toLong()
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup?): View {
        lateinit var view:View
        if(convertView == null)
        {
            val inflater = LayoutInflater.from(context)
            view = inflater.inflate(R.layout.totstatus_itemusr, null)
        }else view = convertView
        val values = getItem(position)

        val name = view.findViewById<TextView>(R.id.Name)
        val lastdt = view.findViewById<TextView>(R.id.lastdt)
        val statimage = view.findViewById<ImageView>(R.id.flagImage)

        name.text = values.nm

        if(values.lastdt != null && values.lastdt!!.length < 5)
            values.lastdt = null
        lastdt.text = values.lastdt?.substring(5) ?: ""

        //blank(시작안함), 하루 지나면 d(ie), 휴가에 해당되면 v_휴가명, 아니면 a(live)
        if(values.st == "a")statimage.setImageResource(R.drawable.circle_green)//values.st = "Alive" // TODO : String Conversion
        else if(values.st == "")statimage.setImageResource(R.drawable.circle_null)//values.st = "start" // TODO : String Conversion
        else if(values.st?.get(0) == 'v')statimage.setImageResource(R.drawable.circle_orange)//
        else if(values.st == "d")statimage.setImageResource(R.drawable.circle_black)//values.st = "Died" // TODO : String Conversion

        return view
    }
}