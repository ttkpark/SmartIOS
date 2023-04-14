package com.example.inout_nodeapp;

import android.app.Application;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Objects;
import java.util.logging.Level;
import java.util.logging.Logger;

public class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();

        InitHashes();

        try {
            PackageInfo pi = getPackageManager().getPackageInfo(getPackageName(),0);
            versionName = pi.versionName;
            versionCode = String.valueOf(pi.versionCode);
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            versionName = "";
            versionCode = "";
        }


    }
    private String parseDomainData_getDomain(int domaindatacode)
    {
        String[] split = getStringCode(domaindatacode).split("\\^");
        if(split.length == 2) return split[1];
        else return "";
    }

    private void domainSetting() {
        if(getStringCode(9992).length() > 3)//있으면
        {
            getStringCodeHash.put("10112", parseDomainData_getDomain(9992));// 현재 도메인 설정
        }
        else//비어있으면
        {
            getStringCodeHash.put("10112", getStringCode(10110));// 현재 도메인 설정
            return;
        }


        String NewDomainData = getStringCode(9993);
        if (NewDomainData.length() > 3) {// 비어있지 않으면
            String[] split = NewDomainData.split("\\^");
            if(split.length == 2) {

                SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                try {
                    Date domainStartDate = format.parse(split[0]);
                    Date now = new Date();
                    if(domainStartDate.compareTo(now) < 0)// 시작 시간이 현재보다 과거에 있으면
                    {
                        //적용 시작
                        getStringCodeHash.put("10111", parseDomainData_getDomain(9992));//기존 도메인을 10111에 백업
                        getStringCodeHash.put("10112",split[1]);// 새로운 도메인 업데이트
                        return;//탈출
                    }
                } catch (ParseException e) {
                    e.printStackTrace();
                    Log.e("domainSetting","NewDomainData.spilit.length ParseException");
                }
            }
            else Log.e("domainSetting","NewDomainData.spilit.length is not 2.");
            getStringCodeHash.put("10112",parseDomainData_getDomain(9992));// 현재 도메인 설정
        }
    }

    static final int TESTING = 0;
    static int idxUser_Test = 10;

    static boolean isMainActivityAlive = false;
    static String versionName;
    static String versionCode;
    static GatePosition[] GateTimeList;
    static SimpleGatePosition[] SimpleGateTimeList;
    static APInfo[] APInfoList;
    static CompanyList[] companyList;
    static int APIResult;
    static int SelectedAPIndex = 0;
    static String clientcode = null;
    static String privClientcode = null;
    static int idxUser;
    static AccountData accountData;
    static String AppColor  = "#FF6200EE";
    static String TextColor = "#FFFFFFFF";
    static boolean isAdmin;
    static GatePosition GatePositionToRemark;

    static final int UUID_DECODE_ERROR = 133;


    static String getStringCode(int CODE)
    {
        String getstring;
        getstring = getStringCodeHash.get(String.valueOf(CODE));
        if(Objects.equals(getstring, null)){
            getstring = "~";
            Log.d("StringCode","Unvalid data " + CODE);
        }
        return getstring;
    }
    static String getStringCode(int CODE,String repStr)
    {
        String getstring;
        getstring = getStringCodeHash.get(String.valueOf(CODE));
        if(Objects.equals(getstring, null)){
            getstring = repStr;
        }
        return getstring;
    }

    void LangvSucceed()
    {
        UpdateCombinedStrings();
        saveCodeData();
    }
    void UpdateCombinedStrings(){
        if(!getStringCode(768).equals("~")) {
            getStringCodeHash.put("10108",String.format("[%s] %s", getStringCode(768), getStringCode(774)));
            getStringCodeHash.put("10109",String.format("[%s] %s",getStringCode(768),"Domain Set"));
        }

        isAdmin = getStringCode(9991).equals("Y");
    }

    public static String APIRtnErrorCode(int code) {
        String ErrorCode = getStringCodeHash.get(String.valueOf(code));
        if(Objects.equals(ErrorCode, null)){
            ErrorCode = "Unknown : " + code;
            Log.d("RtnErrorCode",ErrorCode);
        }
        return ErrorCode;
    }
    void InitHashes()
    {
        setstringCodeHashfromStrArr(getStringCodeHash,strCodeDefault);
        setstringCodeHashfromStrArr(getStringCodeHash,APIRtnErrorCodeData);
        reloadCodeData();//파일로부터 읽는다.
        getStringCodeHash.remove("10112");

        UpdateCombinedStrings();

        SelectColor();

        //AppColor = "#ff7f7f7f";
        //setTheme(R.style.Theme_MaterialComponents_DayNight_NoActionBar);
        //SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(this).edit();
        //editor.putInt("color", 0xff7fffff);
        //editor.putInt("theme",R.style.Theme_MaterialComponents_DayNight_NoActionBar);
        //editor.apply();
        clientcode = getStringCode(10900,null);//load clientcode
        privClientcode = getStringCode(10902,null);//load clientcode
        isAdmin = getStringCode(9991).equals("Y");


        domainSetting();
        saveCodeData();
    }
    static void SelectColor()
    {
        String Color = getStringCode(9994);
        if(!Color.equals("~")) {
            String[] colors = Color.split(",");
            AppColor  = "#ff" + colors[0];
            TextColor = "#ff" + colors[1];
        }
        Log.i("SetColor","Color : " + AppColor);
        Log.i("SetColor","Text  : " + TextColor);
    }
    static void setstringCodeHashfromStrArr(HashMap<String,String> Hash,String[][] array){
        for (String[] strings : array) {
            Hash.put(strings[0], strings[1]);
        }
    }
    static HashMap<String,String> getStringCodeHash = new HashMap<>();
    static String[][] APIRtnErrorCodeData = {
                    { "1","정상"}, { "2","초기 환경정보 설정 실패(try)"}, { "3","패킷 데이터가 없습니다."}, { "4","정당한 패킷이 아닙니다."}, { "5"," DB저장 오류"}, { "6","패킷 나누기 실패(try)"}, { "7","초기 환경정보 설정 실패(try)"}, { "8","복호화 키 오류(db.key, db.iv)"}, { "9","등록되지 않은 코드 및 사용자"},
                    {"10","사용중 이므로 삭제 불가능"}, {"11","Decoding Packet String"}, {"12","Get ConnectStringName"}, {"13","사용/중지됨"}, {"14","출입안되는 Room"}, {"15","출입 데이터가 없습니다."}, {"16","데이터가 없습니다."}, {"21","암호키파일 없음"}, {"31","관리자가 아닙니다. 권한이 없습니다."}, {"99","기타 시스템 오류. 재시도 요망"},
            };
    static String[][] strCodeDefault = {
                    {"10900",""},//clientcode
                    {"10901",""},//UUID
                    {"10902",""},//선택한 clientcode
                    {"10903",""},//선택한 Type
                    {"10904",""},//선택한 language
                    {"10904","17"},//idxUser

                    {"10101","AP고유번호"},
                    {"10102","센서 종류"},
                    {"10103","권한 요청 필요"},
                    {"10104","로그인을 위한 전화번호와 인터넷 권한, 외부 저장소 권한이 필요합니다.\n앱을 이용하려면 권한에 모두 동의하시기 바랍니다."},
                    {"10105","Permission denied.(phone number)"},
                    {"10106","Data Error\n"},
                    {"10107","Decoding UUID Error"},
                    {"10108","[관리자]등록 및 삭제"},
                    {"10109","[관리자] Domain Set"},
                    {"10110","http://www.eduwon.net:8807/"},//하드코딩된 도메인(초기 도메인)
                    {"10111","http://www.eduwon.net/"},//백업 도메인
                    {"10112","http://www.eduwon.net:8807/"},//현재 API 도메인
            };
    static String[][] languageCode = {
                    {"afza","Afrikaans(Suid-Afrika)"}, {"amet","አማርኛ(ኢትዮጵያ)"}, {"arae","العربية (الإمارات)"}, {"arbh","العربية (البحرين)"}, {"ardz","العربية (الجزائر)"}, {"areg","العربية (مصر)"}, {"aril","العربية (إسرائيل)"},
                    {"ariq","العربية (العراق)"}, {"arjo","العربية (الأردن)"}, {"arkw","العربية (الكويت)"}, {"arlb","العربية (لبنان)"}, {"arma","العربية (المغرب)"}, {"arom","العربية (عُمان)"}, {"arps","العربية (فلسطين)"},
                    {"arqa","العربية (قطر)"}, {"arsa","العربية (السعودية)"}, {"artn","العربية (تونس)"}, {"azaz","Azərbaycan(Azərbaycan)"}, {"bgbg","Български(България)"}, {"bnbd","বাংলা(বাংলাদেশ)"}, {"bnin","বাংলা(ভারত)"},
                    {"caes","Català(Espanya)"}, {"cscz","Čeština(Česká republika)"}, {"dadk","Dansk(Danmark)"}, {"dede","Deutsch(Deutschland)"}, {"elgr","Ελληνικά(Ελλάδα)"}, {"enau","English(Australia)"}, {"enca","English(Canada)"},
                    {"engb","English(Great Britain)"}, {"engh","English(Ghana)"}, {"enie","English(Ireland)"}, {"enin","‪English(India)"}, {"enke","English(Kenya)"}, {"enng","English(Nigeria)"}, {"ennz","‪English(New Zealand)"}, {"enph","English(Philippines)"},
                    {"ensg","English(Singapore)"}, {"entz","English(Tanzania)"}, {"enus","English(United States)"}, {"enza","English(South Africa)"}, {"esar","Español(Argentina)"}, {"esbo","Español(Bolivia)"}, {"escl","Español(Chile)"}, {"esco","Español(Colombia)"},
                    {"escr","Español(Costa Rica)"}, {"esdo","Español(República Dominicana)"}, {"esec","Español(Ecuador)"}, {"eses","Español(España)"}, {"esgt","Español(Guatemala)"}, {"eshn","Español(Honduras)"}, {"esmx","Español(México)"}, {"esni","Español(Nicaragua)"},
                    {"espa","Español(Panamá)"}, {"espe","Español(Perú)"}, {"espr","Español(Puerto Rico)"}, {"espy","Español(Paraguay)"}, {"essv","Español(El Salvador)"}, {"esus","Español(Estados Unidos)"}, {"esuy","Español(Uruguay)"}, {"esve","Español(Venezuela)"}, {"eues","Euskara(Espainia)"},
                    {"fair","فارسی (ایران)"}, {"fifi","Suomi(Suomi)"}, {"filph","Filipino(Pilipinas)"}, {"frca","Français(Canada)"}, {"frfr","Français(France)"}, {"gles","Galego(España)"}, {"guin","ગુજરાતી(ભારત)"}, {"heil","עברית (ישראל)"}, {"hiin","हिन्दी(भारत)"}, {"hrhr","Hrvatski(Hrvatska)"}, {"huhu","Magyar(Magyarország)"},
                    {"hyam","Հայ(Հայաստան)"}, {"idid","Bahasa Indonesia(Indonesia)"}, {"isis","Íslenska(Ísland)"}, {"itit","Italiano(Italia)"}, {"jajp","日本語（日本）"}, {"jvid","Jawa(Indonesia)"}, {"kage","ქართული(საქართველო)"}, {"kmkh","ភាសាខ្មែរ(កម្ពុជា)"}, {"knin","ಕನ್ನಡ(ಭಾರತ)"}, {"kokr","한국어(대한민국)"}, {"lola","ລາວ(ລາວ)"},
                    {"ltlt","Lietuvių(Lietuva)"}, {"lvlv","Latviešu(latviešu)"}, {"mlin","മലയാളം(ഇന്ത്യ)"}, {"mrin","मराठी(भारत)"}, {"msmy","Bahasa Melayu(Malaysia)"}, {"nbno","Norsk bokmål(Norge)"}, {"nenp","नेपाली(नेपाल)"}, {"nlnl","Nederlands(Nederland)"}, {"plpl","Polski(Polska)"}, {"ptbr","Português(Brasil)"},
                    {"ptpt","‪Português(Portugal)"}, {"roro","Română(România)"}, {"ruru","Русский(Россия)"}, {"silk","සිංහල(ශ්රී ලංකාව)"}, {"sksk","Slovenčina(Slovensko)"}, {"slsi","Slovenščina(Slovenija)"}, {"srrs","Српски(Србија)"}, {"suid","Urang(Indonesia)"}, {"svse","Svenska(Sverige)"},
                    {"swke","Swahili(Kenya)"}, {"swtz","Swahili(Tanzania)"}, {"tain","தமிழ்(இந்தியா)"}, {"talk","தமிழ்(இலங்கை)"}, {"tamy","தமிழ்(மலேசியா)"}, {"tasg","தமிழ்(சிங்கப்பூர்)"}, {"tein","తెలుగు(భారతదేశం)"}, {"thth","ไทย(ประเทศไทย)"}, {"trtr","Türkçe(Türkiye)"},
                    {"ukua","Українська(Україна)"}, {"urin","اردو (بھارت)"}, {"urpk","اردو (پاکستان)"}, {"vivn","Tiếng Việt(Việt Nam)"}, {"yuehnthk","廣東話(香港)"}, {"zhcn","普通话(中国大陆)"}, {"zhhk","普通話(香港)"}, {"zhtw","國語(台灣)"}, {"zuza","IsiZulu(Ningizimu Afrika)"},
            };
    static String[][] loginType = {{"S","student"}, {"T","teacher"}, {"P","professor"}, {"E","employee"}, {"M","manager"}};
    static String CodeFileName =  File.separator + "system.cfg";
    static String CodeFileDir = Environment.getExternalStorageDirectory() + File.separator + "smartios";

    static String ReadUUID() {
        return getStringCode(10901,null);
    }


    public void ResetFile() {
        Log.d("ResetFile",CodeFileDir + CodeFileName);
        File file = new File(CodeFileDir + CodeFileName);

        if(file.exists())
            file.delete();
    }
    static int SaveTrying = 0;
    void reloadCodeData()
    {

        Log.d("FileRead",CodeFileDir + CodeFileName);
        File file = new File(CodeFileDir + CodeFileName);
        try {
            if(!file.exists())
            {
                File directory = new File(CodeFileDir);
                if(!directory.exists())
                    directory.mkdirs();

                file.createNewFile();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            SaveTrying++;
            if(SaveTrying==1) {
                reloadCodeData();
                CodeFileDir = getFilesDir().toString();
            }

            e.printStackTrace();
            Logger.getLogger(MyApplication.class.getName()).log(Level.SEVERE,null,e);
        }

        try {
            ObjectInputStream ois = new ObjectInputStream(new FileInputStream(file));
            getStringCodeHash = (HashMap<String, String>)ois.readObject();
        } catch (ClassNotFoundException | IOException e) {
            e.printStackTrace();
        }

    }
    void saveCodeData()
    {
        Log.d("FileWrite",CodeFileDir + CodeFileName);
        File file = new File(CodeFileDir + CodeFileName);
        try {
            if(!file.exists())file.createNewFile();
            ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(file));
            oos.writeObject(getStringCodeHash);
            oos.close();
        } catch (IOException e) {
            //e.printStackTrace();
            Log.e("FILE Save","file save failed. : " + e.toString());
        }
    }

    void checkFileAndRestore()
    {
        if(!iscodeDataVaild()) {
            InitHashes();
            return;
        }

        Log.d("FileRead",CodeFileDir + CodeFileName);
        File file = new File(CodeFileDir + CodeFileName);
        try {
            if(!file.exists())
            {
                File directory = new File(CodeFileDir);
                if(!directory.exists())
                    directory.mkdirs();

                file.createNewFile();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            SaveTrying++;
            if(SaveTrying==1) {
                reloadCodeData();
                CodeFileDir = getFilesDir().toString();
            }

            e.printStackTrace();
            Logger.getLogger(MyApplication.class.getName()).log(Level.SEVERE,null,e);
        }
        saveCodeData();
    }
    static boolean iscodeDataVaild(){
        return getStringCode(9995,null) != null;
    }

    public void onDomainChanged(String newDomain)
    {
        getStringCodeHash.put("9996",newDomain);
        saveCodeData();
    }


    public static class InformationSettingData
    {
        String SensorType;
        String str_ModuleName;
        String str_ModuleDistance;
        String str_suffix;


        void CheckValue() throws IntegrityException {
            try {
                if (SensorType.equals("") || str_ModuleName.equals("")
                        || str_ModuleDistance.equals("") || str_suffix.equals(""))
                    throw new IntegrityException();
            }
            catch(NullPointerException ignored)
            {
                throw new IntegrityException();
            }
        }
    }
    public static class IPSettingData
    {
        final int IPARRAYSize = 4*5;
        byte[] IP;
        byte[] DefGate;
        byte[] SubMask;
        byte[] DNS1;
        byte[] DNS2;
        String APCode;
        String SensorType;

        void CheckValue() throws IntegrityException {
        try {
            if (IP.equals(new byte[]{0,0,0,0}) || IP.length != 4)
                throw new IntegrityException();
        }
        catch(NullPointerException ignored)
        {
            throw new IntegrityException();
        }
    }
    }
    public static class DomainSettingData
    {
        String domain;
        String APCode;

        void CheckValue() throws IntegrityException {
            try {
                if (domain.equals(""))
                    throw new IntegrityException();
            }
            catch(NullPointerException ignored)
            {
                throw new IntegrityException();
            }
        }
    }
    public static class AccountData
    {
        String Name;
        String ID;
        String lang;
    }

    public static class GatePosition
    {
        String time;
        String gatename;
    }
    public static class SimpleGatePosition
    {
        String time;
        String io_type;
        String gatename;
        String io;
    }
    public static class APInfo
    {
        String apname;
        String apcode;
        String dist;
        String useyn;
        String flag;
        String IPAddr;
        String sensType;
    }
    public static class CompanyList
    {
        String code;
        String CompanyName;
    }
    public static class NewAccountData
    {
        String ID;
        String Name;
        String clientcode;
        String Type;
        String phone;
        String Language;
        String Password;
    }

}

class IntegrityException extends Exception{}
