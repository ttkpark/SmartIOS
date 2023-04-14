package com.example.myfirstkotlin

import android.graphics.Color
import android.graphics.Color.parseColor
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import android.widget.AdapterView.OnItemSelectedListener
import androidx.fragment.app.Fragment
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.SelectedidxEquip
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.apInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.makePartialArray
import com.example.myfirstkotlin.MyApplication.Companion.sensorType
import com.example.myfirstkotlin.MyApplication.Companion.timeZone
import java.util.*

class FragmentAP_1APdetail(var parent: APSettingActivity) : Fragment(){
    lateinit var viewApply : Button
    lateinit var viewAPSelect : Button

    lateinit var viewSensortype : Spinner
    lateinit var viewUseAutoDoor : CheckBox
    lateinit var viewDistance : EditText
    lateinit var viewTimeZone : Spinner
    lateinit var viewAPUsing : Button
    lateinit var viewAPDelete : Button
    lateinit var swipe : SwipeRefreshLayout

    lateinit var Data : MyApplication.APInfo
    var nowAPposition = 0

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragmentap_1detail,container,false)
        viewApply = view.findViewById(R.id.apply1)
        viewAPSelect = view.findViewById(R.id.APSelect)
        viewSensortype = view.findViewById(R.id.sensor_type)
        viewUseAutoDoor = view.findViewById(R.id.checkBox2)
        viewDistance = view.findViewById(R.id.editText3)
        viewTimeZone = view.findViewById(R.id.sensor_timezone)
        viewAPUsing = view.findViewById(R.id.button2)
        viewAPDelete = view.findViewById(R.id.button3)

        swipe = view.findViewById(R.id.swipe_AP1)
        swipe.setOnRefreshListener { onFocus(); swipe.isRefreshing = false }

        viewApply.setOnClickListener { onclickApply() }
        viewAPUsing.setOnClickListener { onclickAPUsing() }
        viewAPDelete.setOnClickListener { onclickAPDelete() }

        onFocus()
        preDataSet()
        setColor()

        viewAPSelect.setOnClickListener { parent.callAPListActivity(0) }
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
        viewAPUsing.setTextColor(parseColor(TextColor))
        viewAPUsing.setBackgroundColor(parseColor(AppColor))
        viewAPDelete.setTextColor(parseColor(TextColor))
        viewAPDelete.setBackgroundColor(parseColor(AppColor))
        viewApply.setTextColor(parseColor(TextColor))
        viewApply.setBackgroundColor(parseColor(AppColor))
    }
    fun onFocus()
    {
        try{
            //viewAPSelect.adapter = ArrayAdapter(parent,R.layout.spinner_item, Array(apInfoArray.size) { i -> apInfoArray[i].apname ?: "Error" })
            for(i in apInfoArray.indices) {
                if (apInfoArray[i].idx == SelectedidxEquip) {
                    Data = apInfoArray[i]
                    viewAPSelect.text = Data.apname ?: "Error" // TODO : String Conversion
                    //viewAPSelect.setSelection(i)
                }
            }
            viewUseAutoDoor.isChecked = Data.openlock == 1//Data.useio == "Y"
            viewDistance.setText(Data.dist)
            viewAPUsing.text = if(Data.useyn == "Y")"사용 안함" else "사용" // TODO : String Conversion

            for(i in timeZone.indices) {
                if (timeZone[i][0] == Data.tz.toString()) {
                    viewTimeZone.setSelection(i)
                }
            }
            for(i in sensorType.indices) {
                if (sensorType[i][0].lowercase() == Data.sty?.lowercase()) {
                    viewSensortype.setSelection(i)
                }
            }
        }catch(e:Exception){e.printStackTrace()}
    }
    fun preDataSet()
    {
        viewTimeZone.adapter = ArrayAdapter(parent,R.layout.spinner_item, makePartialArray(timeZone,1))
        viewSensortype.adapter = ArrayAdapter(parent,R.layout.spinner_item, makePartialArray(sensorType,1))
        viewAPDelete.text = "삭제" // TODO : String Conversion
    }

    fun onclickApply()
    {
        //apply API
        val distance = viewDistance.text.toString()
        val opendoorlock = if(viewUseAutoDoor.isChecked)1 else 0
        val sensortype :String?
        val timezone: String?

        try {sensortype = sensorType[viewSensortype.selectedItemPosition][0] }catch (e:ArrayIndexOutOfBoundsException){e.printStackTrace();return}
        try {timezone = timeZone[viewTimeZone.selectedItemPosition][0] }catch (e:ArrayIndexOutOfBoundsException){e.printStackTrace();return}

        parent.controller.apin({ succeed, reply, replyArray ->
            if(!succeed)return@apin

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
            if(rtncode == 1) {
                parent.MessageAlert("apin","설정 완료.") // TODO : String Conversion
            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {
                    MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = MyApplication.getStringCode(errorstr.toInt())

                parent.MessageAlert("apin",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
        }, idxUser.toString(), SelectedidxEquip.toString(),distance,sensortype,opendoorlock.toString(),timezone)
    }

    fun onclickAPUsing()
    {
        if(Data.useyn == "Y")
        {
            //nonuse
            parent.controller.apnotuse({ succeed, reply, replyArray ->
                if(!succeed)return@apnotuse

                val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
                if(rtncode == 1) {
                    parent.MessageAlert("apnotuse","설정 완료.") // TODO : String Conversion
                    parent.finish()
                }else{
                    var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                    catch(e : Throwable) {
                        MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                    if(errorstr.toIntOrNull() != null)
                        errorstr = MyApplication.getStringCode(errorstr.toInt())

                    parent.MessageAlert("apnotuse",errorstr) // TODO : String Conversion
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                }
            },idxUser.toString(),SelectedidxEquip.toString())
        }
        else
        {
            //use
            parent.controller.apuse({ succeed, reply, replyArray ->
                if(!succeed)return@apuse

                val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
                if(rtncode == 1) {
                    parent.MessageAlert("apuse","설정 완료.") // TODO : String Conversion
                    parent.finish()
                }else{
                    var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                    catch(e : Throwable) {
                        MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                    if(errorstr.toIntOrNull() != null)
                        errorstr = MyApplication.getStringCode(errorstr.toInt())

                    parent.MessageAlert("apuse",errorstr)
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                }
            },idxUser.toString(),SelectedidxEquip.toString())
        }
    }

    fun onclickAPDelete()
    {
        //delete
        parent.controller.apout({ succeed, reply, replyArray ->
            if(!succeed)return@apout

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
            if(rtncode == 1) {
                parent.MessageAlert("apout","삭제 완료.") // TODO : String Conversion
                parent.finish()
            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {
                    MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = MyApplication.getStringCode(errorstr.toInt())

                parent.MessageAlert("apout",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
        },idxUser.toString(),SelectedidxEquip.toString())
    }

}