package com.example.myfirstkotlin;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;

import org.json.JSONArray;

import java.util.Calendar;
import java.util.Random;

import static com.example.myfirstkotlin.CryptUtil.Base64Encode;
import static com.example.myfirstkotlin.CryptUtil.sha256;


@SuppressLint("DefaultLocale")
public class MyController {
    CryptUtil Crypt;
    Context context;
    Handler APIHandler;

    public MyController(Context context1, Looper looper){
        context = context1;
        APIHandler = new Handler(looper);
        Crypt = new CryptUtil();
    }

    int AESNo;
    int CValue;//0~9
    void GetnerateAESKey30(){
        Calendar cal = Calendar.getInstance();
        int month = cal.get(Calendar.MONTH) + 1;
        int day = cal.get(Calendar.DAY_OF_MONTH);

        Random random = new Random();
        random.setSeed(cal.getTimeInMillis());

        CValue = random.nextInt(10);//0~9
        AESNo = ((month + day)/2) + CValue;//1~30
    }
    int DValue;//1000~9999
    void GetnerateAESKey300(){
        Calendar cal = Calendar.getInstance();
        int month = cal.get(Calendar.MONTH) + 1;
        int day = cal.get(Calendar.DAY_OF_MONTH);

        Random random = new Random();
        random.setSeed(cal.getTimeInMillis());
        CValue = random.nextInt(10);//0~9
        DValue = random.nextInt(9000) + 1000;//0~9

        int FValue = ((month + day)/2) + CValue - 1;//0~29

        AESNo = FValue*10 + DValue%10 + 1;//1~300
    }
    public void sendAPIandGET(APIReturnCalback callback,APIClass api)
    {
        api.start();

        try {
            api.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        APIHandler.sendMessage(Message.obtain(APIHandler,
                () -> callback.Return(api.isSucceed,api.reply,api.replyArr)));

        if(api.isAlive())
            api.interrupt();
    }
    public Thread apiWithparam3(APIReturnCalback callback,String TaskName,String id1) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();

            Log.d(TaskName,"id1 = " + id1);
            String id1Encoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(id1,AESNo),0),0));

            String data = String.format("/%s/%d%d/%s",id1Encoded,DValue,CValue,TaskName);

            API.configureAPI(TaskName,"http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }

    public Thread Echo(APIReturnCalback callback,String idxUser)
    {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();

            Random random = new Random();
            random.setSeed(System.currentTimeMillis());
            int echoval = random.nextInt(9000) + 1000;//0~9

            API.configureAPI("Echo","http://www.eduwon.net:8807","/api/sio","/echo_" + echoval + "u" + idxUser);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread join(APIReturnCalback callback,String hp,String name,String id,String logintype,String languageCode,String password) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();


            String hpEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(hp,AESNo),0),0));
            String nameEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(name,AESNo),0),0));
            String idEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(id,AESNo),0),0));

            String passEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(password,AESNo),0),0));


//      /api/sio/암호화(hp)/암호화(이름)/암호화(id)/유형/언어코드/암호화(암호)/aes_no/join
//                    -- {id}암호화(hp) : HttpUtility.UrlEncode(Base64(AES암호화.aes_no(hp)))...+1 555-521-5554
//                    -- {val}암호화(이름) : HttpUtility.UrlEncode(Base64(AES암호화.aes_no(이름)))
//                    -- {id2}암호화(id) : HttpUtility.UrlEncode(Base64(AES암호화.aes_no(id)))
//                    -- {val2}유형 : E (학생이든 사원이든 기본값 E로 한다.)
//                    -- {id3}언어코드 : kokr (가장긴 코드 : yuehanthk)
//            -- {val3}암호화(암호) : HttpUtility.UrlEncode(Base64(AES암호화.aes_no(암호)))
//                    -- {id4}aes_no : 임의값(1000~9999) + aes_no숫자값(0~9)

            @SuppressLint("DefaultLocale")
            String data = String.format("/%s/%s/%s/%s/%s/%s/%d%d/join",hpEncoded,nameEncoded,idEncoded,logintype,languageCode,passEncoded,DValue,CValue);

            API.configureAPI("join","http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread loginhp(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"loginhp",idxUser);
    }
    public Thread loginid(APIReturnCalback callback,String id,String pwd) {
        return apiWithparam3(callback,"loginid",String.format("%s^%s",id,sha256(pwd)));
    }
    public Thread findpwdid(APIReturnCalback callback,String id,String name) {
        return apiWithparam3(callback,"findpwdid",String.format("%s^%s",id,name));
    }
    public Thread langv(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"langv",idxUser);
    }
    public Thread pers(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"pers",idxUser);
    }
    public Thread cpers(APIReturnCalback callback,String idxUser,String id,String Name,String pwd) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();

            String SHAPWD = sha256(pwd);

            String idxUser_id = new String(Base64Encode(Base64Encode(Crypt.AESEncode(idxUser + "^" +  id,AESNo),0),0));
            String Name_pwd   = new String(Base64Encode(Base64Encode(Crypt.AESEncode(Name + "^" + SHAPWD,AESNo),0),0));

            String data = String.format("/%s/%s/%d%d/cpers",idxUser_id,Name_pwd,DValue,CValue);

            //-- {id}암호화(idxUser^id)
            //-- {val}암호화(이름^SHA256(pwd))

            API.configureAPI("langv","http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread iodtlist(APIReturnCalback callback,String idxUser,String date,String Clientcode) {
        return apiWithparam3(callback,"iodtlist",date+"^"+idxUser+"^"+Clientcode);
    }
    public Thread adm(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"adm",idxUser);
    }
    public Thread aplist(APIReturnCalback callback,String Clientcode) {
        return apiWithparam3(callback,"aplist",Clientcode);
    }
    public Thread comlist(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"comlist",idxUser);
    }
    public Thread visitcomlist(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"visitcomlist",idxUser);
    }
    public Thread visitnotcomlist(APIReturnCalback callback,String idxUser) {
        return apiWithparam3(callback,"visitnotcomlist",idxUser);
    }
    public Thread setusecom(APIReturnCalback callback,String idxUser,String Clientcode) {
        return apiWithparam3(callback,"setusecom",idxUser+"^"+Clientcode);
    }
    public Thread setnotusecom(APIReturnCalback callback,String idxUser,String Clientcode) {
        return apiWithparam3(callback,"setnotusecom",idxUser+"^"+Clientcode);
    }
    public Thread basiccom(APIReturnCalback callback,String idxUser,String Clientcode) {
        return apiWithparam3(callback,"basiccom",idxUser+"^"+Clientcode);
    }
    public Thread apin(APIReturnCalback callback,String idxUser,String idxEquip,String distance,String sensortype,String opendoor,String timezone) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();

            String idEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(idxUser+"^"+idxEquip+"^"+distance,AESNo),0),0));
            String valEncoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(sensortype+"^"+opendoor+"^"+timezone,AESNo),0),0));

            String data = String.format("/%s/%s/%d%d/apin",idEncoded,valEncoded,DValue,CValue);

            API.configureAPI("apin","http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread apout(APIReturnCalback callback,String idxUser,String idxEquip) {
        return apiWithparam3(callback,"apout",idxUser+"^"+idxEquip);
    }
    public Thread apuse(APIReturnCalback callback,String idxUser,String idxEquip) {
        return apiWithparam3(callback,"apuse",idxUser+"^"+idxEquip);
    }
    public Thread apnotuse(APIReturnCalback callback,String idxUser,String idxEquip) {
        return apiWithparam3(callback,"apnotuse",idxUser+"^"+idxEquip);
    }
    public Thread apipc(APIReturnCalback callback,String idxUser,String idxEquip,String ipBase64) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();

            String id1_Encoded  = new String(Base64Encode(Base64Encode(Crypt.AESEncode(idxUser + "^" +  idxEquip,AESNo),0),0));
            String val1_Encoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(ipBase64,AESNo),0),0));

            String data = String.format("/%s/%s/%d%d/apipc",id1_Encoded,val1_Encoded,DValue,CValue);

            API.configureAPI("apipc","http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread apgps(APIReturnCalback callback,String idxUser,String idxEquip,String ipBase64) {
        Thread task = new Thread(() -> {
            APIClass API = new APIClass();
            GetnerateAESKey300();

            String id1_Encoded  = new String(Base64Encode(Base64Encode(Crypt.AESEncode(idxUser + "^" +  idxEquip,AESNo),0),0));
            String val1_Encoded = new String(Base64Encode(Base64Encode(Crypt.AESEncode(ipBase64,AESNo),0),0));

            String data = String.format("/%s/%s/%d%d/apgps",id1_Encoded,val1_Encoded,DValue,CValue);

            API.configureAPI("apgps","http://www.eduwon.net:8807","/api/sio",data);
            sendAPIandGET(callback,API);
        });
        task.start();
        return task;
    }
    public Thread apipc(APIReturnCalback callback,String idxEquip,String ipBase64) {
        return apiWithparam3(callback,"apipc",idxEquip+"^"+ipBase64);
    }

    public Thread aptotstat(APIReturnCalback callback,String idxUser,String Clientcode) {
        return apiWithparam3(callback,"totstatus",idxUser + "^" + Clientcode);
            // /api/sio /암호화(11^1000)/aes_no/totstatus
    }
}
interface APIReturnCalback{
    void Return(boolean succeed, String reply, JSONArray replyArray);
}