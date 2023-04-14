package com.example.myfirstkotlin

import android.graphics.Color
import android.graphics.Color.parseColor
import android.util.Base64
import android.os.Bundle
import android.util.MutableByte
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.fragment.app.Fragment
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import com.example.myfirstkotlin.MyApplication.Companion.AppColor
import com.example.myfirstkotlin.MyApplication.Companion.Bytearray2IP
import com.example.myfirstkotlin.MyApplication.Companion.IP2Bytearray
import com.example.myfirstkotlin.MyApplication.Companion.SelectedidxEquip
import com.example.myfirstkotlin.MyApplication.Companion.TextColor
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import java.lang.String.format
import java.util.*

class FragmentAP_2APIP(var parent: APSettingActivity) : Fragment(){
    lateinit var viewApply : Button
    lateinit var viewAPSelect : Button

    lateinit var editAPIP : EditText
    lateinit var editMASK : EditText
    lateinit var editGWIP : EditText
    lateinit var editDNS1 : EditText
    lateinit var editDNS2 : EditText
    lateinit var swipe : SwipeRefreshLayout

    lateinit var Data : MyApplication.APInfo
    var nowAPposition = 0

    lateinit var ipByteArray : ByteArray


    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragmentap_2ip,container,false)
        viewApply = view.findViewById(R.id.apply2)
        viewAPSelect = view.findViewById(R.id.APSelect)
        editAPIP = view.findViewById(R.id.editText3)
        editMASK = view.findViewById(R.id.editText5)
        editGWIP = view.findViewById(R.id.editText6)
        editDNS1 = view.findViewById(R.id.editText7)
        editDNS2 = view.findViewById(R.id.editText8)

        swipe = view.findViewById(R.id.swipe_AP2)
        swipe.setOnRefreshListener { onFocus(); swipe.isRefreshing = false }

        viewApply.setOnClickListener { onclickApply() }

        onFocus()
        setColor()

        viewAPSelect.setOnClickListener { parent.callAPListActivity(1) }
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
                    ipByteArray = Base64.decode(Data.ip,0)

                    editAPIP.setText(Bytearray2IP(ipByteArray.copyOfRange(0,4)))
                    editGWIP.setText(Bytearray2IP(ipByteArray.copyOfRange(4,8)))
                    editMASK.setText(Bytearray2IP(ipByteArray.copyOfRange(8,12)))
                    editDNS1.setText(Bytearray2IP(ipByteArray.copyOfRange(12,16)))
                    editDNS2.setText(Bytearray2IP(ipByteArray.copyOfRange(16,20)))
                }
            }

        }catch(e:Exception){e.printStackTrace()}
    }

    fun onclickApply()
    {
        try {

            val APIP = IP2Bytearray(if(editAPIP.text.length != 0) editAPIP.text.toString() else "0.0.0.0")
            val GWIP = IP2Bytearray(if(editGWIP.text.length != 0) editGWIP.text.toString() else "0.0.0.0")
            val MASK = IP2Bytearray(if(editMASK.text.length != 0) editMASK.text.toString() else "0.0.0.0")
            val DNS1 = IP2Bytearray(if(editDNS1.text.length != 0) editDNS1.text.toString() else "0.0.0.0")
            val DNS2 = IP2Bytearray(if(editDNS2.text.length != 0) editDNS2.text.toString() else "0.0.0.0")

            val byteArray = APIP + GWIP + MASK + DNS1 + DNS2
            val convertedIP = String(Base64.encode(byteArray,0))

            parent.controller.apipc({ succeed, reply, replyArray ->
                if(!succeed)return@apipc

                val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
                if(rtncode == 1) {
                    parent.MessageAlert("apipc","설정 완료.") // TODO : String Conversion
                }else{
                    var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                    catch(e : Throwable) {
                        MyApplication.getStringCode(rtncode, null) ?: "Unknown Error."} // TODO : String Conversion
                    if(errorstr.toIntOrNull() != null)
                        errorstr = MyApplication.getStringCode(errorstr.toInt())

                    parent.MessageAlert("apipc",errorstr)
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                }
            }, idxUser.toString(),SelectedidxEquip.toString(),convertedIP)
        }catch (e:Throwable){e.printStackTrace()}
    }
}