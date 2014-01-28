package com.polysfactory.newtype;

import java.io.IOException;
import java.util.List;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.JavaCameraViewEx;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfPoint3f;
import org.opencv.core.Point;
import org.opencv.core.Point3;
import org.opencv.core.Scalar;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.Window;
import android.view.WindowManager;

import com.polysfactory.n3.jni.NativeMarkerDetector;

public class MainActivity extends Activity implements CvCameraViewListener2 {

    private GlassConnection mGlassConnection;
    private JavaCameraViewEx mCameraView;
    private NativeMarkerDetector mMarkerDetector;
    private static final String TARGET = "22:13:04:16:58:32";
    private static final float[] mCameraParameters = { 357.658935546875F, 357.658935546875F, 319.5F, 179.5F };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON | WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main);

        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mGlassConnection = new GlassConnection(bluetoothAdapter);

        mMarkerDetector = new NativeMarkerDetector(mCameraParameters[0], mCameraParameters[1], mCameraParameters[2],
                mCameraParameters[3]);

        mCameraView = (JavaCameraViewEx) findViewById(R.id.fd_activity_surface_view);
        mCameraView.setCameraIndex(C.CAMERA_INDEX);
        mCameraView.setCvCameraViewListener(this);
        mCameraView.setMaxFrameSize(C.MAX_FRAME_SIZE_WIDTH, C.MAX_FRAME_SIZE_HEIGHT);
        mCameraView.enableFpsMeter();

    }

    @Override
    protected void onStart() {
        super.onStart();
        try {
            mGlassConnection.connect(TARGET);
            mGlassConnection.startReaderThread();
        } catch (IOException e) {
            Log.w(C.TAG, "error while connecting", e);
        }
        if (mCameraView != null) {
            mCameraView.enableView();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        mGlassConnection.close();
        if (mCameraView != null) {
            mCameraView.disableView();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public Mat onCameraFrame(CvCameraViewFrame frame) {
        Mat rgba = frame.rgba();
        if (mMarkerDetector != null) {
            // mMarkerDetector.findMarkers(mFrame, transformations);
            // Log.d(C.TAG, "mScale=" + mScale);
            MatOfPoint3f cameraCenters = new MatOfPoint3f();
            mMarkerDetector.findMarkers(rgba, cameraCenters, 1.0F);
            List<Point3> list = cameraCenters.toList();
            if (list.size() > 0) {
                // TODO
                Point3 center = list.get(0);
                if (center.y < -0.6 && center.y > -1.2) {
                    double x = center.x + 1.5F;
                    int chord = (int) (x / 0.4);
                    if (chord < 8) {
                        char c = (char) ('C' + chord);
                        Log.d(C.TAG, "send:" + c);
                        if (mGlassConnection != null && mGlassConnection.isConnected()) {
                            try {
                                mGlassConnection.write(new byte[] { (byte) c });
                            } catch (IOException e) {
                                Log.w(C.TAG, "write error", e);
                            }
                        }
                    }
                }
                Log.d(C.TAG, "center: " + center.toString());
            } else {
                // TODO
            }
            Core.rectangle(rgba, new Point(300, 160), new Point(340, 200), new Scalar(255, 0, 0));
        }
        return rgba;
    }

    @Override
    public void onCameraViewStarted(int arg0, int arg1) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCameraViewStopped() {
        // TODO Auto-generated method stub

    }

}
