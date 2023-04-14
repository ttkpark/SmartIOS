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

class FragmentAdminstat_2user(var parent: AdminStatusActivity) : Fragment(){
    lateinit var list : ListView
    lateinit var adapter : APUsrListViewAdapter

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragmentadminstat_1ap,container,false)
        list = view.findViewById(R.id.APListView)

        list.refreshDrawableState()
        adapter = APUsrListViewAdapter(parent.arrayUsr,parent)
        list.adapter = adapter
        return view
    }
}