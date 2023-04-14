package com.example.myfirstkotlin

import android.content.Intent
import android.os.Bundle
import android.os.Looper
import android.util.Base64
import android.util.Log
import android.widget.Button
import android.widget.EditText
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import org.json.JSONArray
import org.json.JSONException

class Find_IDPWActivity : AppCompatActivity() {
    lateinit var Controller : MyController
    lateinit var editID : EditText
    lateinit var editName : EditText
    lateinit var btnFind : Button
    lateinit var btnCancel : Button
    var ID : String = ""
    var Name : String = ""
    var PW : String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_find_idpwactivity)

        Controller = MyController(this, Looper.getMainLooper())

        editID  = findViewById(R.id.editID)
        editName = findViewById(R.id.editName)
        btnFind  = findViewById(R.id.btnfind)
        btnCancel = findViewById(R.id.btnCancel)

        btnFind.setOnClickListener{ FindIDPW() }
        btnCancel.setOnClickListener{ onCancel() }

        val intent = getIntent()
        if (intent != null) {
            ID = intent.getStringExtra("ID") ?: ""
            editID.setText(ID)
        }

    }
    fun FindIDPW()
    {
        ID = editID.text.toString()
        Name = editName.text.toString()

        Controller.findpwdid({ isSucceed, reply, replyArray ->
            val rtncode: Int

            rtncode = try {Integer.parseInt(replyArray.getJSONObject(0).getString("rtncode") ?: "1")}
            catch(e: JSONException) {0}

            if(rtncode != 1)
            {
                var errorstr = try{replyArray.getJSONObject(0).getString("msg")}
                catch(e : Throwable) {""}
                if(errorstr.isEmpty())errorstr = MyApplication.getStringCode(rtncode, null) ?: "Unknown Error." // TODO : String Conversion
                if(errorstr.toIntOrNull() != null)
                    errorstr = MyApplication.getStringCode(errorstr.toInt())

                MessageAlert("findpwdid",errorstr)
                Throwable("Unknown Error.($rtncode)").printStackTrace() // TODO : String Conversion
            }
            else {
                val EncodedPW = try {
                    replyArray.getJSONObject(0).getString("npw")
                } catch (e: Throwable) {
                    null
                }
                if (EncodedPW == null) {
                    MessageAlert("loginid", "NPW Error") // TODO : String Conversion
                }
                PW = Controller.Crypt.AESDecode(Base64.decode(Base64.decode(EncodedPW,0),0),Controller.AESNo)

                Log.i("TAG", "AESNo : " + Controller.AESNo)
                Log.i("TAG","AESIV : " + CryptUtil.BytesToString(Base64.decode(Controller.Crypt.AESKeyIVs[Controller.AESNo - 1][0], 0)))
                Log.i("TAG","AESKey : " + CryptUtil.BytesToString(Base64.decode(Controller.Crypt.AESKeyIVs[Controller.AESNo - 1][1],0)))

                AlertDialog.Builder(this).setTitle("New Password").setMessage("new password is $PW") // TODO : String Conversion
                    .setPositiveButton("OK") {_,_->}.setOnDismissListener { CallbacktoLoginActivity() }
                    .create().show() // TODO : String Conversion
            }

        },ID,Name)
    }
    fun onCancel()
    {
        val intent = Intent(this,LoginActivity::class.java)
        intent.putExtra("ID",ID)
        startActivity(intent)
    }
    fun CallbacktoLoginActivity()
    {
        val intent = Intent(this,LoginActivity::class.java)
        intent.putExtra("ID",ID)
        intent.putExtra("PW",PW)
        startActivity(intent)
        finish()
    }
    fun MessageAlert(Title: String?, Messages: String?) {
        AlertDialog.Builder(this).setTitle(Title).setMessage(Messages)
            .setPositiveButton("OK") {_,_->}.create().show() // TODO : String Conversion
    }
}