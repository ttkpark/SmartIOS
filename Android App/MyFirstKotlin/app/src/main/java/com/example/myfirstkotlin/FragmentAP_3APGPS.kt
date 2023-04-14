package com.example.myfirstkotlin

import android.graphics.Color
import android.graphics.Color.parseColor
import android.os.Bundle
import android.util.Base64
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.fragment.app.Fragment
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.TextColor

class FragmentAP_3APGPS(var parent: APSettingActivity) : Fragment(){

    lateinit var viewApply : Button
    lateinit var viewAPSelect : Button

    lateinit var editLongitude : EditText
    lateinit var editLatitude : EditText

    lateinit var swipe : SwipeRefreshLayout

    lateinit var Data : MyApplication.APInfo
    var nowAPposition = 0

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragmentap_3gps,container,false)
        viewApply = view.findViewById(R.id.apply3)
        viewAPSelect = view.findViewById(R.id.APSelect)
        editLongitude = view.findViewById(R.id.editLongitude)
        editLatitude  = view.findViewById(R.id.editLatitude)

        viewApply.setOnClickListener { onclickApply() }

        swipe = view.findViewById(R.id.swipe_AP3)
        swipe.setOnRefreshListener { onFocus(); swipe.isRefreshing = false }

        onFocus()
        setColor()

        viewAPSelect.setOnClickListener { parent.callAPListActivity(2) }
//        viewAPSelect.onItemSelectedListener = object : OnItemSelectedListener {
//            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long){
//                if(nowAPposition != position) {
//                    nowAPposition = position
//                    try {
//                        val apinfo : MyApplication.APInfo = apInfoArray[position] ?: return
//
//                        Data = apinfo
//                        SelectedidxEquip = apinfo.idx ?: return
//
//                        onFocus()//refreshes this fragment.
//                    }catch (e:Throwable){e.printStackTrace()}
//                }
//
//            }
//
//            override fun onNothingSelected(parent: AdapterView<*>?){
//
//            }
//        }

        return view
    }
    fun setColor()
    {
        viewApply.setTextColor(parseColor(TextColor))
        viewApply.setBackgroundColor(parseColor(AppColor))
    }
    fun onFocus()
    {
        try{
            for(i in MyApplication.apInfoArray.indices) {
                if (MyApplication.apInfoArray[i].idx == MyApplication.SelectedidxEquip) {
                    Data = MyApplication.apInfoArray[i]
                    viewAPSelect.text = Data.apname ?: "Error" // TODO : String Conversion
                    val GPSs = Data.gps?.split(",") ?: return
                    if(GPSs.size != 2)return
                    editLongitude.setText(GPSs[0])
                    editLatitude.setText(GPSs[1])
                }
            }

        }catch(e:Exception){e.printStackTrace()}
    }

    fun onclickApply()
    {
        try {
            val gpsValue = editLongitude.text.toString() + "," + editLatitude.text.toString()

            parent.controller.apgps({ succeed, reply, replyArray ->
                if(!succeed)return@apgps

                val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
                if(rtncode == 1) {
                    parent.MessageAlert("apgps","설정 완료.") // TODO : String Conversion
                }else{
                    var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                    catch(e : Throwable) {
                        MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                    if(errorstr.toIntOrNull() != null)
                        errorstr = MyApplication.getStringCode(errorstr.toInt())

                    parent.MessageAlert("apgps",errorstr)
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                }
            }, MyApplication.idxUser.toString(),
                MyApplication.SelectedidxEquip.toString(),gpsValue)
        }catch (e:Throwable){e.printStackTrace()}
    }
}