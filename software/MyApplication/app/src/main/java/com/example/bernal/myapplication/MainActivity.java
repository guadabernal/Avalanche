package com.example.bernal.myapplication;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.UUID;

import static java.lang.Math.abs;

public class MainActivity extends AppCompatActivity {
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    String address = null;
    private ProgressDialog progress;
    private boolean isBtConnected = false;
    private boolean sendChangeToArduino = false;

    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    Button btnDis;
    Joystick jsLeft;
    RelativeLayout layoutJsLeft;
    TextView status;
    GamepadController gp;
    boolean sendData = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS); //receive the address of the bluetooth device
        setContentView(R.layout.activity_main);
        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        layoutJsLeft = (RelativeLayout)findViewById(R.id.layout_joystick);
        status = (TextView) findViewById(R.id.textView2);
        jsLeft = new Joystick(getApplicationContext(), layoutJsLeft, R.drawable.image_button);

        gp = new GamepadController();
        new ConnectBT().execute(); //Call the class to connect
        new Thread(new Runnable() {
            public void run(){
                SystemClock.sleep(2000);
                while(true) {
                    if (btSocket != null && isBtConnected == true) {
                        if (sendChangeToArduino) {
                            setThrottle((short) (1024), (short) (1024));
                            sendData = !sendData;
                            sendChangeToArduino = false;
                        }
                        if (sendData == true) {
                            float yl = -1 * gp.getJoystickPosition(0, GamepadController.AXIS_Y) * 255;
                            float yr = -1 * gp.getJoystickPosition(1, GamepadController.AXIS_Y) * 255;
                            if (abs(yl) > 10 || abs(yr) > 10) setThrottle((short) (yl), (short) (yr));
                        }
                    }
                    SystemClock.sleep(20);
                }
            }
        }).start();

        btnDis = (Button)findViewById(R.id.btnDisconnect);
        btnDis.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("Disconnecting", "Disconnecting");
                Disconnect();
            }
        });
    }

    byte [] ShortToByte_ByteBuffer_Method(short [] input)
    {
        int index;
        int iterations = input.length;

        ByteBuffer bb = ByteBuffer.allocate(input.length * 2);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        for(index = 0; index != iterations; ++index)
            bb.putShort(input[index]);
        return bb.array();
    }

    void setThrottle(short l, short r) {
        try {
            DataOutputStream out = new DataOutputStream(btSocket.getOutputStream());
            short[] v = {l, r};
            byte[] b = ShortToByte_ByteBuffer_Method(v);
            Log.e("setThrottle", "set " + v[0] + " - " +v[1]);
            out.write(b);
        }
        catch (IOException e)
        {
         //   msg("Error");
        }
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
        gp.setDeviceId(event.getDeviceId());
        gp.handleMotionEvent(event);
        return true;
    }

    @Override
    public boolean onKeyDown(int keycode, KeyEvent event)
    {
        gp.setDeviceId(event.getDeviceId());
        gp.handleKeyEvent(event);
        if(gp.isButtonDown(GamepadController.BUTTON_A)) {
            sendChangeToArduino = true;
            sendData = !sendData;
            if (sendData == true) {
                msg("Gamepad Enabled");
            } else {
                msg("Gamepad Disabled");
            }
        }
        return true;
    }


    @Override
    public boolean onKeyUp(int keycode, KeyEvent event)
    {
        gp.setDeviceId(event.getDeviceId());
        gp.handleKeyEvent(event);
        return true;
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                isBtConnected = false;
                SystemClock.sleep(2000);
                btSocket.close(); //close connect
                btSocket = null;
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish(); //return to the first layout

    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_activity_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true; //if it's here, it's almost connected

        @Override
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(MainActivity.this, "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice device = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = device.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }


}
