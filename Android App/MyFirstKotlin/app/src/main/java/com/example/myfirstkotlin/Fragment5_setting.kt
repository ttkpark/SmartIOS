package com.example.myfirstkotlin

import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.graphics.Typeface
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.fragment.app.Fragment
import com.example.myfirstkotlin.MyApplication.Companion.InitHashes
import com.example.myfirstkotlin.MyApplication.Companion.apInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.clientInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.clientcode
import com.example.myfirstkotlin.MyApplication.Companion.getClientName
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.isAdmin
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.lang.String.format

class Fragment5_setting(var parent: MainActivity) : Fragment(){
    lateinit var ReloadBtn: Button
    lateinit var Menuitem_1: MmenuIconContent
    lateinit var Menuitem_2: MmenuIconContent
    lateinit var Menuitem_3: MmenuIconContent
    lateinit var Menuitem_4: MmenuIconContent
    lateinit var clientcodeBtn: Button
    lateinit var arrayAdapter : APListAdapter
    var alreadyFocused = false
    var clientname = getClientName(clientcode.toString())

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragment5_setting,container,false)

        Menuitem_1 = view.findViewById(R.id.adminMenu_1)
        Menuitem_2 = view.findViewById(R.id.adminMenu_2)
        Menuitem_3 = view.findViewById(R.id.adminMenu_3)
        Menuitem_4 = view.findViewById(R.id.adminMenu_4)

        Menuitem_1.setOnClickListener { callAPSettingListActivity() }
        Menuitem_2.setOnClickListener { callManageSiteActivity() }
        Menuitem_3.setOnClickListener { callAdminStatusActivity() }
        Menuitem_4.setOnClickListener {  }
        Menuitem_4.visibility = View.GONE

        clientcodeBtn = view.findViewById(R.id.clientCode)
        //clientcodeBtn.setTitle(clientname ?: "")
        setColor()

        clientcodeBtn.setOnClickListener {
            parent.controller.adm({ isSucceed, _, replyArray ->
                if(isSucceed) {
                val rtnCode = try { replyArray.getJSONObject(0).getString("rtncode").toInt() } catch (e: Throwable) { 1 }
                if (rtnCode == 1) {
                    val count = replyArray.length()
                    clientInfoArray = Array(count){
                            val jsonObject: JSONObject = replyArray.getJSONObject(it)
                            val obj = MyApplication.ClientInfo()
                            try {//"cc" : "1000", "cn" : "client명1"
                                obj.clientCode = jsonObject.getString("cc")
                                obj.clientName = jsonObject.getString("cn")
                            } catch (e: JSONException) {
                                e.printStackTrace()
                            }

                            obj//를 lambda에서 리턴하여 각 항의 내용이 된다.
                        }

                    isAdmin = true
                }else{
                    //isAdmin = false
                    //parent.bottomNavigationView.inflateMenu(R.menu.menu_main_bottomnav)
                }
            }
                parent.onClientCodeTextClick_Dialog(
                    Array(clientInfoArray!!.size) { i ->clientInfoArray!![i]?.clientName ?: "Error" }, // TODO : String Conversion
                    getStringCode(824,"회사 선택"),
                    getStringCode(823, "출입할 회사를 선택해 주세요!"),
                    { dlg,pos ->
                        parent.setclientCode(pos)
                        dlg.dismiss()
                        parent.callLanguageCode{
                            isFocused()
                            InitHashes()
                        }
                    },
                    null)
            }, MyApplication.idxUser.toString())
        }

        if(parent.bottomNavigationView.selectedItemId == R.id.settingItem && !alreadyFocused)
            isFocused()
        return view
    }
    fun setColor()
    {
        Menuitem_1.setTextColor(Color.parseColor(MyApplication.TextColor))
        Menuitem_1.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        Menuitem_2.setTextColor(Color.parseColor(MyApplication.TextColor))
        Menuitem_2.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        Menuitem_3.setTextColor(Color.parseColor(MyApplication.TextColor))
        Menuitem_3.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        Menuitem_4.setTextColor(Color.parseColor(MyApplication.TextColor))
        Menuitem_4.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        clientcodeBtn.setTextColor(Color.parseColor(MyApplication.TextColor))
        clientcodeBtn.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
    }



    fun isFocused()
    {
        Focused{ startTask() }
    }
    fun Focused(callback : ()->Unit)
    {
        alreadyFocused = true
        if(clientname.isNullOrBlank())
        {
            parent.onClientCodeTextClick_Dialog(
                Array(clientInfoArray!!.size) { i ->clientInfoArray!![i]?.clientName ?: "Error" }, // TODO : String Conversion
                getStringCode(824,"회사 선택"),
                getStringCode(823,"출입할 회사를 선택해 주세요!"),
                { dlg,pos ->
                    parent.setclientCode(pos)
                    clientname = getClientName(clientcode.toString())
                    //clientcodeBtn.setTitle(clientname ?: "")
                    dlg.dismiss()
                    callback.invoke()
                },
                null)
        }
        else callback.invoke()
    }
    fun isUnFocused()
    {

    }

    fun startTask()
    {

    }
    fun callAPSettingListActivity()
    {
        val intent = Intent(parent,APSettingListActivity::class.java)
        if(clientcode == null)
            Focused { startActivity(intent) }
        else startActivity(intent)
    }
    fun callManageSiteActivity()
    {
        val intent = Intent(parent,ManageSiteActivity::class.java)
        if(clientcode == null)
            Focused { startActivity(intent) }
        else startActivity(intent)
    }
    fun callAdminStatusActivity()
    {
        val intent = Intent(parent,AdminStatusActivity::class.java)
        if(clientcode == null)
            Focused { startActivity(intent) }
        else startActivity(intent)
    }

}