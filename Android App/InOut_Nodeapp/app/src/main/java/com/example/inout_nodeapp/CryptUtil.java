package com.example.inout_nodeapp;

import android.util.Base64;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class CryptUtil {
    AES256Util AESCode;

    public void start() {
        Log.d("CryptUtil","start!!");

        int no  = 17;
        byte[] datas = Base64.decode(Base64.decode("WlJ2QmVKbkw5NkdBZExpR1JFK2pGUT09",0),0);
        String hp = AESDecode(datas,no);
        byte[] ddata = AESEncode(hp,no);
        String dataold = new String(Base64Encode(Base64Encode(ddata,0),0));

        Log.d("CryptUtil","data ["+datas.length+"]: " + BytesToString(datas));
        Log.d("CryptUtil","hp     : " + hp);
        Log.d("CryptUtil","ddata["+ddata.length+"]: " + BytesToString(ddata));
        Log.d("CryptUtil","dold   : " + dataold);

    }
    static byte[] Base64Encode(byte[] data,int flag)
    {
        byte[] encoded64 = Base64.encode(data,flag);

        if(encoded64.length<=1)return encoded64;

        int focus;
        byte[] encoded;

        for(focus=0; focus<encoded64.length;) {

            for (focus = 0; focus < encoded64.length; focus++) {
                if (encoded64[focus] == 10) break;
                if (focus > encoded64.length - 1) focus = -1;
            }
            if (focus == -1) return encoded64;

            encoded = new byte[encoded64.length - 1];

            System.arraycopy(encoded64, 0, encoded, 0, focus);
            System.arraycopy(encoded64, focus + 1, encoded, focus, encoded64.length - focus - 1);

            encoded64 = encoded;
        }
        return encoded64;
    }

    byte[] AESEncode(String str,int code)
    {
        byte[] key,iv;
        try {
            key = Base64.decode(AESKeyIVs[code-1][0], 0);
            iv = Base64.decode(AESKeyIVs[code-1][1], 0);
        }catch(ArrayIndexOutOfBoundsException e){ return new byte[0]; }

        try {
            AESCode = new AES256Util(iv, key);
        } catch (UnsupportedEncodingException e) { e.printStackTrace(); return new byte[0];}
        return AESEncode(str.getBytes());
    }
    byte[] AESEncode(byte[] data,int code)
    {
        byte[] key,iv;
        try {
            key = Base64.decode(AESKeyIVs[code-1][0], 0);
            iv = Base64.decode(AESKeyIVs[code-1][1], 0);
        }catch(ArrayIndexOutOfBoundsException e){ return new byte[0]; }

        try {
            AESCode = new AES256Util(iv, key);
        } catch (UnsupportedEncodingException e) { e.printStackTrace(); return new byte[0];}
        return AESEncode(data);
    }
    String AESDecode(byte[] str,int code)
    {
        byte[] key,iv;
        try {
            key = Base64.decode(AESKeyIVs[code-1][0], 0);
            iv = Base64.decode(AESKeyIVs[code-1][1], 0);
        }catch(ArrayIndexOutOfBoundsException e){ return "Invaild Numcode."; }

        try {
            AESCode = new AES256Util(iv, key);
        } catch (UnsupportedEncodingException e) { e.printStackTrace(); return "";}

        return new String(AESDecode(str));
    }

    byte[] AESEncode(byte[] in)   {
        try {
            return AESCode.encode(in);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (NoSuchPaddingException e) {
            e.printStackTrace();
        } catch (InvalidKeyException e) {
            e.printStackTrace();
        } catch (InvalidAlgorithmParameterException e) {
            e.printStackTrace();
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }
        return new byte[0];
    }
    byte[] AESDecode(byte[] in)   {
        try {
            return AESCode.decode(in);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (NoSuchPaddingException e) {
            e.printStackTrace();
        } catch (InvalidKeyException e) {
            e.printStackTrace();
        } catch (InvalidAlgorithmParameterException e) {
            e.printStackTrace();
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }
        return new byte[0];
    }

    static String BytesToString(byte[] in)
    {
        StringBuilder out = new StringBuilder();
        for (int i=0; i<in.length; i++)
            out.append(String.format("%02X ", in[i]));

        return out.toString();
    }

    String[][] AESKeyIVs = {
//            {"RnylrE0lNr8yf0YAzUtVskJS9KqjUbhZCougPCwP8A8=", "5kKMVwg8LS1Xw7DmO79S8A=="},
//            {"6u7vXieaa3Gtn0Uxqh2zaXCWfZbEPTVq27cZdLPruv4=", "bwqcL8ehp1kEHyTv8WOUgg=="},
//            {"5NkTYUbbcAsDwcXWt0Ti2uTB4S80u9Ze2gpVAHC7Drk=", "IA2a3t4uiNXJ40rvA0jQbQ=="},
//            {"4sKIzzmAIcUAvSJ5CFpJRRxfLmbKLJZcXrel42y5lPA=", "r1dn567ZWSwDVKDheM7shA=="},
//            {"98DH4ePvLpPDDh729zCOTcmIPFZaOfIYn6RdUPyoETU=", "cmwiVEeg3WPH0JHN0GvXeQ=="},
//            {"ad39MZJGtKkk1MjKyUybbPGuuYPRhVhivFNS26jHQPE=", "rptkpmDtKyjaW18mCrikVw=="},
//            {"JncdkS0l086QUZWO00rSYSoNjTfYyLsfbyB2HftlJXY=", "5RWw7BH7jOdoZQWHabG5RA=="},
//            {"vrAB0MVfJKtDn0IlGqUsRND2e3AbcO4pw0vXIlUc6yY=", "aCjcMbCxO0iN0QWFwLia3w=="},
//            {"0mJJ0F0NS3xwPgslHSZtfpAlDC6GgO0m6AtCBPQZpgU=", "DfQt6sBYgZk0zW05yo4OJA=="},
//            {"6wfUH2wAAcGiSXGh0zBNd609pXNGZrdpge3hCwJQCRc=", "mSgtamCSm617j8BHHxQypg=="},
//            {"j90pYk2BI0aQJ9r8x9aV9pJ8v2XK21yJnD0Xj5nJbD0=", "ppr3HGEE4KPZGi4llFSSTQ=="},
//            {"EGuRiPDwgiN77T0jeT5nw8TeFmRTJJInc7fKjQQomvM=", "ObYvF0IncDrhGnLvOTXh6w=="},
//            {"k5eSgIjP5Ejfy75IEjxbYYeDTuiNobSyGkmGSS07Q3o=", "JvE77X7WoLOD6DtZoT56JA=="},
//            {"406w1GFk0txXRM4kQtEDzRIv3cEBawZPMkAzjTRs8LA=", "rASbDloqZ26U5Ia6KvC3AA=="},
//            {"02HLviNTwQ8JkKslEwcvGJaxE26qjjJ0MBJF8GBNS9U=", "bkC0x2yjqX0DJOmKgKdR3g=="},
//            {"NXyYRjXtS1JQ0kdq1wwS9ZC3PJv0ErbgIGPgMPPBgeA=", "e5840tZxGbnqimINUrskkA=="},
//            {"9M5C40xj5to85JEB7KUB33CV0R1zdnzYjciUTyXyIuc=", "bbCYCYmAwR1DJCiBgAxmPA=="},
//            {"iI0IVH0jXit1PtdGY5DYIv7twIayHLxiIFLHkNBHxzk=", "2oCSjcp1mbcljQ7J0nO6xg=="},
//            {"X7asxJcFBvWX6Z1den0Qap4DPeWerPbl50kTK6tLwuo=", "SC6le4tw9XN74dxU9i07kg=="},
//            {"Lnd80vYgvVinb2G9xeTW6QXtOl4GJW5at0CgnzcIt48=", "v14cTy5EjSQv2TYtC3YJxg=="},
//            {"pVtiRquV67OMuYliPkPWQ3YQOlLxvyH4wgbSiy6rDAE=", "KJcwGDtJP0joiOqvUt8k1A=="},
//            {"DzlqcUP4b0GYRdu7G06shPHoAQhBXfblMGPgVj8HizY=", "EAMs71zx6O2B5IGESwqMXA=="},
//            {"bjl1CX2PqNHqN0I0ClLu7noD4dB9CdW4zL30WRzQyJ0=", "kYlh5jKNhqc7PEJwM96CRg=="},
//            {"ttwdz6ugWlfCYUMfmczDw0AeaZtqQKAmq6glqoGNP2g=", "JVxCCgSoQv1yL38OL6kWEQ=="},
//            {"wgmPDvNcaBSuN5xss5DofmXcIbKdheoLVo7gwqwsm2w=", "E7YBuPyZz5M1XDpqlj721w=="},
//            {"NPoN5tgYMtPCNnarlZDE0U9X5mCscd3L4SQ0eH23BtQ=", "N7FiahySIWXKW2RmKKdZ7w=="},
//            {"BlAQwh1nH4VWkshiMunEXNpyPgodObfU0O8A8SeD3HM=", "4cM3yVky0Tk9XGwgOFt0qw=="},
//            {"wIJyNdvPBPADmcayPbYOEN9sxl1VeaFT6dJb4OhUA2Y=", "TghokqzOEePhjZmNEsBeHA=="},
//            {"pVGcXnfiFF6c1np9nqdRXjAfl83Jxsu164NZm2arAbw=", "Ti75F94a5bzL07rzD3g0FA=="},
//            {"BlDZbyFMWz0P5YrX0YDyuuMZtUVYFkRV8xMgiDN6dH4=", "oHDLVq13LJ35LZOgEVuKeA=="}


            {"NW17vJm8kcYjjLjoxYfjG59hyNiTYeFpwAcjFgxbktk=", "+3+rPBLucPPHVjnUgFZUSw=="},
            {"ugT+lkDXCY/wAhSFe6P1NrhwMWZhJpNWhaP/8+/dSB8=", "4srjGzJV8oOCJRis0Clztw=="},
            {"UVnfmVzZhHe0l1FfY/5InhJ1k29RYp/jkS0cj2Be0fE=", "3u5pPxZzP9BsfqvbygDKAw=="},
            {"VrVdqEZE6RwEYH6wZaydLNeNzobpRQeBpxlbM90zIks=", "Zfs2mM782D+PO2CemOyuYw=="},
            {"98oFEi9rByZb+5xmjpSyq27EdysmcL2e5MAIf6sZ+t8=", "fETC/3KBGJtzkc3a5CazAg=="},
            {"BecAXGQ2SG8wzcqnj+RKD/W/dYf7zssTb5YgxqAfrIw=", "vA9xbA1cJ9+u2AYnfMs64g=="},
            {"QzX88uRtflKuvey17lkfVzyJ45NlbUYFAgfsU8UoTKI=", "SjEwkI3v6M1MRg4tK9aO0Q=="},
            {"As8uZZkehQoVLUMOrLT4aacAmSZcfC/cQ48ihS8Pnv8=", "nkCGEDcbY3EiA1cHVlyZBg=="},
            {"Qy9Ot1Pjnh4gkGK+Y99QQnU8dPPG53Qcfffkc1LNNwo=", "bD6ApHSIjzz2aU2vezIp6g=="},
            {"Mw3pyEzjq4KfqW7LzR9E294A8bi7n2bxlQILKLVgAKc=", "Ckwe8wNSTcKELXstRVVoOg=="},
            {"MrozIT8dsjX872g76atHHBuDXJxn4Cq1sQ1YUy0JFas=", "hzFHxhP/Q5yEo0cwPSPX+A=="},
            {"b2KAs83mkmCobvYgYEXkglLqxhQZXazaVUuHfFt2IKo=", "6cIGHQWAYj3TG4XBg/Qa4g=="},
            {"d1Ikq7dNTT9A8yHImTJTwCzp+/wcAxXpkBMEwr0gQCE=", "p4u41v7FXIlJ96EJYlBFig=="},
            {"ZIz9qeQIk9b3y69i/MGYWILiIOXUB1SBDsUfVQFg3vM=", "Cy7QK9aYr5xgm3wAbX7q7w=="},
            {"OlfO9Yf6T8XCs6Br6Os/5v1MT8A9iCTWnFZK9Al4RkE=", "pdadbWS0Iu9re2bCYe+qoA=="},
            {"gK1ecP2BH10KWGZLrWYXR8ccuyOCXExbHK4SD4sBO3I=", "Cx/FaAdiCkhXhPlIX0z7tA=="},
            {"vEjQX6mQc1c2LUFMddGVtrUp10wOfr5v5f9yL9ONf38=", "JzkYCvFXx4Pas3LzsfuYWQ=="},
            {"s7TAVQk8CofUFftr9h8P+Xo7dVez3GStVtCR/PNTyOo=", "QqgRT8/a3SNSl2lylVb3tA=="},
            {"6BWIQS2Sn6mMUi1YnygPQ987WVc93PE3CAuFQ9+CciY=", "R+UuYynmO+9+NKUiSMZ8ng=="},
            {"AMn68RiJxuySr06J/6ZGpd4aSdiAjX98nwu8kKklC0U=", "dTiYHMp6tiAKd/t+pEDeTg=="},
            {"pCDh2dE1GxZyBZMiDYBNugIpYOAD9BYLSTFbvg/LiAk=", "seWkQYt6P5jyjB07NFgpKw=="},
            {"VzkUBYVthRWj5e5PbgLBGMWIM1tvZpSbPQ3kS6TzA5w=", "aCH0wsAY4z7WT9phtVABjw=="},
            {"4mBkTZKPhsN5kEDxgjcgGv0EkGtOqlTbxN/97VbEpWo=", "vrwNKirXkwVuBW7mRMyISw=="},
            {"gJpRas7a1RsVFautMDJ/c1NIXwxCGEYo0+TxxEY2k2E=", "TCf3jeFt+VgBiXigtlaj2A=="},
            {"HoGnCQXNe0C2d/REyfa/liDSo13PE8mkJSPDOt7HL9w=", "+tJYy4ASZCgf3NlBbmChvg=="},
            {"QfMF6xh5u6gsOpGehAb/nvpILHFey/uL9q7ylFdiV7g=", "ZChoXN/cTY52lj3BMhPV/g=="},
            {"Nbo/dIqF9J4AzdaestgEgtsxnyECCswN93fSgOXD11E=", "a5mkjpWks02zE/7M/w1XSw=="},
            {"12n8jZ3uYAKxhQYPGgmcI7Tdpmqu9yWv14MJjwDuajk=", "8+rnqpDO3/cbiJbXchrNDw=="},
            {"i5TelI5znhUG5XX4W7/nh4PuqgMciX/clWFTdMcC/EE=", "xghJuEOha866qE6FTrpruw=="},
            {"WyJB6TyG+E032/eeXUi2cBpQep8YzoAvTJdBX5ayaQM=", "/KYQnL6TraKj9gCARK20gw=="},
            {"l4IDRQWo/YbI8sPiyUW/GZofqAfkPso+l09U/R2mExA=", "rHJ2BKmyoACn72vQYX04dA=="},
            {"mPNlwB08N0N+S8EqItLAzCqrh/f05xbhuAFJzse1bsQ=", "lwhX8zaixYEbWsHNWJspXg=="},
            {"RBvcQQyMKGQdVADy9QWG92gxDrVmDb0o6Gxd7EEl6Ls=", "meZ1ffOkzjvicJDOUo7cyA=="},
            {"5uP1D7d9w0TRrHWla+VUp8yU/Lv6R0q74rcBc+lQEQ8=", "vM3mJ1IRMn3oV1NlUZloOQ=="},
            {"fqcgNG9AeRN2zvMWWwNNJ2xkNiZuiAZHZdH1aRzCLsk=", "bo/GQU/Ix1Ugz4IIK4ZASQ=="},
            {"sLHYgh1hm2J5Sgeh4GkxboG+5/NpiCHFvDhwvJCH464=", "Ps98fe4rn/4/bsijg+oNZw=="},
            {"GbXkjk4ogbS0hm/nJCD+FYwzzSfortCWpOqhvieZIDE=", "4laPJU+CNIrjJVQ3dASBeA=="},
            {"PPcRGDrQBaMAHpjlmjsB0lzvDZNU90L0MfBp/Uo+68w=", "0ZhBqTZjOpuMBi84ZaH2tg=="},
            {"isNpdApJDd9DflXEkEb5k8+9JvVAakGRMxpj0ZTxtWo=", "2YD6sJY2zKhmDAtfDZx6jQ=="},
            {"OZWo27zAn+kF4Q22nOEPcrbd2O8IB16O3/EEg67srhk=", "j9G1ImVCqi1+giK0ob/bng=="},
            {"LkYQWYOLPcgonMdPLSRYg+fGGUtXL61g/K2fUeMnVF0=", "Z65ynbixF30YfRwb3QXSkA=="},
            {"9t28O/1RpmtlS0IE6seGEFvWVc7EuWFwjhkhLxJQgvA=", "PSWJYHTwGlJyqOjWOXhJhQ=="},
            {"UKdkFFKs3J5zx4YHoeFPgi7nadHr8b4MYrvf4WdPVdQ=", "MvopxwY9VnrooEvIki+SVw=="},
            {"gyHBxulMI4H7tGDKws5aPTZrJAe5iyxG/mHctFcv0SA=", "G/JGh1RRHwrljnif2mXApA=="},
            {"NKM9dybd7I2ioWiILQpPamlv5yyuIeBLyCBgetkhQg4=", "qvbySK8qqMLsylM7PlskYA=="},
            {"tOCjwQMkbH8RCv15HriHKXxgfHY1YOUUN58WePS1YJY=", "SrJSyE1C6UzqMosBf++Hzg=="},
            {"mPeT0VDSz494TWV5dmc9PMaB1dtWczmX9Zw5OlxGAyk=", "bm9qfJ8Wadoo+wdGLQ1s1w=="},
            {"GLZxyCdrRTmyHAyFyx6WBgm9Fh8Y/0rMBiJnmmpTN1g=", "5dgrR83j2TQUAK4mSBq1tA=="},
            {"NQeJSivKONOcBr9P9FUzaaHCz9IEyl6U5g7PqPUjzlo=", "1oWIlE+gZNROK8/uA2FRgQ=="},
            {"ZzCjwB5D39c3n9Ty25xILvSK70p2pI9hFww+WFdX6So=", "MLDGO8veQlg1geBtxjP9jg=="},
            {"pQrlfNFNl4tGMX/QHtKWVea7hkA08i1nkT3xqg69O0g=", "XZM+gapMAfdOZi0JvkonNA=="},
            {"C+OGQnG3x/pQlpITFKxCv+5gUkP3kis3smTux3wt4DA=", "hw/hPrmQf2pMJtHIQHH/pg=="},
            {"pFqvm0rUHTx90CuGACyX9cX0SPXBLlRoY9BnA+g2gQ0=", "AFy6/YFP4t5NgPuEbPU4Uw=="},
            {"HB5IGyvm8O7LsH/MsS/UN+LFYcpHlVZMX21cOw/gC4U=", "5nvSQ98e8w8Puwamkey+MA=="},
            {"9t2uBKKZDw+hs4oc+JSZtDZPDKTWurdSvgn1efJqhIo=", "GM74JwoXejjD0wR3PodhZg=="},
            {"IiNSG3zvBQelfRTZggp5/DC/LS2+h8x2ys4eMhX0rXQ=", "GufE+NuH412EMXkQBpFNZg=="},
            {"N0hz9k2iz6uqm4aDioQjChfrwyZEmzlRPSi8/ZYlNac=", "QWuDz2Bq/J7gL7p7cE/bSg=="},
            {"epRFFhunsXjDIA9X4N9r/flAe+QBNMRjC2nR7Qp2uVQ=", "6R53H/gguGOhPgmxvAf3NA=="},
            {"dpwIPTVkyalvQfilGYzCvRRIq9LSH8SDH7qF0zx5N5g=", "sc5CO3eQiKbMYnsFWfwb4g=="},
            {"guAarl7r///aGe8Du2MvveIBjUqaKCImpVB0XxQ9hE4=", "sHhw3cbemSA5JXOtt5w1dg=="},
            {"YIe8anJzXg15GQnFaIPCBy0hnE4ne1AxT+/tYVONnxw=", "pUD8oGahR/4x0KeWq3Ofpg=="},
            {"bTHCI6qlBPtZn/rOO4ounvrRrLDoViBrT+ysscEEo9Y=", "y8ioq8LGPny7Bd//Ur0jPw=="},
            {"6hnRLgY96OnHEsXGuAVn9Xlq+p2d4kCZawBmluJmPkE=", "SK+9jxNg33+wu4oE53/lVg=="},
            {"ShWB5H0nh/iXY8WFgJKDGbyI3JxviIuOaQv0TO0FYGk=", "EW0IKJ+ICebUFCdgeWcilg=="},
            {"lsZWevuLtbfHMN1g3elkCI9T2j1fyFpsvfcWprYotP8=", "Ll+waBhPm3MtOERv+FNs1A=="},
            {"hhdUEfWofVVf7eGSgnW+DPKeAD3CUhrPKd+xdh9xMac=", "pGQzczQwXZyWdS2opta1xA=="},
            {"TDB/guzUW8v97C1LhCdb/TfsW+au1cO8GTYGcNvqhjc=", "l9sF5KT/iG66YAltvyR8Mw=="},
            {"CY+TcpzJMrgW4b/NSOLYZDqJ2xwNBiRFlmKGX0kilQs=", "NxaLtKTx6dUcQZ8tcSxHJQ=="},
            {"DunAvQSCbZ2R28rB09km7XO8+kkRRFDhd0adeqTAYbc=", "QbOMbVnSMrIv0XtIf+fdUg=="},
            {"e5Iw1yMCpm5RMHplPaCM0BCs2QauaGpjBSI9PZUHdns=", "YbS1oNk/lIbslyRSyerMjw=="},
            {"se7VziIBWbbg2JJwof00FNxRK4aXHq1RdXaVG1HTUBk=", "+xkQuqppqR2plZ5Sr/YK5w=="},
            {"qlJxnpwlfxzXFopccD8NLhm1ZTSGoPRnShI9Q8lmzlw=", "VG9J/tHgINFN59nChPrI3A=="},
            {"mlBXSn5tGIt5nm2p67N4Xp1N8u+eZroON5qUUViEvhM=", "3JUsPey8xX1ibohTAAcDjw=="},
            {"N/7v6qBMIAQ0Fs8lBUSTo1sc3qo9mCTGGG3muzw1bek=", "g5gC18lAeUWQlOVWr5m32g=="},
            {"9Ck5BQrR+1W+nSJ7H+5tssyOcZckHbnCrQaZpajidFc=", "XkLw95NzXEMWIyDxTM+oUg=="},
            {"ak7KrHsliUGRj4Vg9zRG7fg0ngEHM1INcC1I54c5/Iw=", "uIhyblZ9m8UlduWI8vx0Qg=="},
            {"fl11Al4JkTeAazvJRjrhLwGYFFBe5EA+IPd1qMfuEsU=", "Yx0Rkn5QD2vYKnHx8FWibQ=="},
            {"3d8txNt4cjg2VQC21X8vmJjEFJfpFMRXqXcaFgCPa6Q=", "ZLGEKIEt5rXHMI1jzRUA5A=="},
            {"C+dcv6SqljKxBsLfwuokwwNctilPf5nydND/1xI84E4=", "jLNwYnrqZD4d3y9mcafNCw=="},
            {"YblTvpsP3V4N+z6XFFRAxlo78rIS0O1RF8u9KPciMpg=", "o4rZRm5+LhW+wunU+1/htw=="},
            {"NPh+NUkuzUgL1pXOxdvAc4/SFhBLpOHcmk5YXh4OaKw=", "85oq+uNSwhBd0vyMHUXLMg=="},
            {"ffgXeXdC0BtcaxaTz2HS7+h+KgwJeO7WCHxNUcp0vQo=", "TdaC0Vp23FQSkqGG6D1CpA=="},
            {"l8PDORmbIAc9pZShJfaUAyPZhCGiqk8mOTuv/9ZY+rU=", "9+Hnsmx5S48yvXSlGRSZBQ=="},
            {"S8l7IQYS414LSbVrYYyQkvELoZFpxm8gSonbWy++kIc=", "BdoWm/W63VIar+0M9buQ6A=="},
            {"HbRjrDWn8PT8JG0+rgiZ5AlcklVZe46RN+WrkmBBNhU=", "puMDp6TdtQ4XC36zbBTNyw=="},
            {"XepLtb6yiEwZPLIDN+0e6dMxrWSs+wWjN5Sbt9MY5+M=", "UAaBaNB54Yck2aFPg9I3RQ=="},
            {"DOLVldSyx/a2H8/z01frs2aotA4D1EMoBrwE8txm+jY=", "TmN588JeHS9qh/eMGDwg1A=="},
            {"haTGQyIvZgca7FIxGSA2HGfPT0EQxw6POWKez/kycXk=", "gSp18tvQwQSicw3zym9pbQ=="},
            {"JXNtOWEtP2rZHz44/90hpOkfDa6pqz3sdVPu48Nw1Qg=", "s5CfsKj1GFkhi+N4mEj2zw=="},
            {"q4xNzCaxShKzxqHzrgJdi6rJ3+Nv3Cmr8eI05lRS9Po=", "zgHwbc+0Bx1L8Fb/8KogQg=="},
            {"OIp2NpCNbG2vhiTGTERmlQw/h6J/3OVBnPnMiNRhjcE=", "GWXvA31nffEiiYd2VuCm2g=="},
            {"QzDNB5Ex0ZMcoxy0Xjx3moDbG2SIlm1UxjttXHOdRcE=", "moAeu7Ei4pnwYMkaaJraSg=="},
            {"ZTDosL1NDPRycmCPw4wfNB0nI+J3KRk97VbnpJJX4pI=", "Hu3kTP+v+V9At6BGCpvD6A=="},
            {"Gy/488f0iW/PQ0Ja05YJwl9xONl5jxeXHzjBYolGYo8=", "7vFPUIhQ2rXcVpa/qciVHA=="},
            {"2QzdhbcXbOVdff5nZAkYcPrLLP2fDiDJ3atKBT0twCo=", "gAqNneZP/Po74kkd2olFNw=="},
            {"KoRnb3hkOJ0V6quvslAJhz29TuIGiXPinlMpNq0Ojhs=", "++U91DthZEdv3eNs6/OP4Q=="},
            {"ymjruzneG7g510m8SDiO6HpeCiYUib7f4zNSSMz0Tbs=", "xVclAsjPR8xF378X9cQhGw=="},
            {"sUMSSCnE6ZXEm4jeXlewm2D0Wgq0UMaO69LlMogC+a8=", "QrfgmhKz7zWatHJhYDBWHw=="},
            {"pF+lgWDWKYAyii+ol58te2iyBqnSvUAvuyfQ+9d5asc=", "/oMHi73HQOZu+AfUOgnzpA=="},
            {"qHN0yEoCWLMUYVFpPMbmzksr2hWDDjOWuElnK00qyOU=", "IyzvK+U7Or6jLgCHBTe/tw=="},
            {"J0SRGANjKf7MIvNQ6uMefqlexD0RnFON+YK6xcQ/NNk=", "r1IRDedHECfNMNe20oVyRA=="},
            {"kCjV3vUP8VD1auyFzxeP1UwDHY8uKiIPaRoNq+7SdJg=", "CGiPiuBLauS+mcN8XYrOXg=="},
            {"FmzRFrD1FltE6wpZmy2zisJnX38hWN06bD6srft0fa8=", "e5c6H4Kni9jdp4zpj/M50A=="},
            {"Io+z5+93Hucf9VIKowK8RGsgTQYQL8oAMzheFimvl0M=", "70Fo1qspsUwn3k1gy9Rjhg=="},
            {"1Xxg6atqDp+7CbQvBaJCkqmO2VVzaajyakrvyk39Qag=", "O1M5ZbjQLMvvz30d0WAihg=="},
            {"MeedXLdHFsu5PenF3KfwG03W1QaKV6V0a1Hl1J3KkX0=", "6C/WZsUih8Zse+yst57JyA=="},
            {"Z21AxW5ND4/TCc6Y8LGVTE90Cm/aT9cguK3IgzmKNOA=", "JpZB/O43egVaKsr2y408WQ=="},
            {"ikd8U4mJznnJA5vMTN2MKZzA7pZRG/AOJP2m5f0VTfo=", "VJrfrLY0xnZhPW6JYYuAPw=="},
            {"DeK73NNAXo8gk0ZKTqjhzo1XnI1vuHmHxfjWyoTvSn0=", "c+2vg4Yfp6eRjyVPm3/7iQ=="},
            {"A+GBjoACZoNMXgqbxuqrdst5fEC9bHivkAtlZZQTJT4=", "qN/RhHwoHL+omhcxyEbTMw=="},
            {"0+VKFA6+i5bPX2tdp54NmvzRPNDIQRHXgLw8PoYz6VE=", "jJ1Y0d6JaL4Q2mHTehPnZA=="},
            {"Xk9+YZhqM7tcGVNpgxQ8J1Ih14KG4+rRWhCZH7BH050=", "YHDVDcrEZ1lwyXHl8ny7sw=="},
            {"Uwt+2ARfzwmawk7MlsMjb2hE6KegdYHfdhiOhA7TOh4=", "lWWw4a+SfearyhtWzCTx0w=="},
            {"1c5xQwFV4c7yKjZHiwBV7XN5qDmQtAbdApgRlt2zZ9w=", "aYe6qxzTyTo0rqsXTdjc6Q=="},
            {"O9/fdwfyovpzZo2Kn8vb/L+tJBhpJ3uFjby7BxDIgEQ=", "jZnsSDcg06G2L99q5B6HVQ=="},
            {"mH31MD5ARIRBJ2BrC5JwjsjoO5kHfTWLCJ+GkIvrbdk=", "/BLzYZNyGSTJcmo/9yPdjw=="},
            {"KGazW4N8KusJQLhfK2Ubb2WBiJluMxeFjLiBFovSqcQ=", "181P+FRbXtQXVMLHk0xIcQ=="},
            {"YQVyrSMv89BusEhPq6lhTvUQzeVxlU7KZ7+/PjdzQuA=", "NmLr8gOM1wSHf+4NrDB9sg=="},
            {"Oql+8POz9bRN1LNIxnK2iwGtI/7U7J2MRtfSe9eLOaA=", "y3QUDo76OizBO7l4JFG6gA=="},
            {"9J4pqzDY3Q8BGECHm0kkNxrUGr+Qi6ZcGBj4giuhF4A=", "/NbbjXQhB6u5Jlv7xI3zMQ=="},
            {"6D2NRs/f6EGM2HaKqE3f8MiHOdRCLPjEceF/qRqgmKQ=", "CzBIwkQD1qd/UgV80ErRAA=="},
            {"Jpe3mc8Iu1P9EkiFcCdM4P+mfTR2mROwwRu+jwXqEg4=", "9tZS3woE08TUWP6CXyQyiw=="},
            {"D+goNC5co+xGfF66kW1nbX5nJdNYfLQWjpJRRNs5Qd0=", "gMYDEisz5PZNj4PhOFiBhg=="},
            {"+/AUcrhyLDzr0WEVP9P9kO8FEJpMgbPaf4q4w2l4zfM=", "crOf1MEUBB/mpQ1aNDBkkQ=="},
            {"Yy2xGLHIEiqLI9f7lHah2hIV7POejtf89bjDShmca88=", "W3WDV8Q3qkmBhVVK3YE8VA=="},
            {"lVy5cLLAlsAgiHfaL1jDqbga74V/NfhZ73abpJUsZnY=", "Jg0eEU/Lv3W8U+l9QlmXmg=="},
            {"N/DdecOtkrdm70oAnby6bSxEWHitWS/+JWBOTBZVx1c=", "+VnnPmtlVr5dBD2j+XhLcw=="},
            {"5vGffHnGC/F5bsFQ62s1OaVkd+eRiJrx3d/rPj5zJRY=", "bcTRxvf0wKiO5VCwac6A+Q=="},
            {"vvonW/Nr+5IrskafNpDxi8tVLLSPXdfGVWowX1MIh+M=", "tM1SCj4im8soPNQ9tSPkFA=="},
            {"w8PU7oShgLdPGkBSMM26Ym2eXmA2T3r/dNsyIEg6Pik=", "SjgQr2wmSPzJAAsqxiBnrQ=="},
            {"eG8CoVBruNrGQl0eHutm4mSchUnB7vLFXoQI3/Ou6BY=", "6vMgZ/RnJRqYysptLS8yug=="},
            {"U5H6D9Zzd3A8bHhdYGebT/GCbPfBZSy0qP/65m44IG0=", "Ovkc+xAXuO1/Z4e0dt/Hjg=="},
            {"8v4QMKdjK1gGUsQoOFwlWaG624UYzkTw//rrasKJSb8=", "fOxM9grHj11hIVjgrzXorw=="},
            {"vUVLlou0reSs5AjxldcmlVVlBfNrJB37m/SPpe0+4m8=", "/XCqJrE/JxSe6M9ZhWR8+A=="},
            {"ZmkyhAtqr0BuyMcrIhVKIizesM6IHK3RfD88adJHDjA=", "WF5gcQv6Ztri3OTHnRJLDA=="},
            {"bekVwMxO83BYTEt+vuvRCIh+HUC/wXgB4fHhjzmFGNc=", "qclNYmjHiuDBLWPsWeJ1rg=="},
            {"7sqpchatMOylV3pQOU/oW0QOep2nKreTnSKaPGh3DaQ=", "ki44qSHBVVu/madzXSNwSQ=="},
            {"KdpiaI6upu38GL3goXIcjvQqef2bYr64pf8et/+Y6Tk=", "1a1Bdjc/WQaG5Vr/VSl3oQ=="},
            {"b7KwYjRDWpdkqJdM2YpP0QxYTIqKyqKiiEiEoc0D9go=", "VZVCEEK9M+UVczXoa1NNrw=="},
            {"hiV2S9Zjg35Ppq8aHjDHwO3lAZmcJxAIL2XQ6kssjCM=", "YUIqs/lgqt0lV47kCNFEXQ=="},
            {"TvMLCtIJLh+6jfEl/g88NqpqtYj7xtkQVGh6Bmc00tQ=", "MvcS8aolT16Aplf0/whfpw=="},
            {"1nMu5ynvTQoW1sVqZsX0723cVdaELRWP+StUuGlkwck=", "ydjqR/MJTZNjIKy0kdzYew=="},
            {"V3iNMkVFYuLHGYQGFH+wdp6Nh7KE4/VZDiZ4xUfEML0=", "ULCsAT/Op3vViv4EtfkSGQ=="},
            {"HdTUKXLn5hySdECgZjvM/9U4q2kPPUYP89GZ1Qx6vgc=", "PpgEi1sddqIEk+MAnbJo8A=="},
            {"Pxt3UXs5zdSzs6SgptYO5tEb/sPfAjZ9XH5DrkWRyzg=", "EULD8LylHZi+E7vDmH8fUQ=="},
            {"3TvkobeDL2+0Izg4PJotQItjY0X5l+nE9tgDk4R9PYM=", "L+IDbgXHCBp2zsTwph9FlA=="},
            {"RPWOVcFy7W1NC6YlXxo+7Qm1VcL3uwjN7kJ9jQuNVos=", "A21K6IVOaWprQWJt7qKPwQ=="},
            {"JOxAJWiRDUI/qKWJrSxptBiGlU+Obia6wN/ZtPDT1Tg=", "ApdbWzXSYX+GsQqY16+ZdQ=="},
            {"bPDdhZ1D+hASC8P5uLhidggfh7ayDKFGEtoXnmhPwEc=", "xP26j9eZ7cL4zkuow/gKIw=="},
            {"XaYzO10tpW5v8NJk0DHoOb5RW33kws8ebcL3AfIMglU=", "ZM2Vrqjr7Y5asOd5E7RRdw=="},
            {"G0y6OBEHEU+L7ufW+yZsTZCh+bG0ispCXMefw+bYbes=", "OE3u+Hgu3EHOdztiuGM7MQ=="},
            {"o+2N8QdtAgyi/tAju40/m02jJb6Xzqg5XUqipKhG37E=", "I43AKbXcR+rMxxj8AFIEag=="},
            {"jHqoZ0r5KBsA5ZoXgwVssCzCZ0JRPa9l9hLll0LeenE=", "2S/DRyZ1PKq8wAfd3ygPpQ=="},
            {"S9XXnvxGc5864+mVtrlgSKR9cpxiDzKFGGTBqHj73cU=", "ZgdJCtsG19vbIEGezpMxzg=="},
            {"0fCiqvESE6c6rFwvP3IUSFlqNpp9PVKV09cj1CNG9vc=", "uZ28wlSrfkTrezMIQWmzHQ=="},
            {"A75LRb7wFrbyYnuul64x/KtnvNcFPEgxJ/zKBxx08XU=", "3bbKJPpz0LiR4/Zvw66+hA=="},
            {"LbpC6EBZD3AjLewjfSZRjb937N7NbcjCREvciMDNV/o=", "oiSWX55CPRcg+wQlkX3YuQ=="},
            {"+ZRdhoVo/Vvckw1hDeNst+J33DMwHVN3zuC1nrHQWLE=", "KaXWL/yta7b7f6phBG3MCw=="},
            {"NG6QFJaPdEEGatTadr73NHboZ/rpF69/tRyLIWTlt2c=", "9y854+YGwHOZonRUaYYpTQ=="},
            {"Yt2r+uwHi8iiSDdbSo1MGZezy1P5ANxg1maPcduCYFM=", "XkygN/T8uMOeEPBJ8+CryQ=="},
            {"mlSwh/3JOYKH+bF4Ws6isodiBr8L0gnGItg+4WUmHmU=", "hDUYfbMCnkzqu8jo3ymOmA=="},
            {"KdLDqut2xkcMKioNTwU1h6fXtIOYBldwHdzZPoyBcqA=", "ZdPNhyesSdUj3pSWmBzlQw=="},
            {"auvuXafqdczzRmjZ6WZUly5+lDUINZ4noYxaN9MXGCQ=", "8AylS1UYDXvZTpyJ0FCbpg=="},
            {"O90BWQ+AQjLiccX8zUhNi2ywEKA5vcdgqK3KyOduQHM=", "Jhjg8Aq50EAnV30Qh+ghMg=="},
            {"7cpN8SexJxVTXVMrlMILOYVkqLQdxrFIic07fzGpohI=", "oYxgAubzQPF3IFLpftR1PQ=="},
            {"PmTEPfbH2QnXxBc6ySL3TCQSDUYbS1MKhOHfOP9GJqE=", "oLYKOT8Rog+TfLjFuRnfNA=="},
            {"Q3Ffn64J2oct9vjQr95DmWe/JKeCxgq+l5+gcYm87mg=", "a7Mk3VblTDzGs/rcuYUa1Q=="},
            {"xRzrct+FPyZNaodfytd+1knhMNzzmufC6eBU8ODB23k=", "Pd1ofYAFzmFdx1voZG2YZw=="},
            {"w4YeIQW7JFAjdYkNlKPRwhOfKnIkjmelsi6uXK0Xo8g=", "+erAMTFWjyYc6TPiXZu+6A=="},
            {"OSThmbkRx7oeGeVprrigi4SR3J0m+Na8YYrxRfmml/U=", "VgncXBR/I8X0Hs3OLZBjUQ=="},
            {"214pDeadFvLpTENbEaH/xbJu+MC8CZeW9+3uKfmxi0U=", "ECLkLF7LWTX7oELgNjEB1A=="},
            {"yM7U6iOCtdL+euhg5taZOPvVzpSQFPjtQzUlO4X3biw=", "DMBMBWMSQ75JSHIz8WgM/A=="},
            {"zEMwEQ8bi1+ZeNlWKtNBfwEtdH80+gy+KKE0JWC3ioM=", "vGk/az6UM8isvm12EFsVcw=="},
            {"0HM2Eum4bu+7vKarPLu4uVChua3/RcR1xky0RVkjOBk=", "8/AdhPHOWn99WKOikEu2Ew=="},
            {"YRCEGqg462k8z+JvNTQj38hW9Jtva+oWqmfez/fAJJo=", "UAHKG0ZJxcXDznN/Kfcpkg=="},
            {"xyfum+JmILTCsqyxNjoNtXOZnOONLrcHBLB89MpERKI=", "AKVoMSozS6rJjekNntX/pQ=="},
            {"GB3SoIMJYkdRQIyM9f6YNg96kgwqyN9kF+krOn+LJfs=", "AW5mxE4/PDDdiAktzoHgjw=="},
            {"JMsjpsT/44b7VNuK7XQASCjeLt2tt6HwrE9a2Cgw4cA=", "UqsmhXJKJudeMkjkYUh/Kw=="},
            {"OUPRk1sTxYuXLFMCKd3uOQbERmy5zg023TLtVlDJZbw=", "XsVxts+YjIr1XzAGtL93GA=="},
            {"qisUyxqHLZSvze5dnml0drDnazw6FQ06SooSo0eG4mY=", "5B2Lz8j34OUfNbYxDypJ5A=="},
            {"I3Soz9spXC43fzss4bnxh+95V18dYifzxcbBNhG+zu8=", "bWDlW7PRflFBai7gsYa6sQ=="},
            {"k8+2aLoyqRXR97eiSJvV046m9bPuJoU+MEj465IJN/c=", "LW7wjiV8CFIHZIVc0tkrDg=="},
            {"S03+7XTSK1ixIqpQuNIOrBrxUeh6OlfL4HJtdvZPz14=", "puevPT64127/jZsXBTJmZQ=="},
            {"4G3NY8waCoT9z3xrc9ij2KA0HpFPlIrXcS6EYfMvKc0=", "jjM/8+wS30UO7MANk0fRAQ=="},
            {"bi1qig9Ou2fXemJQXOQYfc/l8rdQ+pk7NL+CaeubXYU=", "Ce43nYTZ6iwKCQ7ODF5eEQ=="},
            {"alm/8ErgSQcQ4lPuCm9tu5cy7K1cXAEccgAEpBau3nw=", "326ZnqXs1qiYw7FI6MWrVA=="},
            {"JN6P2PTXCOH2iPU84whWO2VI4LEWhjcpYFOmya0Lu8Y=", "iNo4h8dK+QPku+zbt0p8HA=="},
            {"dD6H6T9wjMMpnl1EE5Y3vEEgfl+VwAlikSV5JYXhUow=", "DZKfwmb+2emBrMRoTqM3+A=="},
            {"4pmL9HPyQ/YmTSUVNV1QNNi29PxSW9xSZnzhjwRoZXk=", "raOhNgTikYvcijd3hh98OA=="},
            {"YWxK2MkHSN3bI3hk0yMorrAT3DXzbpc8J+T1mcW12Z8=", "y7YZG5HnF3OrEg1hvpJlnA=="},
            {"iDjy6HNRGAddJ8elIAmrtVEnx9piq55Il/HpVyeWzhk=", "1zeJwKE4tEL1dks8Dg3LuA=="},
            {"m3Kd5OL3wjikmJqqPEo3bG/egdyVT7QSVDR9OQj3RZ8=", "tKkPQrbgl/vy3OQ+eZn2ew=="},
            {"Q2stCnKLbAsJpLP40p8dtHws1sfgKWq4wp02brz0lW8=", "iIUp8bCCngGRGk+rUgkdVg=="},
            {"vyhYTDU2vvl8K/21J6VyV5Zra/4ObZe7S6FXCv1Y7wI=", "wh3pMxiupAfDNfRzs7Nkew=="},
            {"6DBEELkZlbEyfh5obNfo2feE71c0mPCJYyMH7hjCJdk=", "3YKvisai8PM6MgqTJBb75w=="},
            {"Z+m/gDOTDSCX5Gg7QVDNvyJ7l6dVXH5Ehad41GKcd8s=", "tkTSBoSiKKaq/p8h99kNvg=="},
            {"ImWcUzEfkrEWsb6/EsUO3OqpQJ8VE28NuRdQo+e3wtE=", "fsoXHwU2aJCf85w3O8TuIg=="},
            {"sSopm34MM0ipZ0LBgt3aj2YtGsUGCrROepHY+/Q8gMM=", "b8wlpidfXtK87Ncb3hVGJA=="},
            {"SifGPxNWBdWsREOKfchIV0VoN5UqLUJiseGPoGrsdmg=", "A/C3shPprHV4ted5QdUMDA=="},
            {"rUHf7OhcnBDoRwxMqTolI2LewUuJB+b3r92mz0HjhVo=", "fZIU1JtgoPsaA1lfsCLAsQ=="},
            {"kMam9UPLrsP+wpUedmk67+jpxu7SMYJNHrpycRKr8jk=", "eHuk25x1jbB2AXktLDQzOA=="},
            {"iPU6Hi4wj2OcuNaKa9kaoIgM3U9ILv/7FcFBQX1b2PA=", "TopikdPHZj2JuWWIUzDkJw=="},
            {"25sywz4N555rGGn5/47whMgHIhkHvJVVf2wn7F9HvP4=", "xJL3JX5yNYPe8njRSpdaKA=="},
            {"EbERfbXv2BsJyf5uIggsP0JT7Ly9hphdKDC8Vl7EZ5g=", "v7VDruvFWWUXLNfO2stbFA=="},
            {"eJXt9Ze5UQDeRFQynab2qiEPtInT0aCT3LExDQP0POc=", "wKFPIwGetMzxtqmXMJuR7A=="},
            {"TMwwS363SkDvUNW/LCRBLAjM+jpBcUpaDhNIaZDrN/E=", "tzqfWzZ1/0xzsLtU1fkZZA=="},
            {"1SffmgrwmPyjIgo0ug5FocOuBKkfMaO3W9D5Zie2l4A=", "WDUcggsFiS6uvmMKhkKIiw=="},
            {"EozzbQITKZE9v2BbX1dDkYOwjoaamHVjfI1870nPS0Q=", "O/20rbm8xNjKTPfaV1XusQ=="},
            {"DUVrdtEImdeQQ3Kdl9dpb4O+OBvuDmQ14ix6zIRwg8c=", "zuvlTtHGkuf3+ZMR4Wm5ww=="},
            {"+QKoST77uIiYErju0MTMdwMHVlcXNbAAkqx3wGbh/d8=", "gnQWHU8XD0l6pvt7CfYK8g=="},
            {"0Wi2ZObvrIb3EI1GGb1hXuaXPuNGucGur/6TPBt9XZ0=", "Rv+C/Ajci0/FZtEgwbd+Gg=="},
            {"vs6AuFn4xKClZfjRuF0k9/k9wSU8D8xDoMDa4C/CA4o=", "j8ckQ9A+Q+aXE8eV0Kcl3w=="},
            {"BsyB+SEzSGEw3m9slRCI0RyHH1tFZ8nC40eUbJXVCCA=", "XiRmjAJMtTb9BvlW2FEdgA=="},
            {"dBdt8e/YthzzStjFtbGmlyHVKQ2CBGHKxErHPyqf1vs=", "yhGMsG5qfMBnQzRpWE+wHw=="},
            {"7XAhGo6nT26S2uQdS2CxXExEqfKq02+ilo5XKVi4KaY=", "nglZIccMz/u/6r9HMUEXDA=="},
            {"Pk5Zxy1pTNG3JMxtkiOOk2OpbHbdhiHRylIQacoaMks=", "gna23rUNihmcU9p+JLc2CA=="},
            {"ECwJ00mh6RSV8eor2yHLLpj8JyEc6EBF6oSo8v1zNy0=", "3o/5nBirAAk7nKv17U6Mcw=="},
            {"49ZwWmxNJ+GbhNci4vYiGAkWn/fsNEW40u7OtKVxyl4=", "n6gZLDkewJYkEyOEBPawfw=="},
            {"BAlEFcn2hV7FzQlZ+4PdWbIsiogLpUkdxihPmypo8rw=", "m98aw1AaZE+F61v+yWxc6g=="},
            {"iMrUDz+vjaYKmESHwA6tZ3taUs1WaSGySSsJO+oNwbo=", "+RBrgGyXAybzpVhZ+m/9BQ=="},
            {"X49U96jUQTh2sb+ZCRZm9dlAaBc+tRxZ8gVS1BEBU24=", "Nbi7BPawUDYXcqrZ+0dn3w=="},
            {"bGby3J6Q9vtjF5zbtFqmkBwsnq+h7FOXRkmazpzko7g=", "j/VDBefC/irEqzrvD4twaw=="},
            {"UFgxrHd9o8aWhxJWyoxmeh3jSJq5gxiL42fQnexKQSQ=", "VI6uGimHnqko/vmtoCqTJw=="},
            {"CnzQCa/jqs9hLhqxbAiAUhOe0Q1ZeoOTY2tdtf5GSYo=", "d/auYG17lXJ79sgFG1bvyw=="},
            {"FKw71AkHZZfmmzPiJNExoCC2ZDfbqr3H4TS1L2nTu/k=", "sqdXuvbDJPq2hUzjjDo11A=="},
            {"IqUzumOijvIv1rIGSiXrUZhToxTUz8T1IjhuUvUh2QA=", "iVw3WTvZ8AqSyBY5/gTzVA=="},
            {"XNX3RYVjmZCkHaqEitINJbpv7a9kA1xi9vl5V70vtmE=", "/GLYMfcR7TQQKOvw4r6o5w=="},
            {"+80aiJ8gRy28ksZ59yy3/Li8RkYb4JmyU38efCHs35Y=", "29jc6F7FibeVdiFpII05Bw=="},
            {"IxlZnNCRHendKWKS7UhrOIpLdnCgiNWL0dAf8bbdzpc=", "g5AOdmBIyvuNN8CGFECJXg=="},
            {"gOofMOup3quB4UvykCcT7F6BsNKy66fNOFEAlVhVuh0=", "V2esHU8NbSrj08iJepUGyQ=="},
            {"Q8GGkrlBfUP0Ut72aed0rsEmIHXJ0ysdFYPXuNfTgAM=", "ySKx+ZE/UG+Ql8fMlCQWgA=="},
            {"uRKb96YTm2wYAD/knUsJbxeKUdrT0PeAUTrPN9hbPbg=", "SeEULDoNjJ02AFz4l3c8HA=="},
            {"YPodDA44BTFL7rh6/95K43wtlHrozRcjoZfY4kMYUxc=", "WdI1ABvC4fp65O4LOKwIRQ=="},
            {"/D6LJOTpizK5k2MfPGmKiI92HufsXPzDaRCvoNU7Zh4=", "dFd4KDC/IGU0fCipLo/P0g=="},
            {"N4+8AjaDwVxT2BjhIgGVWKzXlJ+/pVO3IbM2mB3L+74=", "G6rTRQWUgQe0Srhej0ktyA=="},
            {"ooUvtZyDo+oDkKkxS5q0SLmiQ4I81YxW6jWNmtVhIA0=", "oBuCMCFL8rmB8K1RR3gE2w=="},
            {"2lDAMj3jDHWJek1IKurFCCDkOmgbZk2iRlDfRNR7clk=", "EFBm4uQyZDrC3bPt7jYddQ=="},
            {"MyWA41PWRcKLxMSZPgQdNyIZqYBzy6a+VXL6GAQRGh8=", "EoN/HFUQpUeBA8ovgpHlQw=="},
            {"HO356id3xvtaV/bPhBEH8w20wGfpeYGimgWV90uIpW4=", "tSLboyxMKO4ZV+D4raFSiw=="},
            {"HYkHE7r1B0YetB3eBKNrZSZNgfUfWvdS/DYmXZHoclQ=", "Cdw5Ijh4Wywwoll5uDR4Ow=="},
            {"/DPqBtivlhyqsEQElW/CZwIgIXHlEotNOPcy008LM8U=", "w/oAl8PF6l5CbR4lO3bgPA=="},
            {"QA9TJWLr0+99dB9fBsWN1Dr86mPRf//wZdWOv6QZ2kA=", "pGhVUIgvhmf8yzPJ1Zodrg=="},
            {"I37fqTYij1WvZxUeSdWr3NXve7lbAYDUZXhWAU7wXzE=", "9jQQnIdY19az7NskzD20Kw=="},
            {"SSpdVbaiNEhF/1VPNm92QP9dVz0KDtQJgYqEhrK05tU=", "eSHyDPSGEHvdjEUXxBjzSA=="},
            {"GTB+V3gwJaiQBPawAOduUxqhUsqx6280qBGNrgQ75RA=", "3FIulDYiWMA2QhhvCJB2rg=="},
            {"HJZfmpdIpjvUMYIuYbfJDTdXvGBKrEUUSIdHlmADAZw=", "9fIVqmGJ8T7MfqXFOYTl/Q=="},
            {"dOfzRszVOCIERlUDtepp7xiZmQMnuMtFB3fX2MVQfQE=", "nOQxs4cV/vMRAUm6uqAnXw=="},
            {"6hrqEt3Nusfs2teDSxxZ7vd08/X79be//iGDiKuB7Qg=", "fUBMggLiqdconUTxpzq4fg=="},
            {"yjZitdy3DmmnRMtMEM66m8THFsatXibaKyG9ukK0JDA=", "9SHd3yitXv4dI2ArU//5Ng=="},
            {"g4bE+nE0nR6DlW/Zjn8/Q7WyNMfGtSvlH/sDY69ubdw=", "VxMOKEJmm5Ezn0/PeY4R5w=="},
            {"o1eq/fW90qrorZnQLjXOMfbye+LC2dIMXqfRGDsEIrE=", "U1t2dzhCe+bOGWzzZX6MFg=="},
            {"fT+kcXtrEjJpBCScmdIcyCKAcSN4id15XS4wB/Q/Dt0=", "9bNLF4kmJKUa+Awt4qeD1A=="},
            {"ompL3eZp1dIAydOCbthN3wybYS6kRpcDYL5KIzvkVXU=", "JtDyjLbUVkLdNNnVthq0lA=="},
            {"9OyJrisgLkSylKhvp2AYk4r1rfNbuFNz5Cm1A+V4Vbw=", "tQBVFZ4hfgCVr+jRlpPIxg=="},
            {"p+enjCRXd1LGgc+a3eDfdgtxiSQ/g52j00wVwl8K0s8=", "svId5NttpH33Ij0bswGOwQ=="},
            {"hoEuXV49B+19sO5S/cyCXppB5JrgfiO2pdyr0FSU2zI=", "WRvlbhw4q00SxxamYjpadQ=="},
            {"Gk8gyd9TkUunaGuPT91sMGEvUPu7FmkE4DClGFPyWUw=", "geTOivdb/2FETwDfVyrYYQ=="},
            {"ShCpH2Z1fqXAoNaBqRSJBSBLWqgw91kwR++O6+53TWk=", "agpjdW9VkPJCAZBwEhizEQ=="},
            {"+5G+IbfFFeyqjAW6WfzgLcxvdvxvrM+0psT8cRDvX7A=", "JKTL2rNAH4tgEJqjBZYTiA=="},
            {"suW08XDHP6PSqDtHexN91YxeLE6x5jw1A75ftj11Amo=", "nihvZNZ1BN1CN0W6FBlr+A=="},
            {"DGqp1W7dlguXiQlBpnwQ6aKqyk4qkLL0mRzscnY2AQo=", "y7Ru8dRLocWR7Zn7Ef2xVw=="},
            {"csWn4Jz9XI0EVGm3Xy0jt1oyJBSJDSAqwC3eFzo+W28=", "tnWk/nD/f4pWDDEFZIHvAQ=="},
            {"bNCzgerTeLBGpd9r/EnnSzJuHXZ2P39KzG9JgHpBriY=", "gowIaOIz9o2XNIMv1lrpHQ=="},
            {"Onu/OGXnYgQTL5a+bcgSlTLUE4md2r9MK6qWRBKoPI4=", "IsHbgO5PF51RMOdbhYbbjg=="},
            {"+XosFt13qb1vUhvOi1DvHvmb7woZ+1k0QieOJVXLIXY=", "XnWvU61wjeidKrSnvdmi4A=="},
            {"yBcYd9KZjQkqXEzhzgyz6rybu379JDRKQMmDFwWi1Lo=", "1DBwdGUXqs1VYK5ItDPiAQ=="},
            {"2GqUB1nt2cpVthXW+rEC0NP678hiNkwMg6tnc62xvec=", "a6ie8Jl7BLOWiYn/guL/bA=="},
            {"ZWRFo6LuvefyPSJIWpt//+9uKrQWeXb92KKqy53Tvnc=", "+oxGoqERt4fp6FwywM6Rcg=="},
            {"kyQg7msvoOXvQ2zW0hhaDiwokIWsbzkimILBdHfzyhY=", "vT3VBq0yjTycGKxq/N+o0A=="},
            {"MqPteJ6pRNrYyxymbLaksuprdARgnPxWJDstdYVDDko=", "UwecUNycy1/fZgV4hRGgBA=="},
            {"13LpJ4l8mtC99Rsqjht5asRG6sPSa4LDSMCEfdAPJBQ=", "SBb8oPwgJEVPXK74a4r2IQ=="},
            {"urzndsnm58+cySsXpu1Oy2bnJz8bfAQVtISXKg3i1bg=", "Mv+8Cub1cAJoDaZuWEaeOg=="},
            {"oDfaCA5+qDJ2kc85TYwyPXpfgoP7gu1UwBlRCC/dcpQ=", "pPdvhYapF5hT4IyQkJ0kjw=="},
            {"EXyBvtfZNB2q2qBJYXhYXtfSg67jn2b3i6psS7LEkJ4=", "TGvQwroBV0gRrE23igsZ7A=="},
            {"BiQH2houoRBMKUQVMrgeQu3OVurA8YK8NbodDG35LzM=", "8CrOa2B2vHj2ugAoEaZVdQ=="},
            {"wsa6lBqXYZ5TgAWbtdXqT859OdF76cUnATbFLPV4BnU=", "Y7wHWcWLuhzV2jPgFhsb0g=="},
            {"N4SKryQnN2pVPl4us6R+8wqNnpH0umT22Svf0crsiWM=", "sdGwvIcCsy3DLp3slWDXow=="},
            {"6g6JRnU4y/GrF1xGPbNH8DuL2rTk5CVrSThq0cmMCgU=", "t8Qbih2cLT+iR1ky9X7zpg=="},
            {"NhFmTG73B6M0SoM2ImxQPKQ71ItJiF92xjtb5PzqYSM=", "2AcbxdgqfIvaU0/3NPS2Eg=="},
            {"K2nFa+uihwSXltAzSv1p1Lo9kYlpawZkDjySI0Zg+68=", "8p/7K+VXLj2DOu33DJkD8g=="},
            {"EuvdEaLkQ+YP72392zUabQEYYh/9RlLsxTHiMti8ccA=", "F9AQbcuVUZL+feVk/ltxqg=="},
            {"GN4VTGOJ1Ntk6zpSf0tczKYVCiKPorR0qnRhQP1VQlQ=", "7PktFHq6V6Uj3QuWHBjwFg=="},
            {"8TuV505trZf8jgNSZtaYhNA38x3UwQyF/nkikrkCaIs=", "mVorfrss8Ufz2E859pAf5A=="},
            {"n+4EKQuGPLF7CGMJ0Q4HjFQ7TTKiMh4IqE+TjqQVxi0=", "19Pz2eWxAZKnr+ePerUGUA=="},
            {"VAqEj+1sDfyenx9q4qJ4OZkfbWYH9tbvSEqgQ1XWmKM=", "yFE/p2busOyQZc9aZGaaPA=="},
            {"EEOpXoElkosp0MISwKsZFqRKPm6MSCN8uC5tbto+pNc=", "2E/+rygY9T9ub65Eb6ydXQ=="},
            {"WmPpZr9dEHcd7SFzJrQ8rEFJRZXuxLvjlMApGgssJao=", "m4AQ1dGIu37m1UY4oICW6Q=="},
            {"czPox8Su5fx4tR/Lp0S8hGNCgBp9SDqSlsyp0LnvjGI=", "oKmxO5NQRBTBhxnepVVgfw=="},
            {"n/A0gxgouBNm/xNdNuofqGPnLPCuR+smixO3casKQ6s=", "Wx+x0KEkfaFS0rNLx1+wxw=="},
            {"hCH5GVjuNif6k6yZwbMQvrKQGy20et6bXnWUrgiuOIk=", "jdjlxzDegF3t2ZSNgdZUjg=="},
            {"KEFZ4z6UTHep6v7I9MTb8itAd3Jsf6JaecRlOah9uZI=", "09WC6rsHI2haO1ja177d8A=="},
            {"Cmrn3zT1vTNMb4wZ86Nl8CqlS1L0TcQbml+nwfKOFrU=", "AY+VTrkrPQsqBRnKhrdlfA=="},
            {"hJl6cKYyxo5Kl5qSz0mWa6P+ZDHWq5IT91tgO2diGA0=", "zsJ4X2st7JVTgI/nOH1zag=="},
            {"R2OdapRTauKbtJ35BT1VfDNxoiwK6Tc1m35701kup5Q=", "GgWS1IkdnAhqINs01Hh1kw=="},
            {"Ny2oTw33GBECLkU5W/jm3l7GPiOCjeRAArvmD2j8QPw=", "H4KyCl78uvPJ26MnyYZpQg=="},
            {"zD/Qv7kpEOaqtd15dS16HdBEf6k4/DfwwTwT5GZ2wSs=", "FBFH4kTId6hHPRoq/Qb88Q=="},
            {"qhZUtZCTt1NXmC5fFG7Clqs3CtSePMIs/9vShA1G4kc=", "I2Ldm9ltbZjZK+m6rfQn1A=="},
            {"kxxsdKQseyBgygiQY4GqPXnSel5GX3+52xUaA8bcFtI=", "68VSzUp4X2HigtsAYpJ0fA=="},
            {"ubcHC98t5b/w6lrfz0TIHW0QwnG5vRS4Am2uaI1e+qo=", "DmIJDTcBKrHykf3wF5NjAA=="},
            {"md59EBURKmYrzfHkqaFE0i4d6nXHG/mAf8stWXs5Hf0=", "iz4YHe9LCCg3Z9ktcOcQ1Q=="}
    };
}
