#include <jni.h>
#include <string>
#include "android/log.h"
#include "cstdio"
#include "vector"

// 定义log 宏 输出  __VA_ARGS__表示可变参数 代表了前面括号里的"..."
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "JNI", __VA_ARGS__)
#define BUF_SIZE 1024
char buf[BUF_SIZE];

//把字符串s按照字符串c进行切分得到vector_v
static inline std::vector<std::string> split(const std::string &s, const std::string &c) {
    std::vector<std::string> v;
    int pos1 = 0, pos2;
    while ((pos2 = s.find(c, pos1)) != -1) {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
    return v;
}

// 相同给返回1 不同返回0
static inline int isEndWith(const char *str, const char *p) {
    int len1, len2;
    len1 = strlen(str);
    len2 = strlen(p);

    if (len2 <= 0)
        return 0;

    if (strncmp(str + len1 - len2, p, len2) == 0)
        return 1;

    return 0;
}


static inline int
isAnyPackageFromListInstalled(JNIEnv *env, std::vector<std::string> &packages) {
    //1 获取Activity Thread的实例对象
    jclass activityThread_class = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread_class,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject activityThread_obj = env->CallStaticObjectMethod(activityThread_class,
                                                             currentActivityThread);

    // 2 获取 ContextImpl -> 通过context的实现类ContextImpl来获取 packageManager
    jmethodID getSystemContext = env->GetMethodID(activityThread_class, "getSystemContext",
                                                  "()Landroid/app/ContextImpl;");
    jobject contextImpl_obj = env->CallObjectMethod(activityThread_obj, getSystemContext);
    jclass contextImpl_class = env->GetObjectClass(contextImpl_obj);

    // 3 获取packagemanager
    jmethodID getPackageManager = env->GetMethodID(contextImpl_class, "getPackageManager",
                                                   "()Landroid/content/pm/PackageManager;");
    jobject packageManager_obj = env->CallObjectMethod(contextImpl_obj, getPackageManager);
    jclass packageManager_class = env->GetObjectClass(packageManager_obj);


    //4 获取getPackageInfo方法
    jmethodID getPackageInfo = env->GetMethodID(packageManager_class, "getPackageInfo",
                                                "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

    int result = 0;
    for (int i = 0; i < packages.size(); i++) {

        env->CallObjectMethod(packageManager_obj, getPackageInfo,
                              env->NewStringUTF(packages[i].c_str()), 0);
        jboolean ret = env->ExceptionCheck();
        if (JNI_TRUE == ret) {
            // 打印Java层抛出的异常堆栈信息
            env->ExceptionDescribe();
            // 清除异常信息
            env->ExceptionClear();
            // ...这里可以处理异常发生逻辑
            // 发生异常不处理 未发生异常返回1 即 true
        } else {
            LOGE("ROOT_NATIVE_isAnyPackageFromListInstalled 检测到root ->:%s", packages[i].c_str());
            result = 1;
        };
    }
    env->DeleteLocalRef(activityThread_obj);
    env->DeleteLocalRef(activityThread_class);
    env->DeleteLocalRef(contextImpl_obj);
    env->DeleteLocalRef(contextImpl_class);
    env->DeleteLocalRef(packageManager_obj);
    env->DeleteLocalRef(packageManager_class);
    return result;
}

static inline int
detectRootManagementApps_Check(JNIEnv *env,
                               std::string additionalRootManagementApps[]) {
    std::vector<std::string> packages;
    packages.push_back("com.noshufou.android.su");
    packages.push_back("com.noshufou.android.su");
    packages.push_back("com.noshufou.android.su.elite");
    packages.push_back("eu.chainfire.supersu");
    packages.push_back("com.koushikdutta.superuser");
    packages.push_back("com.thirdparty.superuser");
    packages.push_back("com.yellowes.su");
    packages.push_back("com.example.ndktest");
    if (sizeof(additionalRootManagementApps) > 0) {
        for (int i = 0; i < sizeof(additionalRootManagementApps) / sizeof(std::string); i++) {
            packages.push_back(additionalRootManagementApps[i]);
        }
    }
    return isAnyPackageFromListInstalled(env, packages);
}

static inline int detectRootManagementApps(JNIEnv *env) {
    std::string additionalRootManagementApps[] = {};
    LOGE("ROOT_NATIVE＿detectRootManagementApps　检测root");
    return detectRootManagementApps_Check(env, additionalRootManagementApps);
}

static inline int
detectPotentiallyDangerousApps_Check(JNIEnv *env,
                                     std::string additionalDangerousApps[]) {

    std::vector<std::string> packages;
    packages.push_back("com.noshufou.android.su");
    packages.push_back("com.noshufou.android.su");
    packages.push_back("com.noshufou.android.su.elite");
    packages.push_back("eu.chainfire.supersu");
    packages.push_back("com.koushikdutta.superuser");
    packages.push_back("com.thirdparty.superuser");
    packages.push_back("com.yellowes.su");
    packages.push_back("com.example.ndktest");
    if (sizeof(additionalDangerousApps) > 0) {
        for (int i = 0; i < sizeof(additionalDangerousApps) / sizeof(std::string); i++) {
            packages.push_back(additionalDangerousApps[i]);
        }
    }
    return isAnyPackageFromListInstalled(env, packages);
}

static inline int detectPotentiallyDangerousApps(JNIEnv *env) {
    LOGE("ROOT_NATIVE＿detectPotentiallyDangerousApps　检测root");
    std::string additionalDangerousApps[] = {};
    return detectPotentiallyDangerousApps_Check(env, additionalDangerousApps);
}


static inline int checkForBinaryNative() {
    std::string suPath[] = {
            "/data/local/",
            "/data/local/bin/",
            "/data/local/xbin/",
            "/sbin/",
            "/su/bin/",
            "/system/bin/",
            "/system/bin/.ext/",
            "/system/bin/failsafe/",
            "/system/sd/xbin/",
            "/system/usr/we-need-root/",
            "/system/xbin/",
    };
    std::vector<std::string> pathsArray(suPath, suPath + (sizeof(suPath) / sizeof(std::string)));

    for (int i = 0; i < pathsArray.size(); i++) {
        std::string completePath = pathsArray[i].append("su");
        FILE *file = fopen(completePath.c_str(), "r");
        if (file != NULL) {
            LOGE("ROOT_NATIVE＿checkForBinaryNative　检测到root : %s", completePath.c_str());
            //文件存在就说明是root的 返回1 否则我返回false
            return 1;
        }
    }
    return 0;
}

static inline int checkForDangerousPropsNative() {

    std::string shell_pro = "getprop ro.debuggable";
    FILE *p_file = popen(shell_pro.c_str(), "r");
    while (fgets(buf, BUF_SIZE, p_file) != NULL) {
        std::string s = buf;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        if (s == "1") {
            LOGE("ROOT_NATIVE_checkForDangerousPropsNative 检测到root ");
            return 1;
        }
        LOGE("debuggable: %s", buf);
    }
    pclose(p_file);

    std::string shell_secure = "getprop ro.secure";
    FILE *s_file = popen(shell_secure.c_str(), "r");

    while (fgets(buf, BUF_SIZE, s_file) != NULL) {
        std::string s = buf;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        if (s == "0") {
            LOGE("ROOT_NATIVE_checkForDangerousPropsNative 检测到root ");
            return 1;
        }
        LOGE("secure : %s", buf);
    }
    pclose(s_file);
    return 0;
}

static inline int checkForRWPathsNative() {
    std::vector<std::string> paths = {
            "/system",
            "/system/bin",
            "/system/sbin",
            "/system/xbin",
            "/vendor/bin",
            "/sbin",
            "/etc"
    };
    std::string mount = "mount";
    FILE *p_mount = popen(mount.c_str(), "r");
    while (fgets(buf, BUF_SIZE, p_mount) != NULL) {
        std::string s = buf;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        LOGE("line: %s", s.c_str());
        std::vector<std::string> args = split(s, " ");
        for (int i = 0; i < args.size(); i++) {
            LOGE("split: %s a[%d]", args[i].c_str(), i);
        }
        if (args.size() < 4) {
            break;
        }
        std::string mountPoint = args[2];
        std::string mountOptions = args[5];
        for (int i = 0; i < paths.size(); i++) {
            if (strcasecmp(mountPoint.c_str(), paths[i].c_str()) == 0) {
                std::vector<std::string> a3Array = split(mountOptions, ",");
                for (int j = 0; j < a3Array.size(); j++) {
                    if (strcasecmp(a3Array[j].c_str(), "rw") == 0) {
                        LOGE("ROOT_NATIVE_heckForRWPaths 检测到root :%s", paths[i].c_str());
                        return 1;
                    }
                }
            }
        }
    }
    pclose(p_mount);
    return 0;
}

static inline int checkSuExists() {
    std::string shellwhich = "which su";
    FILE *p_which = popen(shellwhich.c_str(), "r");
    if (p_which == NULL) {
        return 0;
    }
    while (fgets(buf, BUF_SIZE, p_which) != NULL) {
        std::string s = buf;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        if (isEndWith(s.c_str(), "su")) {
            LOGE("ROOT_NATIVE_checkSuExists 检测到root :%s", s.c_str());
            return 1;
        }
    }
    return 0;
}


// 该函数与checkForBinaryNative实际执行内容相同
static inline int checkForRootNative() {
    std::vector<std::string> suPath = {
            "/data/local/",
            "/data/local/bin/",
            "/data/local/xbin/",
            "/sbin/",
            "/su/bin/",
            "/system/bin/",
            "/system/bin/.ext/",
            "/system/bin/failsafe/",
            "/system/sd/xbin/",
            "/system/usr/we-need-root/",
            "/system/xbin/",
    };
    int count = 0;
    for (int i = 0; i < suPath.size(); i++) {
        std::string completePath = suPath[i].append("su");
        FILE *file = fopen(completePath.c_str(), "r");
        if (file != NULL) {
            count++;
            LOGE("ROOT_NATIVE_checkForRootNative 检测到 root count: %d -> path:%s  ", count,
                 completePath.c_str()
            );
        }
    }
    if (count > 0) {
        return 1;
    }
    return count;
}

static inline int isRootNative(JNIEnv *env) {
    return /*detectRootManagementApps(env) ||
           detectPotentiallyDangerousApps(env) ||
           checkForBinaryNative() ||*/
           checkForDangerousPropsNative() ||
           checkForRWPathsNative() ||
           checkSuExists() ||
           checkForRootNative();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_nativecheckroot_RootUtils_checkRoot(JNIEnv *env, jobject thiz) {
    jboolean root = false;
    if (isRootNative(env)) {
        root = true;
    }
    return root;
}