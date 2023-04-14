package com.example.myfirstkotlin;

import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class AES256Util {
    private byte[] iv;
    private byte[] key;
    private Key keySpec;

    public AES256Util(byte[] iv, byte[] key)
            throws UnsupportedEncodingException
    {
        this.iv = iv;
        this.key = key;

        byte[] arr_key = new byte[32];

        System.arraycopy(key, 0, arr_key, 0, 32);
        SecretKeySpec keySpec = new SecretKeySpec(arr_key, "AES");
        this.keySpec = keySpec;
    } // 암호화

    public byte[] encode(byte[] input)
            throws UnsupportedEncodingException,
            NoSuchAlgorithmException,
            NoSuchPaddingException,
            InvalidKeyException,
            InvalidAlgorithmParameterException,
            IllegalBlockSizeException,
            BadPaddingException
    {
        Log.d("AES256","encode start!!");
        Cipher c = Cipher.getInstance("AES/CBC/PKCS5Padding");

        c.init(Cipher.ENCRYPT_MODE, keySpec, new IvParameterSpec(iv));
        byte[] encrypted = c.doFinal(input);
        //Log.d("AES256","input ["+input.length+"]: " + BytesToString(input));
        //Log.d("AES256","output["+encrypted .length+"]: " + BytesToString(encrypted));
        return encrypted;
    }
    //복호화
    public byte[] decode(byte[] input)
            throws UnsupportedEncodingException,
            NoSuchAlgorithmException,
            NoSuchPaddingException,
            InvalidKeyException,
            InvalidAlgorithmParameterException,
            IllegalBlockSizeException,
            BadPaddingException
    {
        Log.d("AES256","decode start!!");
        Cipher c = Cipher.getInstance("AES/CBC/PKCS5Padding");
        c.init(Cipher.DECRYPT_MODE, keySpec, new IvParameterSpec(iv));
        byte[] byteStr = c.doFinal(input);

        //Log.d("AES256","input ["+input.length+"]: " + BytesToString(input));
        //Log.d("AES256","output["+byteStr .length+"]: " + BytesToString(byteStr));

        return byteStr;
    }
    String BytesToString(byte[] in)
    {
        StringBuilder out = new StringBuilder();
        for (int i=0; i<in.length; i++)
            out.append(String.format("%02X ", in[i]));

        return out.toString();
    }

    //출처: https://shary1012.tistory.com/224 [샤리의 인디코딩]

}
