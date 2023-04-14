package com.example.inout_nodeapp;

import android.annotation.SuppressLint;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseSettings;
import android.content.pm.PackageManager;
import android.util.Log;

import org.altbeacon.beacon.Beacon;
import org.altbeacon.beacon.BeaconParser;
import org.altbeacon.beacon.BeaconTransmitter;

public class BLEBeacon {
    Beacon beacon;
    BeaconParser beaconParser;
    BeaconTransmitter beaconTransmitter;
    BackgroundService parent;

    boolean isBLEEnabled = true;

    interface BLEStatChangeListener {
        void onChange(int BLEStat);
    }
    BLEStatChangeListener BLEStatChangelistener;
    String data;
    byte[] datafield;
    // -1 : not initalized
    // 0 : intalizing BLE 1:failed 2:succeed
    int BLEstatus = -1;
    int aesNo = 0;

    void setBLEStatus(int BLEStat)
    {
        BLEstatus = BLEStat;
        if(BLEStatChangelistener != null)BLEStatChangelistener.onChange(BLEstatus);
    }

    BLEBeacon(BackgroundService p_parent, BLEStatChangeListener listener){
        parent = p_parent;
        BLEStatChangelistener = listener;
        int flag = BeaconTransmitter.checkTransmissionSupported(parent);
        if(flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER || flag == BeaconTransmitter.NOT_SUPPORTED_CANNOT_GET_ADVERTISER_MULTIPLE_ADVERTISEMENTS)
        {
            setBLEStatus(-1);
            Log.e("BEACON","LE Adapter in not available!!");
        }else if(!parent.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE))
        {
            isBLEEnabled = false;
            setBLEStatus(-2);
            Log.e("BEACON","LE Adapter CAN'T be used!!");
        }
        else isBLEEnabled = true;
        Log.e("BEACON","isBLEEnabled : " + isBLEEnabled);

    }
    void makedata(byte[] binary)
    {
        datafield = binary;
        StringBuilder out =  new StringBuilder();
        for (int i=0;i<binary.length;i++) {
            out.append(String.format("%02x", binary[i]));
        }

        data = out.toString();
    }
    void onCreate(String uuid)
    {
        if(!isBLEEnabled)return;
        onDestory();
        setBLEStatus(0);

        Log.w("BEACON",String.format("UUID:%s",uuid));
        BLEStart();
    }
    @SuppressLint("DefaultLocale")
    void BLEStart()
    {
        beacon = new Beacon.Builder()
                .setId1(data)
                .setManufacturer(0xEFFF)
                .setTxPower(-66) // Power in dB	  -66 dBm
                /*.setBluetoothName(parent.getString(R.string.app_name))
                //.setDataFields(new ArrayList<Byte>())
                .setId1(data)
                .setTxPower(-66) // Power in dB	  -66 dBm
                .setManufacturer(0xEFFF) // AJUAttender 0xEFFF
                */

                /*.setId1(data)m:0-15:8d346b0ae7ac3ef91d4d4cc71c1c1d8e
                setId1(uuid) // UUID for beacon "2f234454-cf6d-4a0f-adf2-f4911ba9ffa6"
                .setId2("1") // Major for beacon
                .setId3("5") // Minor for beacon
                .setManufacturer(0x004C) // Radius Networks.0x0118  Change this for other beacon layouts//0x004C for iPhone
                .setDataFields(Arrays.asList(new Long[] {0l})) // Remove this for beacon layouts without d: fields
                */
                .build();

        Log.i("Beacon",data);
                /*
        final int set = 4;
        int iterator = (datafield.length+set-1) / set;
        String[] builder = new String[iterator];
        StringBuilder all =  new StringBuilder();
        for(int i=0;i<iterator;i++)
        {
            byte[] now = new byte[set];
            System.arraycopy(datafield,i*set,now,0,set);

            StringBuilder out =  new StringBuilder();
            for( byte b : now)
                out.append(String.format("%02X",b));
            builder[i] = out.toString();
            all.append(String.format("m:%d-%d=%s,",i*set+2,i*set+set-1+2,builder[i]));
        }
        all.append(String.format("p:%d-%d",iterator*set+2,iterator*set+2));
*/

        beaconParser = new BeaconParser()
                //.setBeaconLayout("m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24");
                //.setBeaconLayout("i:2-17,p:18-18");//2-17 : 17 <= 2 + length - 1
                //.setBeaconLayout(String.format("d:2-%d,p:%d-%d",data.length()+1,data.length()+2,data.length()+2));//2-17 : 17 <= 2 + length - 1
                .setBeaconLayout(String.format("m:2-2=%02X,i:3-%d,p:%d-%d",aesNo-1,3+datafield.length-1,datafield.length+3,datafield.length+3));//2-17 : 17 <= 2 + length - 1
                //.setBeaconLayout(String.format("m:2-2=%02X,i:3-18,p:19-19",aesNo));//2-17 : 17 <= 2 + length - 1
                //.setBeaconLayout("m:2-17=0102030405060708090A0B0C0E0F,p:18-18");
                //.setBeaconLayout("m:2-9=0102030405060708,m:10-17=0102030405060708,p:18-18");
                //.setBeaconLayout(all.toString());
        //4C00 0215 FF166526A9C940D3AFF5509CF9BC8381 0001 0005 C8

        if(beaconParser == null)
        {
            Log.e("TAG", "beaconParser Setting Error!");
            setBLEStatus(1);
            parent.RequireUpdateTextview();
            return;
        }

        if(BeaconTransmitter.checkTransmissionSupported(parent) != BeaconTransmitter.SUPPORTED)
        {
            Log.e("TAG", "LE is not supported!");
            setBLEStatus(1);
            parent.RequireUpdateTextview();
            return;
        }

        beaconTransmitter = new BeaconTransmitter(parent,beaconParser);
        beaconTransmitter.setAdvertiseTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_MEDIUM);//	-66 dBm
        beaconTransmitter.startAdvertising(beacon, new AdvertiseCallback() {

            @Override
            public void onStartFailure(int errorCode) {
                Log.e("TAG", "Advertisement start failed with code: "+errorCode);//"Advertisement start failed with code: "+errorCode
                setBLEStatus(1);
                parent.RequireUpdateTextview();

                //parent.MessageSnack("StartFailure","Advertisement start failed with code: "+errorCode);

                beaconTransmitter.stopAdvertising();
            }

            @Override
            public void onStartSuccess(AdvertiseSettings settingsInEffect) {
                Log.i("TAG", "Advertisement start succeeded.");
                setBLEStatus(2);
                parent.RequireUpdateTextview();

                //parent.MessageSnack("StartSuccess","Advertisement start succeeded.");
            }
        });
    }

    void onDestory()
    {
        if(BLEStatChangelistener != null)BLEStatChangelistener.onChange(BLEstatus);
        if(!isBLEEnabled)return;

        if(beaconTransmitter == null)return;
        beaconTransmitter.stopAdvertising();
        //if(BLEstatus == 0 || isBLEON())
        //    beaconTransmitter.stopAdvertising();
        setBLEStatus(-1);
    }

    boolean isBLEON()
    {
        return BLEstatus == 2;
    }

    static String strBLEStatus(int BLEstatus)
    {
        String BLEstat;
        switch(BLEstatus)
        {
            case -2: BLEstat = "NO"; break;
            case 0: BLEstat = "Turning ON.."; break;
            case 1: BLEstat = "OFF(fail)"; break;
            case 2: BLEstat = "ON"; break;
            default:BLEstat = "OFF"; break;
        }
        return BLEstat;
    }
}
