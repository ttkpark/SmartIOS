package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.app.DatePickerDialog
import android.graphics.Color
import android.os.Bundle
import android.text.InputFilter
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.fragment.app.Fragment
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import com.example.myfirstkotlin.MyApplication.Companion.gatepositionArray
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import org.json.JSONException
import org.json.JSONObject
import java.util.*
import kotlin.collections.ArrayList

class Fragment3_list(var parent: MainActivity) : Fragment(){
    lateinit var editclientcode : Button
    lateinit var editDate : EditText
    lateinit var list : ListView
    lateinit var btnload : Button
    lateinit var listAdapter : AttendListAdapter
    lateinit var swipe : SwipeRefreshLayout

    var setYear = 0
    var setMonth = 0
    var setDay = 0
    var DateString = ""
    var selectedClientcode = 0
    var selectedClientmenu = 0
    var alreadyFocused = false

    var clientInfoArray = arrayOfNulls<MyApplication.ClientInfo>(0)


    fun fCInfo(name : String,idxc : Int) : MyApplication.ClientInfo
    {
        val a = MyApplication.ClientInfo()
        a.clientName = name
        a.clientCode = idxc.toString()
        return a
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragment3_list,container,false)
        editclientcode = view.findViewById(R.id.clientspinner)
        editDate = view.findViewById(R.id.editDate)
        list = view.findViewById(R.id.attendlist)
        btnload = view.findViewById(R.id.button)
        swipe = view.findViewById(R.id.swipe_attendlist)

        editclientcode.filters += InputFilter.LengthFilter(6)

        //clientinfoArray = arrayOf(fCInfo("(주)아주소프트",1))

        editDate.setOnClickListener { editDateClick() }
        btnload.setOnClickListener { RequestListing() }
        swipe.setOnRefreshListener { RequestListing(); swipe.isRefreshing = false }

        if(!alreadyFocused)
            isFocused()

        return view
    }

    fun isFocused()
    {
        setColor()
        alreadyFocused = true
        startTask()
    }
    fun isUnFocused()
    {

    }
    fun setColor()
    {
        editclientcode.setTextColor(Color.parseColor(MyApplication.AppColor))
        btnload.setTextColor(Color.parseColor(MyApplication.TextColor))
        btnload.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
    }

    fun startTask()
    {
        //다이얼로그 띄우기로.
        editclientcode.setOnClickListener{ view: View ->
            onClientCodeTextClick_Dialog { position ->
                if(selectedClientmenu == position)return@onClientCodeTextClick_Dialog


                //아이템이 클릭 되면 맨 위부터 position 0번부터 순서대로 동작하게 됩니다.
                selectedClientmenu = position

                if(position != 0){
                    selectedClientcode = clientInfoArray[selectedClientmenu-1]?.clientCode?.toInt() ?: 0
                    editclientcode.text = clientInfoArray[selectedClientmenu-1]?.clientName ?: "Error" // TODO : String Conversion
                }else {
                    editclientcode.text = "All" // TODO : String Conversion
                    selectedClientcode = 0
                }
                RequestListing()
            }
        }

        val date = Calendar.getInstance()
        setDateAndDateString(date.get(Calendar.YEAR),date.get(Calendar.MONTH) + 1,date.get(Calendar.DAY_OF_MONTH))
        editDate.setText(DateString)
        RequestListing()
    }

    fun onClientCodeTextClick_Dialog(callback : (position:Int)->Unit)
    {
        parent.onClientCodeTextClick_Dialog(
            Array(clientInfoArray.size+1) { i -> if(i==0)"All" else clientInfoArray[i-1]?.clientName ?: "Error" },
            getStringCode(824, "회사 선택"), // TODO : String Conversion
            getStringCode(823, "출입할 회사를 선택해 주세요!"), // TODO : String Conversion
            { dlg,pos-> dlg.dismiss();callback.invoke(pos) },
            null)
    }
    fun RequestListing()
    {
        if(selectedClientcode == 0)editclientcode.text = "All" // TODO : String Conversion

        parent.controller.iodtlist({isSucceed, reply, replyArray ->
            if(!isSucceed)return@iodtlist

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){1}
            if(rtncode == 1) {

                //JSON 데이터 해석하고 그 결과구조체를 gatepositionArray에 넣기
                val array : MutableList<MyApplication.SimpleGatePosition?> = ArrayList()
                val NewClientsArray : MutableList<MyApplication.ClientInfo> = ArrayList()

                var rtncodeStr = ""

                for(i in 0 until replyArray.length())
                {
                    val obj: JSONObject = replyArray.getJSONObject(i)

                    //, "time" : "08:55:30", "cc" : "1000", "ccname" : "회사명", "gatename" : "출입문"
                    // "rtncode":"01","time":"22:37:23","idxc":1,"ccname":"(주)아주소프트","gatename":"출입문2"
                    try{
                        rtncodeStr = obj.getString("rtncode")
                        if(rtncodeStr != "comlist") {
                            val input = MyApplication.SimpleGatePosition()
                            input.clientcode = obj.getString("idxc")
                            input.time = obj.getString("tm")
                            input.gatename = obj.getString("gn")
                            array.add(input)
                        }
                        else {
                            val client = MyApplication.ClientInfo()
                            client.clientCode = obj.getString("idxc")
                            client.clientName = obj.getString("cn")
                            NewClientsArray.add(client)
                        }
                    }catch(e: JSONException){}
                }
                gatepositionArray = array.toTypedArray()
                if(NewClientsArray.size != 0) {
                    clientInfoArray = NewClientsArray.toTypedArray()
                    //editclientcode.text = "All"
                    //editclientcode.setSelection(selectedClientmenu)
                }

                //입출입 리스트를 업데이트하기

            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {
                    getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                }
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

//                //if NO i/o data -> clear the ListView
//                if(rtncode == 15) {
//                    clientinfoArray = emptyArray()
//                    spinnerInit()
//                    listAdapter.list = arrayOfNulls(0)
//                    listAdapter.notifyDataSetChanged()
//                }

                val errorRow = MyApplication.SimpleGatePosition()
                errorRow.time = errorstr
                errorRow.gatename = null
                gatepositionArray = arrayOf(errorRow)

                //parent.MessageAlert("admin",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
            UpdateAttendlist()
        },idxUser.toString(),DateString,if(selectedClientcode!=0)selectedClientcode.toString() else "")
    }

    fun setDateAndDateString(Y: Int, M: Int, d: Int) {
        setYear = Y
        setMonth = M
        setDay = d
        DateString = String.format("%04d%02d%02d", Y, M, d)
    }
    fun editDateClick() {
        val dlg = DatePickerDialog(parent,{ view, year, month, dayOfMonth ->
                setDateAndDateString(year, month + 1, dayOfMonth)
                editDate.setText(DateString)
                RequestListing()
            }, setYear, setMonth - 1, setDay)
        dlg.show()
    }

    fun UpdateAttendlist()
    {
        val len = (clientInfoArray?.size) ?: 1
        val strarray = arrayOfNulls<String>(len+1)
        strarray[0] = "All" // TODO : String Conversion
        for(i in 1..len)
            strarray[i] = clientInfoArray?.get(i-1)?.clientName ?: "Error" // TODO : String Conversion

        if(gatepositionArray == null)return

        list.refreshDrawableState()
        listAdapter = AttendListAdapter(this, gatepositionArray!!)
        list.adapter = listAdapter

//        //val listAdapter = ArrayAdapter<String>(parent,R.layout.spinner_item,strarray)//
//        listAdapter.list = gatepositionArray as Array<MyApplication.SimpleGatePosition?>
//        listAdapter.notifyDataSetChanged()
//        list.removeAllViews()
//        list.refreshDrawableState()

    }
}

class AttendListAdapter(var parent: Fragment3_list, var list: Array<MyApplication.SimpleGatePosition?>) : BaseAdapter() {
    override fun getCount(): Int {
        return list.size
    }

    override fun getItem(position: Int): Any? {
        return list[position]
    }

    override fun getItemId(position: Int): Long {
        return position.toLong()
    }

    @SuppressLint("ViewHolder")
    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        var view = convertView

        // "listview_item" Layout을 inflate하여 convertView 참조 획득.
        if (view == null) {
            val inflater = LayoutInflater.from(this.parent.parent)//this.parent.parent.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
            view = inflater.inflate(
                if(this.parent.selectedClientcode == 0)
                    R.layout.attend_list_item_clientcode
                else
                    R.layout.attend_list_item
                , null)
        }

        val time = view?.findViewById<TextView>(R.id.attend_list_Time)
        val doorname_Clientcode = view?.findViewById<TextView>(R.id.attend_list_door)

        time?.text = list[position]?.time

        if(list[position]?.gatename == null) {
            val timeIcon = view?.findViewById<ImageView>(R.id.poster)
            timeIcon?.visibility = View.GONE
            doorname_Clientcode?.visibility = View.GONE
        }else{
            if (this.parent.selectedClientcode != 0)//show gatename
                doorname_Clientcode?.text = list[position]?.gatename ?: "error" // TODO : String Conversion
            else {
                for (i in this.parent.clientInfoArray)//show clientName
                    if (i?.clientCode == list[position]?.clientcode)
                        doorname_Clientcode?.text = i?.clientName ?: "error" // TODO : String Conversion
            }
        }

        return view!!
    }
}