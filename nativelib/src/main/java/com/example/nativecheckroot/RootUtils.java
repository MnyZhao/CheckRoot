package com.example.nativecheckroot;

public class RootUtils {
    static {
        System.loadLibrary("checkroot");
    }

    private static RootUtils instance;

    public static RootUtils getInstance() {
        if (instance == null) {
            instance = new RootUtils();
        }
        return instance;
    }

    public native boolean checkRoot();
}
