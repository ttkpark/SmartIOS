package com.example.myfirstkotlin

import android.app.Application
import android.content.pm.PackageManager
import android.content.res.ColorStateList
import android.util.Log
import java.io.*
import java.lang.String.format
import java.text.ParseException
import java.text.SimpleDateFormat
import java.util.*

class MyApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        CodeFileDir = filesDir.toString() + File.separator + "smartios"
        InitHashes()
        try {
            val pi = packageManager.getPackageInfo(packageName, 0)
            versionName = pi.versionName
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
            versionName = ""
        }
    }

    companion object {
        fun InitHashes() {
            getStringCodeHash.clear()
            setstringCodeHashfromStrArr(getStringCodeHash, strCodeDefault)
            setstringCodeHashfromStrArr(getStringCodeHash, loginType)
            reloadCodeData() //파일로부터 읽는다.

            updateColorFromStringCode()
            domainSetting()
            saveCodeData()
        }

        fun setstringCodeHashfromStrArr(Hash: HashMap<String, String?>, array: Array<Array<String>>) {
            for (strings in array) Hash[strings[0]] = strings[1]
        }

        fun parseDomainData_getDomain(domaindatacode: Int): String {
            val split = getStringCode(domaindatacode).split("\\^").toTypedArray()
            return if (split.size == 2) split[1] else ""
        }

        fun domainSetting() {
            if (getStringCode(9992, null) != null) //있으면
            {
                getStringCodeHash["10112"] = parseDomainData_getDomain(9992) // 현재 도메인 설정
            } else  //비어있으면
            {
                getStringCodeHash["10112"] = getStringCode(10110) // 현재 도메인 설정
                return
            }
            val NewDomainData = getStringCode(9993, null)
            if (NewDomainData != null) { // 비어있지 않으면
                val split = NewDomainData.split("\\^").toTypedArray()
                if (split.size == 2) {
                    val format = SimpleDateFormat("yyyy-MM-dd HH:mm:ss")
                    try {
                        val domainStartDate = format.parse(split[0])
                        val now = Date()
                        if (domainStartDate != null && domainStartDate < now) // 시작 시간이 현재보다 과거에 있으면
                        {
                            //적용 시작
                            getStringCodeHash["10111"] =
                                parseDomainData_getDomain(9992) //기존 도메인을 10111에 백업
                            getStringCodeHash["10112"] =
                                split[1] // 새로운 도메인 업데이트
                            return  //탈출
                        }
                    } catch (e: ParseException) {
                        e.printStackTrace()
                        Log.e("domainSetting", "NewDomainData.spilit.length ParseException")
                    }
                } else Log.e("domainSetting", "NewDomainData.spilit.length is not 2.")
                getStringCodeHash["10112"] = parseDomainData_getDomain(9992) // 현재 도메인 설정
            }
        }

        fun ResetFile() {
            Log.d("ResetFile", CodeFileDir + CodeFileName)
            val file = File(CodeFileDir + CodeFileName)
            if (file.exists()) file.delete()
        }

        fun MakeFile() {
            var file = File(CodeFileDir + CodeFileName)

            try {
                if (!file.exists()) {
                    val directory = File(CodeFileDir)
                    if (!directory.exists()) directory.mkdirs()
                    file.createNewFile()
                }
            } catch (e: FileNotFoundException) {
                e.printStackTrace()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }

        fun reloadCodeData() {
            Log.d("FileRead", CodeFileDir + CodeFileName)
            MakeFile()

            val file = File(CodeFileDir + CodeFileName)
            try {
                val ois = ObjectInputStream(FileInputStream(file))
                getStringCodeHash = ois.readObject() as HashMap<String, String?>
            } catch (e: ClassNotFoundException) {
                e.printStackTrace()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }

        @JvmStatic fun saveCodeData() {
            Log.d("FileWrite", CodeFileDir + CodeFileName)
            val file = File(CodeFileDir + CodeFileName)
            try {
                if (!file.exists()) file.createNewFile()
                val oos = ObjectOutputStream(FileOutputStream(file))
                oos.writeObject(getStringCodeHash)
                oos.close()
            } catch (e: IOException) {
                //e.printStackTrace();
                Log.e("FILE Save", "file save failed. : $e")
            }
        }

        fun checkFileAndRestore() {
            if (!isCodeDataVaild()) {
                InitHashes()
                return
            }
            Log.d("FileRead", CodeFileDir + CodeFileName)
            MakeFile()
            saveCodeData()
        }

        fun changeSaveCode(Key:String,value:String?) {
            getStringCodeHash[Key] = value
            saveCodeData()
        }
        fun onDomainChanged(newDomain: String?) {
            changeSaveCode("9996",newDomain)
        }

        @JvmStatic fun getStringCode(CODE: Int): String {
            return getStringCode(CODE, null) ?: "~"
        }

        @JvmStatic fun getStringCode(CODE: Int, repStr: String?): String? =
            getStringCodeHash[CODE.toString()] ?: repStr

        @JvmStatic fun APIRtnErrorCode(code: Int): String = getStringCodeHash[code.toString()] ?: "Unknown : $code"
        @JvmStatic fun updateColorFromStringCode() {
            val Color = getStringCode(9994)
            if (Color != "~") {
                val colors = Color.split(",").toTypedArray()
                AppColor = "#ff" + colors[0]
                TextColor = "#ff" + colors[1]
                Log.i("SetColor", "Color : " + AppColor)
                Log.i("SetColor", "Text  : " + TextColor)
            }
        }

        @JvmStatic fun getBLEBeaconInterval(): Int = Integer.parseInt(getStringCode(9989))
        @JvmStatic fun getModifyLimitDate(): Int = Integer.parseInt(getStringCode(9990))
        @JvmStatic fun getAutoLoginAddress(): String? = getStringCode(9996, null)
        @JvmStatic fun getAppBannerCaption(): String? = getStringCode(9999, null)

        fun isCodeDataVaild(): Boolean = getStringCode(9999, null) != null
        var CodeFileName = File.separator + "Stringdata.maphash"
        lateinit var CodeFileDir: String

        @JvmStatic fun getClientName(clientcode:String?) : String?
        {
            if(clientcode == null)return null;
            if(clientInfoArray == null)return null
            for(clientinfo in clientInfoArray!!)
            {
                if(clientinfo == null)continue
                if(clientinfo.clientCode == clientcode)
                    return clientinfo.clientName
            }
            return null
        }
        fun makeNotification(title:String,text:String)
        {
            myNotification.sendMessage(myNotification.obtainMessage(3,arrayOf(title,text)))
        }

        const val TESTING = 0
        const val SUCCEED = 0
        const val FAILED = 1
        var BLEDuplicateEnabled = false
        var BLEDataCount = 0
        var isAdmin =  false
        var idxUser =  0
        var SelectedidxEquip =  1
        var isMainActivityAlive = false
        var versionName: String? = null
        var clientcode: String? = null
        var privClientcode: String? = null
        var duplicateUserArray = emptyArray<Int>()

        val myNotification = BackgroundService.myNotificationHandler()

        @JvmField var AppColor = "#FF6200EE"
        @JvmField var TextColor = "#FFFFFFFF"


        @JvmField var getStringCodeHash = HashMap<String, String?>()
        @JvmField var strCodeDefault = arrayOf(
            arrayOf( "1","정상"),
            arrayOf( "2","초기 환경정보 설정 실패(try)"),
            arrayOf( "3","패킷 데이터가 없습니다."),
            arrayOf( "4","정당한 패킷이 아닙니다."),
            arrayOf( "5","DB저장 오류"),
            arrayOf( "6","패킷 나누기 실패(try)"),
            arrayOf( "7","환경정보 설정 실패(try)"),
            arrayOf( "8","복호화 키 오류(key, iv)"),
            arrayOf( "9","등록되지 않은 코드 및 사용자"),
            arrayOf("10","사용중이므로 삭제할 수 없습니다."),
            arrayOf("11","복호화 오류"),
            arrayOf("12","연결정보 없음"),
            arrayOf("13","등록되어 있지 않은 회사입니다."),
            arrayOf("14","출입이 제한되었습니다."),
            arrayOf("15","출입 데이터가 없습니다."),
            arrayOf("16","데이터가 없습니다."),
            arrayOf("17","출입이 안되는 회사입니다."),
            arrayOf("18","이미 등록된 휴대폰 번호입니다."),
            arrayOf("19","정당하지 않는 사용자입니다.(Hacking)"),
            arrayOf("20","이미 등록된 데이터가 있습니다."),
            arrayOf("21", "암호키파일 없음"),
            arrayOf("31", "관리자가 아닙니다. 권한이 없습니다."),
            arrayOf("99", "기타 시스템 오류. 재시도 요망"),

            //    arrayOf("10900", ""),//clientcode
            //    arrayOf("10901", ""),
            //    arrayOf("10902", ""),
            //    arrayOf("10903", ""),
            //    arrayOf("10904", ""),
            //    arrayOf("10904", "17"),
            //    arrayOf("10905", "0"),//BLEDataCount

            //    arrayOf("10101", "AP고유번호"),
            //    arrayOf("10102", "센서 종류"),
            //    arrayOf("10103", "권한 요청 필요"),
            //    arrayOf(
            //        "10104",
            //        "로그인을 위한 전화번호와 인터넷 권한, 외부 저장소 권한이 필요합니다.\n앱을 이용하려면 권한에 모두 동의하시기 바랍니다."
            //    ),
            //    arrayOf("10105", "Permission denied.(phone number)"),
            //    arrayOf("10106", "Data Error\n"),
            //    arrayOf("10107", "Decoding UUID Error"),
            //    arrayOf("10108", "[관리자]등록 및 삭제"),
            //    arrayOf("10109", "[관리자] Domain Set"),
            //    arrayOf("10110", "http://www.eduwon.net:8807/"),
            //    arrayOf("10111", "http://www.eduwon.net/"),
            //    arrayOf("10112", "http://www.eduwon.net:8807/")
        )
        @JvmField var languageCode = arrayOf(
            arrayOf("afza", "Afrikaans(Suid-Afrika)"),
            arrayOf("amet", "አማርኛ(ኢትዮጵያ)"),
            arrayOf("arae", "العربية (الإمارات)"),
            arrayOf("arbh", "العربية (البحرين)"),
            arrayOf("ardz", "العربية (الجزائر)"),
            arrayOf("areg", "العربية (مصر)"),
            arrayOf("aril", "العربية (إسرائيل)"),
            arrayOf("ariq", "العربية (العراق)"),
            arrayOf("arjo", "العربية (الأردن)"),
            arrayOf("arkw", "العربية (الكويت)"),
            arrayOf("arlb", "العربية (لبنان)"),
            arrayOf("arma", "العربية (المغرب)"),
            arrayOf("arom", "العربية (عُمان)"),
            arrayOf("arps", "العربية (فلسطين)"),
            arrayOf("arqa", "العربية (قطر)"),
            arrayOf("arsa", "العربية (السعودية)"),
            arrayOf("artn", "العربية (تونس)"),
            arrayOf("azaz", "Azərbaycan(Azərbaycan)"),
            arrayOf("bgbg", "Български(България)"),
            arrayOf("bnbd", "বাংলা(বাংলাদেশ)"),
            arrayOf("bnin", "বাংলা(ভারত)"),
            arrayOf("caes", "Català(Espanya)"),
            arrayOf("cscz", "Čeština(Česká republika)"),
            arrayOf("dadk", "Dansk(Danmark)"),
            arrayOf("dede", "Deutsch(Deutschland)"),
            arrayOf("elgr", "Ελληνικά(Ελλάδα)"),
            arrayOf("enau", "English(Australia)"),
            arrayOf("enca", "English(Canada)"),
            arrayOf("engb", "English(Great Britain)"),
            arrayOf("engh", "English(Ghana)"),
            arrayOf("enie", "English(Ireland)"),
            arrayOf("enin", "‪English(India)"),
            arrayOf("enke", "English(Kenya)"),
            arrayOf("enng", "English(Nigeria)"),
            arrayOf("ennz", "‪English(New Zealand)"),
            arrayOf("enph", "English(Philippines)"),
            arrayOf("ensg", "English(Singapore)"),
            arrayOf("entz", "English(Tanzania)"),
            arrayOf("enus", "English(United States)"),
            arrayOf("enza", "English(South Africa)"),
            arrayOf("esar", "Español(Argentina)"),
            arrayOf("esbo", "Español(Bolivia)"),
            arrayOf("escl", "Español(Chile)"),
            arrayOf("esco", "Español(Colombia)"),
            arrayOf("escr", "Español(Costa Rica)"),
            arrayOf("esdo", "Español(República Dominicana)"),
            arrayOf("esec", "Español(Ecuador)"),
            arrayOf("eses", "Español(España)"),
            arrayOf("esgt", "Español(Guatemala)"),
            arrayOf("eshn", "Español(Honduras)"),
            arrayOf("esmx", "Español(México)"),
            arrayOf("esni", "Español(Nicaragua)"),
            arrayOf("espa", "Español(Panamá)"),
            arrayOf("espe", "Español(Perú)"),
            arrayOf("espr", "Español(Puerto Rico)"),
            arrayOf("espy", "Español(Paraguay)"),
            arrayOf("essv", "Español(El Salvador)"),
            arrayOf("esus", "Español(Estados Unidos)"),
            arrayOf("esuy", "Español(Uruguay)"),
            arrayOf("esve", "Español(Venezuela)"),
            arrayOf("eues", "Euskara(Espainia)"),
            arrayOf("fair", "فارسی (ایران)"),
            arrayOf("fifi", "Suomi(Suomi)"),
            arrayOf("filph", "Filipino(Pilipinas)"),
            arrayOf("frca", "Français(Canada)"),
            arrayOf("frfr", "Français(France)"),
            arrayOf("gles", "Galego(España)"),
            arrayOf("guin", "ગુજરાતી(ભારત)"),
            arrayOf("heil", "עברית (ישראל)"),
            arrayOf("hiin", "हिन्दी(भारत)"),
            arrayOf("hrhr", "Hrvatski(Hrvatska)"),
            arrayOf("huhu", "Magyar(Magyarország)"),
            arrayOf("hyam", "Հայ(Հայաստան)"),
            arrayOf("idid", "Bahasa Indonesia(Indonesia)"),
            arrayOf("isis", "Íslenska(Ísland)"),
            arrayOf("itit", "Italiano(Italia)"),
            arrayOf("jajp", "日本語（日本）"),
            arrayOf("jvid", "Jawa(Indonesia)"),
            arrayOf("kage", "ქართული(საქართველო)"),
            arrayOf("kmkh", "ភាសាខ្មែរ(កម្ពុជា)"),
            arrayOf("knin", "ಕನ್ನಡ(ಭಾರತ)"),
            arrayOf("kokr", "한국어(대한민국)"),
            arrayOf("lola", "ລາວ(ລາວ)"),
            arrayOf("ltlt", "Lietuvių(Lietuva)"),
            arrayOf("lvlv", "Latviešu(latviešu)"),
            arrayOf("mlin", "മലയാളം(ഇന്ത്യ)"),
            arrayOf("mrin", "मराठी(भारत)"),
            arrayOf("msmy", "Bahasa Melayu(Malaysia)"),
            arrayOf("nbno", "Norsk bokmål(Norge)"),
            arrayOf("nenp", "नेपाली(नेपाल)"),
            arrayOf("nlnl", "Nederlands(Nederland)"),
            arrayOf("plpl", "Polski(Polska)"),
            arrayOf("ptbr", "Português(Brasil)"),
            arrayOf("ptpt", "‪Português(Portugal)"),
            arrayOf("roro", "Română(România)"),
            arrayOf("ruru", "Русский(Россия)"),
            arrayOf("silk", "සිංහල(ශ්රී ලංකාව)"),
            arrayOf("sksk", "Slovenčina(Slovensko)"),
            arrayOf("slsi", "Slovenščina(Slovenija)"),
            arrayOf("srrs", "Српски(Србија)"),
            arrayOf("suid", "Urang(Indonesia)"),
            arrayOf("svse", "Svenska(Sverige)"),
            arrayOf("swke", "Swahili(Kenya)"),
            arrayOf("swtz", "Swahili(Tanzania)"),
            arrayOf("tain", "தமிழ்(இந்தியா)"),
            arrayOf("talk", "தமிழ்(இலங்கை)"),
            arrayOf("tamy", "தமிழ்(மலேசியா)"),
            arrayOf("tasg", "தமிழ்(சிங்கப்பூர்)"),
            arrayOf("tein", "తెలుగు(భారతదేశం)"),
            arrayOf("thth", "ไทย(ประเทศไทย)"),
            arrayOf("trtr", "Türkçe(Türkiye)"),
            arrayOf("ukua", "Українська(Україна)"),
            arrayOf("urin", "اردو (بھارت)"),
            arrayOf("urpk", "اردو (پاکستان)"),
            arrayOf("vivn", "Tiếng Việt(Việt Nam)"),
            arrayOf("yuehnthk", "廣東話(香港)"),
            arrayOf("zhcn", "普通话(中国大陆)"),
            arrayOf("zhhk", "普通話(香港)"),
            arrayOf("zhtw", "國語(台灣)"),
            arrayOf("zuza", "IsiZulu(Ningizimu Afrika)")
        )
        @JvmField var loginType = arrayOf(
            //arrayOf("S", "student"),
            //arrayOf("T", "teacher"),
            //arrayOf("P", "professor"),
            arrayOf("E", "employee"),
            //arrayOf("M", "manager")
        )
        @JvmField var sensorType = arrayOf(
            arrayOf("b", "Beacon"),
            arrayOf("r", "RFID"),
            arrayOf("q", "QRCode"),
        )
        @JvmField var timeZone = arrayOf(
            arrayOf("1"  ,"Afghanistan Standard Time"),
            arrayOf("2"  ,"Alaskan Standard Time"),
            arrayOf("3"  ,"Aleutian Standard Time"),
            arrayOf("4"  ,"Altai Standard Time"),
            arrayOf("5"  ,"Arab Standard Time"),
            arrayOf("6"  ,"Arabian Standard Time"),
            arrayOf("7"  ,"Arabic Standard Time"),
            arrayOf("8"  ,"Argentina Standard Time"),
            arrayOf("9"  ,"Astrakhan Standard Time"),
            arrayOf("10" ,"Atlantic Standard Time"),
            arrayOf("11" ,"AUS Central Standard Time"),
            arrayOf("12" ,"Aus Central W. Standard Time"),
            arrayOf("13" ,"AUS Eastern Standard Time"),
            arrayOf("14" ,"Azerbaijan Standard Time"),
            arrayOf("15" ,"Azores Standard Time"),
            arrayOf("16" ,"Bahia Standard Time"),
            arrayOf("17" ,"Bangladesh Standard Time"),
            arrayOf("18" ,"Belarus Standard Time"),
            arrayOf("19" ,"Bougainville Standard Time"),
            arrayOf("20" ,"Canada Central Standard Time"),
            arrayOf("21" ,"Cape Verde Standard Time"),
            arrayOf("22" ,"Caucasus Standard Time"),
            arrayOf("23" ,"Cen. Australia Standard Time"),
            arrayOf("24" ,"Central America Standard Time"),
            arrayOf("25" ,"Central Asia Standard Time"),
            arrayOf("26" ,"Central Brazilian Standard Time"),
            arrayOf("27" ,"Central Europe Standard Time"),
            arrayOf("28" ,"Central European Standard Time"),
            arrayOf("29" ,"Central Pacific Standard Time"),
            arrayOf("30" ,"Central Standard Time"),
            arrayOf("31" ,"Central Standard Time (Mexico)"),
            arrayOf("32" ,"Chatham Islands Standard Time"),
            arrayOf("33" ,"China Standard Time"),
            arrayOf("34" ,"Cuba Standard Time"),
            arrayOf("35" ,"Dateline Standard Time"),
            arrayOf("36" ,"E. Africa Standard Time"),
            arrayOf("37" ,"E. Australia Standard Time"),
            arrayOf("38" ,"E. Europe Standard Time"),
            arrayOf("39" ,"E. South America Standard Time"),
            arrayOf("40" ,"Easter Island Standard Time"),
            arrayOf("41" ,"Eastern Standard Time"),
            arrayOf("42" ,"Eastern Standard Time (Mexico)"),
            arrayOf("43" ,"Egypt Standard Time"),
            arrayOf("44" ,"Ekaterinburg Standard Time"),
            arrayOf("45" ,"Fiji Standard Time"),
            arrayOf("46" ,"FLE Standard Time"),
            arrayOf("47" ,"Georgian Standard Time"),
            arrayOf("48" ,"GMT Standard Time"),
            arrayOf("49" ,"Greenland Standard Time"),
            arrayOf("50" ,"Greenwich Standard Time"),
            arrayOf("51" ,"GTB Standard Time"),
            arrayOf("52" ,"Haiti Standard Time"),
            arrayOf("53" ,"Hawaiian Standard Time"),
            arrayOf("54" ,"India Standard Time"),
            arrayOf("55" ,"Iran Standard Time"),
            arrayOf("56" ,"Israel Standard Time"),
            arrayOf("57" ,"Jordan Standard Time"),
            arrayOf("58" ,"Kaliningrad Standard Time"),
            arrayOf("59" ,"Kamchatka Standard Time"),
            arrayOf("60" ,"Korea Standard Time"),
            arrayOf("61" ,"Libya Standard Time"),
            arrayOf("62" ,"Line Islands Standard Time"),
            arrayOf("63" ,"Lord Howe Standard Time"),
            arrayOf("64" ,"Magadan Standard Time"),
            arrayOf("65" ,"Magallanes Standard Time"),
            arrayOf("66" ,"Marquesas Standard Time"),
            arrayOf("67" ,"Mauritius Standard Time"),
            arrayOf("68" ,"Mid-Atlantic Standard Time"),
            arrayOf("69" ,"Middle East Standard Time"),
            arrayOf("70" ,"Montevideo Standard Time"),
            arrayOf("71" ,"Morocco Standard Time"),
            arrayOf("72" ,"Mountain Standard Time"),
            arrayOf("73" ,"Mountain Standard Time (Mexico)"),
            arrayOf("74" ,"Myanmar Standard Time"),
            arrayOf("75" ,"N. Central Asia Standard Time"),
            arrayOf("76" ,"Namibia Standard Time"),
            arrayOf("77" ,"Nepal Standard Time"),
            arrayOf("78" ,"New Zealand Standard Time"),
            arrayOf("79" ,"Newfoundland Standard Time"),
            arrayOf("80" ,"Norfolk Standard Time"),
            arrayOf("81" ,"North Asia East Standard Time"),
            arrayOf("82" ,"North Asia Standard Time"),
            arrayOf("83" ,"North Korea Standard Time"),
            arrayOf("84" ,"Omsk Standard Time"),
            arrayOf("85" ,"Pacific SA Standard Time"),
            arrayOf("86" ,"Pacific Standard Time"),
            arrayOf("87" ,"Pacific Standard Time (Mexico)"),
            arrayOf("88" ,"Pakistan Standard Time"),
            arrayOf("89" ,"Paraguay Standard Time"),
            arrayOf("90" ,"Qyzylorda Standard Time"),
            arrayOf("91" ,"Romance Standard Time"),
            arrayOf("92" ,"Russia Time Zone 10"),
            arrayOf("93" ,"Russia Time Zone 11"),
            arrayOf("94" ,"Russia Time Zone 3"),
            arrayOf("95" ,"Russian Standard Time"),
            arrayOf("96" ,"SA Eastern Standard Time"),
            arrayOf("97" ,"SA Pacific Standard Time"),
            arrayOf("98" ,"SA Western Standard Time"),
            arrayOf("99" ,"Saint Pierre Standard Time"),
            arrayOf("100","Sakhalin Standard Time"),
            arrayOf("101","Samoa Standard Time"),
            arrayOf("102","Sao Tome Standard Time"),
            arrayOf("103","Saratov Standard Time"),
            arrayOf("104","SE Asia Standard Time"),
            arrayOf("105","Singapore Standard Time"),
            arrayOf("106","South Africa Standard Time"),
            arrayOf("107","South Sudan Standard Time"),
            arrayOf("108","Sri Lanka Standard Time"),
            arrayOf("109","Sudan Standard Time"),
            arrayOf("110","Syria Standard Time"),
            arrayOf("111","Taipei Standard Time"),
            arrayOf("112","Tasmania Standard Time"),
            arrayOf("113","Tocantins Standard Time"),
            arrayOf("114","Tokyo Standard Time"),
            arrayOf("115","Tomsk Standard Time"),
            arrayOf("116","Tonga Standard Time"),
            arrayOf("117","Transbaikal Standard Time"),
            arrayOf("118","Turkey Standard Time"),
            arrayOf("119","Turks And Caicos Standard Time"),
            arrayOf("120","Ulaanbaatar Standard Time"),
            arrayOf("121","US Eastern Standard Time"),
            arrayOf("122","US Mountain Standard Time"),
            arrayOf("123","UTC"),
            arrayOf("124","UTC+12"),
            arrayOf("125","UTC+13"),
            arrayOf("126","UTC-02"),
            arrayOf("127","UTC-08"),
            arrayOf("128","UTC-09"),
            arrayOf("129","UTC-11"),
            arrayOf("130","Venezuela Standard Time"),
            arrayOf("131","Vladivostok Standard Time"),
            arrayOf("132","Volgograd Standard Time"),
            arrayOf("133","W. Australia Standard Time"),
            arrayOf("134","W. Central Africa Standard Time"),
            arrayOf("135","W. Europe Standard Time"),
            arrayOf("136","W. Mongolia Standard Time"),
            arrayOf("137","West Asia Standard Time"),
            arrayOf("138","West Bank Standard Time"),
            arrayOf("139","West Pacific Standard Time"),
            arrayOf("140","Yakutsk Standard Time"),
            arrayOf("141","Yukon Standard Time")
        )
        @JvmStatic lateinit var apInfoArray : Array<APInfo>
        @JvmStatic var clientInfoArray : Array<ClientInfo?>? = null
        @JvmStatic var gatepositionArray : Array<SimpleGatePosition?>? = null

        @JvmStatic lateinit var colorStateList : ColorStateList
        @JvmStatic
        inline fun <reified T> makePartialArray(input : Array<Array<T>>, posPartial : Int) : MutableList<T>
        {
            val outArray = mutableListOf<T>()

            for(i in input.indices)
                outArray.add(input[i][posPartial])

            return outArray
        }
        @JvmStatic
        fun IP2Bytearray(ipstr:String) : ByteArray
        {
            var strs = ipstr.split(".")
            if(strs.size != 4) throw InputMismatchException("Invalid IP Address.") // TODO : String Conversion

            return ByteArray(4) { i -> if(strs[i].toInt()>127) (strs[i].toInt()-256).toByte() else strs[i].toByte() }
        }
        @JvmStatic
        fun Bytearray2IP(ipstr:ByteArray): String {
            val ip1 = if(ipstr[0].toInt() < 0) 256+ipstr[0].toInt() else ipstr[0].toInt()
            val ip2 = if(ipstr[1].toInt() < 0) 256+ipstr[1].toInt() else ipstr[1].toInt()
            val ip3 = if(ipstr[2].toInt() < 0) 256+ipstr[2].toInt() else ipstr[2].toInt()
            val ip4 = if(ipstr[3].toInt() < 0) 256+ipstr[3].toInt() else ipstr[3].toInt()

            if(ip1 + ip2 + ip3 + ip4 == 0)return ""
            else return format("%d.%d.%d.%d",ip1,ip2,ip3,ip4)
        }
    }
    //"ty":"usr", , "idx":"16", "nm":"박**", "lastdt":"2021-06-09 06:50:12", "st":"v_휴가", "temp":"온도값"
    class ItemTotalStatus {
        var type: String? = null
        var idx: Int? = null
        var nm: String? = null
        var lastdt: String? = null
        var st: String? = null
        var temp: String? = null
    }
    class SimpleGatePosition {
        var time: String? = null
        var io_type: String? = null
        var gatename: String? = null
        var clientcode: String? = null
    }
    class APInfo {
        var apname : String? = null
        var idx : Int? = null //idxEquip
        var dist : String? = null
        var useyn : String? = null //사용 여부(Y/N)
        var flg : String? = null //상태. 0,1,2,3
        var ip : String? = null //20글자 Base-64ed text
        var sty : String? = null //센서타입(BEACON,...)
        var gps : String? = null //gps값 "위도,경도"
        var useio : String? = null //?
        var openlock : Int? = null //문열기 여부(1/0)
        var tz : Int? = null //timezone(code번호로)
    }
    class ClientInfo {
        var clientCode: String? = null
        var clientName: String? = null
        var clientLocation: String? = null
    }

}