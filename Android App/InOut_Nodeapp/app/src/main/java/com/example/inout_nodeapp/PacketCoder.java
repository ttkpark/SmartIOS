package com.example.inout_nodeapp;

public class PacketCoder {

    String DecodingPacketString(String mty, String str) {
        char[][] ary;
        if(mty.isEmpty())return "";
        
        if (mty.equals("890")) { ary = array890; }
        else if (mty.equals("891")) { ary = array891; }
        else if (mty.equals("892")) { ary = array892; }
        else if (mty.equals("893")) { ary = array893; }
        else if (mty.equals("894")) { ary = array894; }
        else if (mty.equals("895")) { ary = array895; }
        else if (mty.equals("896")) { ary = array896; }
        else if (mty.equals("897")) { ary = array897; }
        else if (mty.equals("898")) { ary = array898; }
        else if (mty.equals("899")) { ary = array899; }
        else if (mty.equals("900")) { ary = array900; }
        else if (mty.equals("901")) { ary = array901; }
        else if (mty.equals("902")) { ary = array902; }
        else if (mty.equals("903")) { ary = array903; }
        else if (mty.equals("904")) { ary = array904; }
        else if (mty.equals("905")) { ary = array905; }
        else if (mty.equals("906")) { ary = array906; }
        else if (mty.equals("907")) { ary = array907; }
        else if (mty.equals("908")) { ary = array908; }
        else if (mty.equals("909")) { ary = array909; }
        else return "";

        int nRow = ary[0].length;
        int nCol = ary.length;

        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < str.length(); i += 2)
            builder.append(GetDencodingCodePacketChar(ary, nRow, nCol, str.substring(i, i+2)));

        return builder.toString();
    }

    String GetDencodingCodePacketChar(char[][] ary, int sRow, int sCol, String pvalue)
    {
        String result = "";

        int x = Integer.parseInt(pvalue.substring(0,1)) - 1;
        int y = Integer.parseInt(pvalue.substring(1,2)) - 1;
        try {result += ary[x][y];}catch (ArrayIndexOutOfBoundsException e){}

        return result;
    }

/// string enc = bizEncrypt.EncodingString("890", str);
/// <param name="mty">890 ~ 909</param>
    String EncodingPacketString(String mty, String str) {
        char[][] ary;
        if(mty.isEmpty())return "";

        if (mty.equals("890")) { ary = array890; }
        else if (mty.equals("891")) { ary = array891; }
        else if (mty.equals("892")) { ary = array892; }
        else if (mty.equals("893")) { ary = array893; }
        else if (mty.equals("894")) { ary = array894; }
        else if (mty.equals("895")) { ary = array895; }
        else if (mty.equals("896")) { ary = array896; }
        else if (mty.equals("897")) { ary = array897; }
        else if (mty.equals("898")) { ary = array898; }
        else if (mty.equals("899")) { ary = array899; }
        else if (mty.equals("900")) { ary = array900; }
        else if (mty.equals("901")) { ary = array901; }
        else if (mty.equals("902")) { ary = array902; }
        else if (mty.equals("903")) { ary = array903; }
        else if (mty.equals("904")) { ary = array904; }
        else if (mty.equals("905")) { ary = array905; }
        else if (mty.equals("906")) { ary = array906; }
        else if (mty.equals("907")) { ary = array907; }
        else if (mty.equals("908")) { ary = array908; }
        else if (mty.equals("909")) { ary = array909; }
        else return "";

        int nRow = ary[0].length;
        int nCol = ary.length;

        StringBuilder builder = new StringBuilder();

        for (int i = 0; i < str.length(); i++)
            builder.append(GetEncodingCodePacketChar(ary, nRow, nCol, str.substring(i, i+1)));

        builder.append(mty);
        return builder.toString();
    }

    String GetEncodingCodePacketChar(char[][] ary, int sRow, int sCol, String pvalue) {

        int idx[] = { 0, 0 };
        String result = "";

        for (int i = 0; i < sCol; i++)
        {
            for (int j = 0; j < sRow; j++)
            {
                if (pvalue.charAt(0) == ary[i][j])
                {
                    idx[0] = i + 1; idx[1] = j + 1;
                }
            }
        }
        result += idx[0];
        result += idx[1];

        return result;
    }

    /// <summary>
/// Packet ��ȣȭ. string denc = bizEncrypt.DecodingPacketString10("110", str);
/// </summary>
/// <param name="mty">110 ~ 119</param>
/// <param name="str">472337534367287571~</param>
/// <returns></returns>
    String DecodingPacketString10(String mty, String str)
    {
        char[][] ary;
        if(mty.isEmpty())return "";

        if (mty.equals("100")) { ary = array100; }
        else if (mty.equals("101")) { ary = array101; }
        else if (mty.equals("102")) { ary = array102; }
        else if (mty.equals("103")) { ary = array103; }
        else if (mty.equals("104")) { ary = array104; }
        else if (mty.equals("105")) { ary = array105; }
        else if (mty.equals("106")) { ary = array106; }
        else if (mty.equals("107")) { ary = array107; }
        else if (mty.equals("108")) { ary = array108; }
        else if (mty.equals("109")) { ary = array109; }
        else if (mty.equals("110")) { ary = array110; }
        else if (mty.equals("111")) { ary = array111; }
        else if (mty.equals("112")) { ary = array112; }
        else if (mty.equals("113")) { ary = array113; }
        else if (mty.equals("114")) { ary = array114; }
        else if (mty.equals("115")) { ary = array115; }
        else if (mty.equals("116")) { ary = array116; }
        else if (mty.equals("117")) { ary = array117; }
        else if (mty.equals("118")) { ary = array118; }
        else if (mty.equals("119")) { ary = array119; }
        else return "";

        int nRow = ary[0].length;
        int nCol = ary.length;

        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < str.length(); i += 4)
            builder.append(GetDencodingCodePacketChar10(ary, nRow, nCol, str.substring(i, i+4)));

        return builder.toString();
    }

    String GetDencodingCodePacketChar10(char[][] ary, int sRow, int sCol, String pvalue)
    {
        String result = "";
        int x = Integer.parseInt(pvalue.substring(0,2)) - 1;
        int y = Integer.parseInt(pvalue.substring(2,4)) - 1;
        try {result += ary[x][y];}catch (ArrayIndexOutOfBoundsException e){}
        return result;
    }

    /// <summary>
/// Packet ��ȣȭ. String enc = bizEncrypt.EncodingString10("110", str);
/// </summary>
/// <param name="mty">110 ~ 119</param>
/// <param name="str">wzms_ko-KR_h00001/WorkZone01/bt77/er00^00^00/tp26.20^ox20.30^ps1018</param>
/// <returns></returns>
    String EncodingPacketString10(String mty, String str)
    {
        char[][] ary;

        if (mty.equals("100")) { ary = array100; }
        else if (mty.equals("101")) { ary = array101; }
        else if (mty.equals("102")) { ary = array102; }
        else if (mty.equals("103")) { ary = array103; }
        else if (mty.equals("104")) { ary = array104; }
        else if (mty.equals("105")) { ary = array105; }
        else if (mty.equals("106")) { ary = array106; }
        else if (mty.equals("107")) { ary = array107; }
        else if (mty.equals("108")) { ary = array108; }
        else if (mty.equals("109")) { ary = array109; }
        else if (mty.equals("110")) { ary = array110; }
        else if (mty.equals("111")) { ary = array111; }
        else if (mty.equals("112")) { ary = array112; }
        else if (mty.equals("113")) { ary = array113; }
        else if (mty.equals("114")) { ary = array114; }
        else if (mty.equals("115")) { ary = array115; }
        else if (mty.equals("116")) { ary = array116; }
        else if (mty.equals("117")) { ary = array117; }
        else if (mty.equals("118")) { ary = array118; }
        else if (mty.equals("119")) { ary = array119; }
        else return "";

        int nRow = ary[0].length;
        int nCol = ary.length;

        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < str.length(); i++)
            builder.append(GetEncodingCodePacketChar10(ary, nRow, nCol, str.substring(i, i+1)));

        builder.append(mty);
        return builder.toString();
    }

    String GetEncodingCodePacketChar10(char[][] ary, int sRow, int sCol, String pvalue)
    {
        int idx[] = { 0, 0 };
        String result = "";

        for (int i = 0; i < sCol; i++)
        {
            for (int j = 0; j < sRow; j++)
            {
                if (pvalue.charAt(0) == ary[i][j])
                {
                    idx[0] = i + 1;
                    idx[1] = j + 1;
                }
            }
        }
        result = String.format("%02d%02d", idx[0], idx[1]);
        //Console.WriteLine("GetEncodingCodePacketChar = " + result);

        return result;
    }


    String packetEncode(String str, int mty)
    {
        String sEncode = "";
        String Mty = "";
        Mty += mty;
        if (890 <= mty && mty <= 909)
            sEncode = EncodingPacketString(Mty, str);
        else if (100 <= mty && mty <= 119)
            sEncode = EncodingPacketString10(Mty, str);
        return sEncode;
    }
    String packetDecode(String str)
    {
        String Mty = str.substring(str.length() - 3);
        int mty = Integer.parseInt(Mty);
        String sPacketData = "";
        str = str.substring(0, str.length() - 3);//data

        if (890 <= mty && mty <= 909)
            sPacketData = DecodingPacketString(Mty, str);
        else if (100 <= mty && mty <= 119)
            sPacketData = DecodingPacketString10(Mty, str);
        return sPacketData;
    }



    private char[][] array890 = { { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' } };
    private char[][] array891 = {
        { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
    };
    private char[][] array892 = {
        { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
    };
    private char[][] array893 = {
        { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
    };
    private char[][] array894 = {
        { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
    };
    private char[][] array895 = {
        { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
    };
    private char[][] array896 = {
        { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
    };
    private char[][] array897 = {
        { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
    };
    private char[][] array898 = {
        { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
    };
    private char[][] array899 = {
        { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '=', 'P' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
    };
    private char[][] array900 = {
        { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' } };
    private char[][] array901 = {
        { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
    };
    private char[][] array902 = {
        { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
    };
    private char[][] array903 = {
        { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
    };
    private char[][] array904 = {
        { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
    };
    private char[][] array905 = {
        { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
    };
    private char[][] array906 = {
        { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
    };
    private char[][] array907 = {
        { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
    };
    private char[][] array908 = {
        { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
    };
    private char[][] array909 = {
        { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '=', 'n' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
    };

    private char[][] array100 = {
        { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' } };
    private char[][] array101 = {
        { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
    };
    private char[][] array102 = {
        { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
    };
    private char[][] array103 = {
        { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
    };
    private char[][] array104 = {
        { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
    };
    private char[][] array105 = {
        { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
    };
    private char[][] array106 = {
        { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
    };
    private char[][] array107 = {
        { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
    };
    private char[][] array108 = {
        { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
                                   , { '+', '=', ';', '#', '@', '\'', ',', ':' }
    };
    private char[][] array109 = {
        { '+', '=', ';', '#', '@', '\'', ',', ':' }
                                   , { '/', '9', '3', 'q', 'F', 'X', 'm', 'e' }
                                   , { 'p', 'B', '^', 'U', 'f', '2', 'k', 'M' }
                                   , { '7', 'l', 'z', 'I', 'b', 'T', 'g', 'o' }
                                   , { 'E', '|', 'r', '!', 'v', '(', 'n', ')' }
                                   , { 'J', '1', 'c', 'y', 'H', 'Y', '~', 'P' }
                                   , { 'C', 'L', '_', 'V', 'h', '4', 'w', 'u' }
                                   , { 'N', 'O', 's', '5', 't', 'Q', 'Z', 'd' }
                                   , { 'K', 'G', 'R', '6', '-', 'x', 'A', 'S' }
                                   , { 'i', 'D', '8', 'a', '.', 'W', 'j', '0' }
    };
    private char[][] array110 = {
        { 'i', '+', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', '=', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', ';', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', '#', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', '@', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', '\'', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', ',', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', ':', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' } };
    private char[][] array111 = {
        { '0', 'o', '+', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '=', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', ';', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '#', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', '@', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', '\'', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', ',', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', ':', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
    };
    private char[][] array112 = {
        { 'j', 'g', 'm', '+', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', '=', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', ';', 'C', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', '#', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '@', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', '\'', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', ',', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', ':', '4', 'Q', '2', 'x', 'Y', '(' }
    };
    private char[][] array113 = {
        { 'W', 'T', 'X', '4', '+', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', '=', 'Z', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', ';', 'd', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', '#', 'N', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', '@', 'O', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', '\'', 's', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', ',', '5', 'U', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', ':', 't', 'f', '-', 'H', 'v' }
    };
    private char[][] array114 = {
        { '.', 'b', 'F', 'h', 't', '+', 'f', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '=', '2', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', ';', 'k', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', '#', 'M', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', '@', 'p', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', '\'', 'B', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', ',', '^', 'R', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', ':', 'U', '6', 'y', '!' }
    };
    private char[][] array115 = {
        { 'a', 'I', 'q', 'V', '5', 'U', '+', '6', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '=', '-', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', ';', 'x', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', '#', 'A', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', '@', 'S', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', '\'', 'K', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', ',', 'G', '1', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', ':', 'R', 'c', 'r' }
    };
    private char[][] array116 = {
        { '8', 'z', '3', '_', 's', '^', 'R', '+', 'c', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', '=', 'y', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', ';', 'H', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', '#', 'Y', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '@', '~', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', '\'', 'P', ')' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', ',', 'J', 'E' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', ':', '1', '|' }
    };
    private char[][] array117 = {
        { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '+', '|' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', '=', 'r' }
                                   , { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', ';', '!' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', '#', 'v' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '@', '(' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '~', '\'', 'n' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', '\0', ')' }// : ,
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', '\0', 'E' }
    };
    private char[][] array118 = {
                                    { 'a', 'I', 'q', 'V', '5', 'U', '6', 'y', '!', '+' }
                                   , { 'i', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E', '=' }
                                   , { 'W', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(', ';' }
                                   , { 'D', 'l', '9', 'L', 'O', 'B', 'G', '1', '|', '#' }
                                   , { 'j', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n', '@' }
                                   , { '8', 'z', '3', '_', 's', '^', 'R', 'c', 'r', '\'' }
                                   , { '.', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v', ',' }
                                   , { '0', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')', ':' }
    };
    private char[][] array119 = {
        { '0', '+', 'o', 'e', 'u', 'd', 'M', 'S', 'P', ')' }
                                   , { 'W', '=', 'T', 'X', '4', 'Q', '2', 'x', 'Y', '(' }
                                   , { 'a', ';', 'I', 'q', 'V', '5', 'U', '6', 'y', '!' }
                                   , { '8', '#', 'z', '3', '_', 's', '^', 'R', 'c', 'r' }
                                   , { 'D', '@', 'l', '9', 'L', 'O', 'B', 'G', '1', '|' }
                                   , { 'j', '\'', 'g', 'm', 'w', 'Z', 'k', 'A', '~', 'n' }
                                   , { '.', ',', 'b', 'F', 'h', 't', 'f', '-', 'H', 'v' }
                                   , { 'i', ':', '7', '/', 'C', 'N', 'p', 'K', 'J', 'E' }
    };
}
