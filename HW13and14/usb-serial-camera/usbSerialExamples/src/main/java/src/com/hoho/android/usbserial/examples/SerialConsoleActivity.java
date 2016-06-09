/* Copyright 2011-2013 Google Inc.
 * Copyright 2013 mike wakerly <opensource@hoho.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: https://github.com/mik3y/usb-serial-for-android
 */

package com.hoho.android.usbserial.examples;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;

/**
 * Monitors a single {@link UsbSerialPort} instance, showing all data
 * received.
 *
 * @author mike wakerly (opensource@hoho.com)
 */
public class SerialConsoleActivity extends Activity implements TextureView.SurfaceTextureListener {

    // camera stuff initializing.
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640,480,Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;
    private int mThreshold = -1;
    private int m_desiredCOM = 320;
    private int m_COMErr = 0;
    private int m_COMErrInt = 0;
    private double m_Kp = 0.22;
    private double m_Ki = 0.0;
    private int m_DesiredDutyCycle = 100;
    private SeekBar mThreshControl;
    private SeekBar mDesiredCOMBar;
    private TextView mThreshDisp;
    private int COM = m_desiredCOM;

    static long prevtime = 0; // for FPS calculation

    private final String TAG = SerialConsoleActivity.class.getSimpleName();

    /**
     * Driver instance, passed in statically via
     * {@link #show(Context, UsbSerialPort)}.
     *
     * <p/>
     * This is a devious hack; it'd be cleaner to re-create the driver using
     * arguments passed in with the {@link #startActivity(Intent)} intent. We
     * can get away with it because both activities will run in the same
     * process, and this is a simple demo.
     */
    private static UsbSerialPort sPort = null;

    private TextView mTitleTextView;
//    private TextView mDumpTextView;
//    private ScrollView mScrollView;
//    private CheckBox chkDTR;
//    private CheckBox chkRTS;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager mSerialIoManager;

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

        @Override
        public void onRunError(Exception e) {
            Log.d(TAG, "Runner stopped.");
        }

        @Override
        public void onNewData(final byte[] data) {
            SerialConsoleActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    SerialConsoleActivity.this.updateReceivedData(data);
                }
            });
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "In on create serial console");
        setContentView(R.layout.serial_console);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();
        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);
        mThreshControl= (SeekBar) findViewById(R.id.seek1);
        mThreshControl.setProgress(10);
        mTextView = (TextView) findViewById(R.id.cameraStatus);
        mThreshDisp = (TextView) findViewById(R.id.threshHoldVal);

        mDesiredCOMBar = (SeekBar) findViewById(R.id.seek2);

        setMyControlListener();

        paint1.setColor(0xffff0000); // red
        paint1.setTextSize(24);
//        mDumpTextView = (TextView) findViewById(R.id.consoleText);
//        mScrollView = (ScrollView) findViewById(R.id.demoScroller);
//        chkDTR = (CheckBox) findViewById(R.id.checkBoxDTR);
//        chkRTS = (CheckBox) findViewById(R.id.checkBoxRTS);

//        chkDTR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
//                try {
//                    sPort.setDTR(isChecked);
//                }catch (IOException x){}
//            }
//        });
//
//        chkRTS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
//                try {
//                    sPort.setRTS(isChecked);
//                }catch (IOException x){}
//            }
//        });

    }


    private void setMyControlListener(){
        mThreshControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int progressChanged = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                mThreshold = -progress;
                //mThreshDisp.setText("The value is :" + mThreshold);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        mDesiredCOMBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                m_desiredCOM = (int)(progress*6.4);
                mThreshDisp.setText("The value is :" + m_desiredCOM);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

    }
    @Override
    protected void onPause() {
        super.onPause();
        stopIoManager();
        if (sPort != null) {
            try {
                sPort.close();
            } catch (IOException e) {
                // Ignore.
            }
            sPort = null;
        }
        finish();
    }

    void showStatus(TextView theTextView, String theLabel, boolean theValue){
        String msg = theLabel + ": " + (theValue ? "enabled" : "disabled") + "\n";
        theTextView.append(msg);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "Resumed, port=" + sPort);
        if (sPort == null) {
            //mTitleTextView.setText("No serial device.");
            Log.d(TAG, "In on resume null");
        } else {
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

            UsbDeviceConnection connection = usbManager.openDevice(sPort.getDriver().getDevice());
            if (connection == null) {
               // mTitleTextView.setText("Opening device failed");
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

//                showStatus(mDumpTextView, "CD  - Carrier Detect", sPort.getCD());
//                showStatus(mDumpTextView, "CTS - Clear To Send", sPort.getCTS());
//                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
//                showStatus(mDumpTextView, "DTR - Data Terminal Ready", sPort.getDTR());
//                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
//                showStatus(mDumpTextView, "RI  - Ring Indicator", sPort.getRI());
//                showStatus(mDumpTextView, "RTS - Request To Send", sPort.getRTS());

            } catch (IOException e) {
                Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                //mTitleTextView.setText("Error opening device: " + e.getMessage());
                try {
                    sPort.close();
                } catch (IOException e2) {
                    // Ignore.
                }
                sPort = null;
                return;
            }
           // mTitleTextView.setText("Serial device: " + sPort.getClass().getSimpleName());
        }
        onDeviceStateChange();
    }

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            Log.i(TAG, "Stopping io manager ..");
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            Log.i(TAG, "Starting io manager ..");
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange() {
        Log.d(TAG, "In DEvice StateChange");
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
        final String message = "Read " + data.length + " bytes: \n"
                + HexDump.dumpHexString(data) + "\n\n";
//        mDumpTextView.append(message);
//        mScrollView.smoothScrollTo(0, mDumpTextView.getBottom());
//        byte[] sData = {'a',0};
//        try { sPort.write(sData, 10); }
//        catch (IOException e) { }
    }

    /**
     * Starts the activity, using the supplied driver instance.
     *
     * @param context
     * @param driver
     */
    static void show(Context context, UsbSerialPort port) {
        sPort = port;
        final Intent intent = new Intent(context, SerialConsoleActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        context.startActivity(intent);
    }
    static void showTest(Context context){
        final Intent intent = new Intent(context, SerialConsoleActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        context.startActivity(intent);
    }


    //All the camera code goes down below.

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        Log.d(TAG, "In tex available");
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setColorEffect(Camera.Parameters.EFFECT_NONE); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException e) {
            // Something bad happened
        }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int i, int i1) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {

            int[] pixels = new int[bmp.getWidth()];
            int startY = 30; // which row in the bitmap to analyse to read
            // only look at one row in the image
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1); // (array name, offset inside array, stride (size of row), start x, start y, num pixels to read per row, num rows to read)

            // pixels[] is the RGBA data (in black an white).
            // instead of doing center of mass on it, decide if each pixel is dark enough to consider black or white
            // then do a center of mass on the thresholded array
            int[] thresholdedPixels = new int[bmp.getWidth()];
            int [] actualThresholdedPixels = new int[bmp.getWidth()];
            int wbTotal = 0; // total mass
            int wbCOM = 0; // total (mass time position)

            for (int i = 0; i < bmp.getWidth(); i++) {
                if(red(pixels[i]) - green(pixels[i]) > mThreshold)
                {
                    thresholdedPixels[i] = 255*3; // Put all the "weight" of the line where the red is higher.
                    actualThresholdedPixels[i] = Color.rgb(255,0,0);
                }
                else
                {
                    thresholdedPixels[i] = 0;
                    actualThresholdedPixels[i] = Color.rgb(0,255,0);
                }
                wbTotal = wbTotal + thresholdedPixels[i];
                wbCOM = wbCOM + thresholdedPixels[i]*i;
            }
            bmp.setPixels(actualThresholdedPixels,0,bmp.getWidth(),0,startY,bmp.getWidth(),1);

            //watch out for divide by 0
//            if (wbTotal<=0) {
//                //COM = bmp.getWidth()/2;
//            }
//            else {
//                COM = wbCOM/wbTotal;
//            }
            if(wbTotal > 0)
            {
                COM = wbCOM/wbTotal;
            }

            // draw a circle where you think the COM is
            canvas.drawCircle(COM, startY, 5, paint1);
            // also write the value as text
            canvas.drawText("COM = " + COM, 10, 200, paint1);

            // send the error of current COM from the center of the image. approx ~320
            m_COMErr = COM - m_desiredCOM;
            m_COMErrInt = m_COMErrInt + m_COMErr;
            if(m_COMErrInt > 10000)
            {
                m_COMErrInt = 10000;
            }
            if(m_COMErrInt < -10000)
            {
                m_COMErrInt = -10000;
            }
            //Compute the direction of rotation. If it is to rotate left, send the duty cycle for left wheel and send it as negative. If it is to rotate right, send the duty cycle for right wheel as postitive
            if(m_COMErr <= 0) // The COM is on the left side. Which means slow down the left wheel so that the robot turns left
            {
                m_COMErr = -m_COMErr; // make error positive
                m_DesiredDutyCycle = (int)(100 - (m_Kp*m_COMErr + m_Ki*m_COMErrInt));
                if(m_DesiredDutyCycle < 10)
                {
                    m_DesiredDutyCycle = 10;
                }
                if(m_DesiredDutyCycle > 100)
                {
                    m_DesiredDutyCycle = 100;
                }
                m_DesiredDutyCycle = -m_DesiredDutyCycle;// since this is negative this will be applied to the left wheel
            }
            else // The COM is on the right side which means turn the robot right. Which means slow down the right wheel.
            {
                m_DesiredDutyCycle = (int)(100 - (m_Kp*m_COMErr + m_Ki*m_COMErrInt)); // since this is positive this will be applied to the right wheel
                if(m_DesiredDutyCycle < 10)
                {
                    m_DesiredDutyCycle = 10;
                }
                if(m_DesiredDutyCycle > 100)
                {
                    m_DesiredDutyCycle = 100;
                }
                //already positive
            }


            String COMString = String.valueOf(m_DesiredDutyCycle) + "\n";
            try {
                sPort.write(COMString.getBytes(), 10);
            }
            catch(IOException ioe){}
            c.drawBitmap(bmp, 0, 0, null);

            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
//            long nowtime = System.currentTimeMillis();
//            long diff = nowtime - prevtime;
////            mTextView.setText("FPS " + 1000/diff);
//            prevtime = nowtime;
        }
    }



}

