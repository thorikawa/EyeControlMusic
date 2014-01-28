package com.polysfactory.newtype;

import org.opencv.android.OpenCVLoader;

import android.app.Application;

public class NewTypeApplication extends Application {
    static {
        if (!OpenCVLoader.initDebug()) {
            // Handle initialization error
        } else {
            System.loadLibrary("n3_marker");
            System.loadLibrary("newtype");
        }
    }

}
