package com.example.inout_nodeapp;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public class APIClass extends Thread{
    static MyService parent;
    Handler mMainHandler;

    String URLPage;
    String Data;
    String URLHost;
    String reply;
    JSONObject replyOBJ;
    JSONArray replyArr;
    static String TaskName = "";
    static String privTaskName = "";


    public static final int RESULT_OK           = 200;
    public static final int RESULT_FAILED       = 201;
    public static final int RESULT_INSTREAM_ERR = 202;
    public static final int RESULT_URLCONN_FAIL = 203;
    public static final int RESULT_JSON_FAIL    = 204;
    public static final int RESULT_WRONG_NUMBER = 205;
    public static final int RESULT_NONE         = 206;

    APIClass(MyService Argparent)
    {
        parent = Argparent;
        mMainHandler = parent.mHandler;
    }

    @Override
    public void run() {
        Log.d(TaskName,"SendURL : " + URLHost + URLPage + Data);
        int res = GetJSONReply(URLHost + URLPage, Data);
        if(!errorCheck(res))return;

        try {
            replyArr = GetJSONArrFromRtn(reply);
            replyOBJ = replyArr.getJSONObject(0);
        } catch (JSONException e) {
            System.err.println(EndedinFail(RESULT_JSON_FAIL));
            e.printStackTrace();
            errorCheck(RESULT_JSON_FAIL);
            return;
        }

        Message msg = new Message();
        msg.what = 2; msg.obj = URLPage;
        mMainHandler.sendMessage(msg);
    }
    boolean errorCheck(int res)
    {
        Message msg = new Message();
        if(res != RESULT_OK){
            msg.what = 1; msg.arg1 = res;
        }else{
            return true;
        }
        mMainHandler.sendMessage(msg);
        return false;
    }

    int GetJSONReply(String getURL,String postdata)
    {
        InputStream in;

        try {
            in = getStreamFromURL(getURL+postdata);
        } catch (IOException e) {
            System.err.println(EndedinFail(RESULT_URLCONN_FAIL));
            e.printStackTrace();
            return RESULT_URLCONN_FAIL;
        }
        try {
            reply = getStrFromStream(in);
        } catch (IOException e) {
            System.err.println(EndedinFail(RESULT_URLCONN_FAIL));
            e.printStackTrace();
            return RESULT_URLCONN_FAIL;
        }
        return RESULT_OK;
    }
    JSONArray GetJSONArrFromRtn(String Return) throws JSONException {
        String jsonstr = Return.substring(1, Return.length()-1).replace("\\\"","\"");

        return new JSONArray(jsonstr);
    }

    public InputStream getStreamFromURL(String strUrl) throws IOException {
        return new BufferedInputStream(new URL(strUrl).openStream());
    }
    public InputStream getStreamFromURL(String strUrl,String postdata) throws IOException {
        URL url = new URL(strUrl);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setRequestProperty("content-type", "application/x-www-form-urlencoded");
        connection.setRequestMethod("POST");
        connection.setDoInput(true);
        connection.setDoOutput(true);
        connection.setUseCaches(false);
        connection.setConnectTimeout(15000);

        OutputStreamWriter wr = new OutputStreamWriter(connection.getOutputStream());
        wr.write(postdata);
        wr.flush();
        wr.close();
        //출처: https://kwon8999.tistory.com/entry/HttpURLConnection-SampleGet-Post [Kwon's developer]
        int resp = connection.getResponseCode();

        return new BufferedInputStream(connection.getInputStream());
    }
    public String getStrFromStream(InputStream in) throws IOException {
        String line;
        StringBuilder sb = new StringBuilder();
        BufferedReader br = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));

        while((line = br.readLine()) != null)
            sb.append(line);

        br.close();
        if(sb.length() == 0)return "";

        return sb.toString();
    }

    public static String EndedinFail(int arg)
    {
        String msg = "";
        switch(arg)
        {
            case RESULT_OK: msg = "Success";break;//
            case RESULT_INSTREAM_ERR: msg = "Network failed." ;break;
            case RESULT_URLCONN_FAIL: msg = "Connecting to Server failed.";break;//
            case RESULT_JSON_FAIL: msg = "Wrong Return data";break;//
            case RESULT_WRONG_NUMBER: msg = "Wrong Number.";break;//
        }
        return msg;
    }
    public static boolean isInternetFail(int res)
    {
        return res == RESULT_INSTREAM_ERR || res == RESULT_URLCONN_FAIL;
    }
}
