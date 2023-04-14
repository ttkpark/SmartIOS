package com.example.myfirstkotlin

import android.content.Intent
import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import androidx.fragment.app.Fragment
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.getStringCodeHash
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.languageCode
import com.example.myfirstkotlin.MyApplication.Companion.saveCodeData
import org.json.JSONException

class Fragment_account(var parent: SettingsActivity) : Fragment(){
    lateinit var editName : EditText
    lateinit var editID : EditText
    lateinit var editPW : EditText
    lateinit var editPWCheck : EditText
    lateinit var editspinner : Spinner
    lateinit var btnApply : Button
    lateinit var btnlogout : Button

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater.inflate(R.layout.fragment_account,container,false)
        editName = view.findViewById(R.id.usrName)
        editID = view.findViewById(R.id.usr_ID)
        editPW = view.findViewById(R.id.usr_PW)
        editPWCheck = view.findViewById(R.id.usr_PWCheck)
        editspinner = view.findViewById(R.id.spinner)
        btnApply = view.findViewById(R.id.savebtn)
        btnlogout = view.findViewById(R.id.logout)

        setColor()
        SpinnerSetting()

        parent.controller.pers({ isSucceed, reply, replyArray ->
            if(!isSucceed)return@pers

            val rtncode = try{replyArray.getJSONObject(0).getString("rtncode").toInt()}catch(e:Throwable){0}
            if(rtncode == 1) {
                var name = "" ; var id = "" ; var language = ""

                try{
                    name = replyArray.getJSONObject(0).getString("name")
                    id = replyArray.getJSONObject(0).getString("id")
                    language = replyArray.getJSONObject(0).getString("langv")
                }catch(e:JSONException){
                    return@pers
                }

                for (i in languageCode.indices)
                    if(languageCode[i][0].replace("-","").lowercase() == language){
                        editspinner.setSelection(i)
                        break
                    }

                editID.setText(id)
                editName.setText(name)

            }else{
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {getStringCode(rtncode,null) ?: "Unknown Error."} // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                parent.MessageAlert("pers",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }

        },idxUser.toString())

        btnApply.setOnClickListener { savebtnClicked() }
        btnlogout.setOnClickListener { logout() }

        return view
    }
    fun setColor() {
//        editName.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
//        editName.setTextColor(Color.parseColor(MyApplication.TextColor))
//        editID.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
//        editID.setTextColor(Color.parseColor(MyApplication.TextColor))
//        editPW.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
//        editPW.setTextColor(Color.parseColor(MyApplication.TextColor))
//        editPWCheck.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
//        editPWCheck.setTextColor(Color.parseColor(MyApplication.TextColor))
//        editspinner.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        btnApply.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        btnApply.setTextColor(Color.parseColor(MyApplication.TextColor))
        btnlogout.setBackgroundColor(Color.parseColor(MyApplication.AppColor))
        btnlogout.setTextColor(Color.parseColor(MyApplication.TextColor))
    }

    fun logout()
    {
        getStringCodeHash.remove("9988")
        saveCodeData()
        val intent = Intent(parent,LoginActivity::class.java)
        intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TASK
        startActivity(intent)

        parent.finish()
    }

    fun SpinnerSetting() {
        val TypeList = arrayOfNulls<String>(languageCode.size)
        for (i in TypeList.indices)
            TypeList[i] = languageCode[i][1]

        val adapter = ArrayAdapter<String>(parent, R.layout.spinner_item, TypeList)
        adapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item)
        editspinner.adapter = adapter
    }

    fun savebtnClicked()
    {
        val ID = editID.text.toString()
        val PW1 = editPW.text.toString()
        val PW2 = editPWCheck.text.toString()
        val Name = editName.text.toString()
        val lang = editspinner.selectedItemId
        if(ID.length < 2 || !PW1.equals(PW2) || Name.length < 2 || lang >= languageCode.size)
        {
            parent.MessageAlert("Account","please Check the text again.") // TODO : String Conversion
            return
        }

        parent.controller.cpers({isSucceed, reply, replyArray ->
            if(!isSucceed) {
                parent.MessageAlert("Account", "Save Failed.") // TODO : String Conversion
                return@cpers
            }
            val rtncode = try{ replyArray.getJSONObject(0).getString("rtncode").toInt() } catch(e:Throwable){0}
            if(rtncode != 1) {
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {getStringCode(rtncode,null) ?: "Unknown Error."} // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                parent.MessageAlert("Account",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                return@cpers
            }else{
                parent.MessageAlert("Account", "Save Succeed.") // TODO : String Conversion
            }
        },idxUser.toString(),ID,Name,PW1)

    }
}