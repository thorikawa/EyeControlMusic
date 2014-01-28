package com.polysfactory.newtype;

import org.opencv.android.CameraBridgeViewBase;

public class C {
    public static final String TAG = "NewType";
    public static final int CAMERA_INDEX;
    public static final boolean FLIP;
    static {
        CAMERA_INDEX = CameraBridgeViewBase.CAMERA_ID_BACK;
        FLIP = false;
    }
    // glass 640*360
    public static final int MAX_FRAME_SIZE_WIDTH = 640;
    public static final int MAX_FRAME_SIZE_HEIGHT = 360;
}
