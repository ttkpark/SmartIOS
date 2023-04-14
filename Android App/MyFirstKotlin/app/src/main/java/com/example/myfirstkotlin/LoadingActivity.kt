package com.example.myfirstkotlin

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.util.Log
import androidx.appcompat.app.AlertDialog
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.reloadCodeData
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject

class LoadingActivity : AppCompatActivity() {
    lateinit var myController : MyController

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_loading)

        myController = MyController(this, Looper.getMainLooper())

        //자동로그인을 시도한다.
        reloadCodeData()
        idxUser = try { getStringCode(9988).toInt() } catch(e:Throwable){0}
        if(idxUser != 0)
        {
            myController.loginhp({ succeed, reply, replyArray->
                if(!succeed)return@loginhp
                var rtncode: Int
                try {
                    rtncode = Integer.parseInt(replyArray.getJSONObject(0).getString("rtncode") ?: "0")
                }catch(e: JSONException){ rtncode = 0 }

                if(rtncode != 1)
                {
                    var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                    catch(e : Throwable) {""}
                    if(errorstr.isEmpty())errorstr = getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion

                    //실패. LoginActivity로 이동한다.
                    MessageAlert("loginhp",errorstr)
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                    goLoginActivity()
                }
                else
                {
                    // TODO: Does we Pass away about utype? : Yes
                    var utype = try {replyArray.getJSONObject(0).getString("rtncode")}catch(e: JSONException){ null }

                    callLanguageCode()
                }
            }, idxUser.toString())
        }
        else{
            //자동로그인 할 수 없으면 LoginActivity로 이동한다.
            goLoginActivity()
        }
    }

    fun callLanguageCode()
    {
        myController.langv({ isSucceed, reply, replyArray -> MyApplication.isAdmin = false; if(isSucceed) {
            val rtncode = try { replyArray.getJSONObject(0).getString("rtncode").toInt() } catch (e: Throwable) { 1 }
            if (rtncode == 1) {
                processLanguageCode(replyArray)
                MyApplication.saveCodeData()
                callAdminSetting()

            } else {
                var errorstr = try {
                    replyArray.getJSONObject(0).getString("msg")
                } catch (e: Throwable) {
                    getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                }
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                //MessageAlert("langv", errorstr)

                MyApplication.InitHashes()
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                //Languagecode를 호출할 때는 에러가 나도 계속한다.
                callAdminSetting()
            }
        }}, idxUser.toString())
        idxUser = try { getStringCode(9988).toInt() } catch(e:Throwable){0}
    }


    fun callAdminSetting()
    {
        myController.adm({ isSucceed, reply, replyArray -> MyApplication.isAdmin = false
            if(isSucceed) {
                val rtncode = try { replyArray.getJSONObject(0).getString("rtncode").toInt() } catch (e: Throwable) { 1 }
                if (rtncode == 31) {//성공 or 권한이 없습니다
                    MyApplication.clientInfoArray = emptyArray()
                    MyApplication.isAdmin = false
                }
                else if (rtncode == 1) {//성공 or 권한이 없습니다
                    val count = replyArray.length()
                    val array: Array<MyApplication.ClientInfo?> = arrayOfNulls(count)
                    for (i in array.indices) {
                        val obj: JSONObject = replyArray.getJSONObject(i)
                        array[i] = MyApplication.ClientInfo()
                        try {//"cc" : "1000", "cn" : "client명1"
                            array[i]?.clientCode = obj.getString("cc")
                            array[i]?.clientName = obj.getString("cn")
                        } catch (e: JSONException) {
                            e.printStackTrace()
                        }
                    }
                    MyApplication.clientInfoArray = array

                    MyApplication.isAdmin = true
                } else {
                    var errorstr = try {
                        replyArray.getJSONObject(0).getString("msg")
                    } catch (e: Throwable) {
                        getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                    }
                    if(errorstr.toIntOrNull() != null)
                        errorstr = getStringCode(errorstr.toInt())

                    MessageAlert("admin", errorstr)
                    Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
                    goLoginActivity()
                }
            }
            goMainActivity()

        }, idxUser.toString())
    }
    fun processLanguageCode(replyArray : JSONArray)
    {
        for (i in 0 until replyArray.length()) {
            val obj: JSONObject = replyArray.getJSONObject(i)
            MyApplication.getStringCodeHash.put(
                obj.getString("code").toInt().toString(),
                obj.getString("val")
            )
            Log.i("Language",obj.toString())
        }
    }
    fun goMainActivity() {
        startActivity(Intent(this, MainActivity::class.java))
        super.finish()
    }
    fun goLoginActivity() {
        startActivity(Intent(this, LoginActivity::class.java))
        super.finish()
    }

    fun MessageAlert(Title: String?, Messages: String?) {
        AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
            .setPositiveButton("OK") {_,_->}.create().show()
    } // TODO : String Conversion

}