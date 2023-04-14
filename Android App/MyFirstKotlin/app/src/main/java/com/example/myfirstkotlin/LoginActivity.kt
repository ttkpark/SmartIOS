package com.example.myfirstkotlin


import android.content.Intent
import android.os.Bundle
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.EditText
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.example.myfirstkotlin.MyApplication.Companion.clientInfoArray
import com.example.myfirstkotlin.MyApplication.Companion.getStringCode
import com.example.myfirstkotlin.MyApplication.Companion.getStringCodeHash
import com.example.myfirstkotlin.MyApplication.Companion.idxUser
import com.example.myfirstkotlin.MyApplication.Companion.isAdmin
import com.example.myfirstkotlin.MyApplication.Companion.saveCodeData
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject

class LoginActivity : AppCompatActivity() {
    lateinit var myController : MyController
    lateinit var btnLogin : Button
    lateinit var btnAccount : Button
    lateinit var editID : EditText
    lateinit var editPW : EditText

    var newPW:String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.login_activity)

        //변수 초기화
        myController = MyController(this, Looper.getMainLooper())
        btnLogin = findViewById(R.id.btnfind)
        btnAccount = findViewById(R.id.btnCancel)
        editID = findViewById(R.id.editID)
        editPW = findViewById(R.id.editPW)

        //로그인 버튼 클릭 이벤트
        btnLogin.setOnClickListener {
            verifyText()
            TryLogin()
        }
        //새로운 계정 버튼 클릭 이벤트
        btnAccount.setOnClickListener {
            val intent = Intent(this, NewAccountActivity::class.java)
            startActivity(intent)
            finish()
        }
        //IDPW찾기 버튼 클릭 이벤트
        findViewById<Button>(R.id.btnfind1).setOnClickListener{
            val intent = Intent(this, Find_IDPWActivity::class.java)
            intent.putExtra("ID",editID.text.toString())
            startActivity(intent)
        }

        //ID/PW찾기 화면에서 넘어오는 새로운 비밀번호를 넘겨받기
        val intent = getIntent()
        if (intent != null) {
            newPW = intent.getStringExtra("PW")
            editPW.setText(newPW)
        }
    }
    // 로그인을 위한 IP/PW의 유효성을 판단하는 함수.
    fun verifyText() : Boolean
    {
        val ID = editID.text.toString()
        val PW = editPW.text.toString()

        if(ID.length < 5 || PW.length < 5) return false

        return true
    }
    fun TryLogin()
    {
        // > 리턴값 : 로그인 실패시 메시지 출력. 현재 페이지 유지
        // > 성공시 메인화면으로 이동
        //  [{"rtncode" : "01"}]
        // > 실패시
        //  [{"rtncode" : "00", "msg" : "비밀번호가 맞지 않습니다. 올바른 비밀번호를 입력하시기 바랍니다."}]
        //  [{"rtncode" : "09", "msg" : "존재하지 않는 ID입니다."}]
        myController.loginid({ succeed, reply, replyArray->
            //응답을 받으면
            if(replyArray == null)
                Throwable("Unknown Error.").printStackTrace() // TODO : String Conversion
            var rtncode = 0
            try {
                rtncode = Integer.parseInt(replyArray.getJSONObject(0).getString("rtncode") ?: "1")
            }catch(e: JSONException){ rtncode = 1 }

            if(rtncode != 1)
            {
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {""}
                if(errorstr.isEmpty())errorstr = getStringCode(rtncode,null) ?: "Unknown Error." // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt(),"로그인 실패.") // TODO : String Conversion

                MessageAlert("loginhp",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
            else
            {
                processLanguageCode(replyArray)
                saveCodeData()
                MyApplication.InitHashes()
                callAdminSetting()
            }
        },editID.text.toString(),editPW.text.toString())
    }
    fun processLanguageCode(replyArray : JSONArray)
    {
        for (i in 0 until replyArray.length()) {
            val obj: JSONObject = replyArray.getJSONObject(i)
            getStringCodeHash.put(
                obj.getString("code").toInt().toString(),
                obj.getString("val")
            )
            Log.i("Language",obj.toString())
        }
        idxUser = try{ getStringCode(9988).toInt() }catch(e:Exception){0}
    }
    fun callAdminSetting()
    {
        myController.adm({ isSucceed, reply, replyArray -> isAdmin = false; if(isSucceed) {
            val rtncode = try { replyArray.getJSONObject(0).getString("rtncode").toInt() } catch (e: Throwable) { 1 }
            if (rtncode == 31) {// 권한이 없습니다
                clientInfoArray = emptyArray()
                isAdmin = false
            }
            else if (rtncode == 1) {// 관리자 성공
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
                clientInfoArray = array

                isAdmin = true

                goMainView()
            } else {
                var errorstr = try {
                    replyArray.getJSONObject(0).getString("msg")
                } catch (e: Throwable) {
                    getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                }
                if(errorstr.toIntOrNull() != null)
                    errorstr = getStringCode(errorstr.toInt())

                MessageAlert("admin", errorstr) {
                    goMainView()
                }
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
        }

        }, idxUser.toString())
    }
    fun goMainView() {
        val intent = Intent(this, MainActivity::class.java)
        intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP // show only main activity
        startActivity(intent)
        super.finish()
        //processLanguageCode(replyArray)
        //saveCodeData()
    }

    fun MessageAlert(Title: String?, Messages: String?, callback : (()->Unit)? = null) {
        AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
            .setPositiveButton("OK") {_,_->}.create().show()
        if(callback != null)
            callback()
    } // TODO : String Conversion

    override fun finish(){
        appTerminate()
    }
    fun appTerminate() {
        finishAffinity()
        System.runFinalization()
        System.exit(0)
    }

}