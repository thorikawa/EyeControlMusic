package com.polysfactory.n3.jni;

import org.opencv.core.Mat;
import org.opencv.core.MatOfPoint3f;

public class NativeMarkerDetector {

    private long mNativeObj;

    public NativeMarkerDetector(float fx, float fy, float cx, float cy) {
        mNativeObj = nativeCreateObject(fx, fy, cx, cy);
    }

    public void findMarkers(Mat imageBgra, MatOfPoint3f cameraCentersMat, float scale) {
        nativeFindMarkers(mNativeObj, imageBgra.nativeObj, cameraCentersMat.nativeObj, scale);
    }

    public void release() {
        nativeDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private native long nativeCreateObject(float fx, float fy, float cx, float cy);

    private native void nativeFindMarkers(long thiz, long imageBgra, long transformations, float scale);

    private native void nativeDestroyObject(long thiz);
}
