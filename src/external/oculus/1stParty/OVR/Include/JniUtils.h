// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/************************************************************************************

Filename    :   JniUtils.h
Content     :   JNI utility functions
Created     :   October 21, 2014
Authors     :   J.M.P. van Waveren, Jonathan E. Wright

*************************************************************************************/

#ifndef OVR_JniUtils_h
#define OVR_JniUtils_h

#include "OVR_Types.h"

#if defined(OVR_OS_ANDROID)
#include "OVR_LogUtils.h"
#endif // defined(OVR_OS_ANDROID)

//==============================================================
// Various helper functions.
//==============================================================

enum {
    ANDROID_KITKAT = 19, // Build.VERSION_CODES.KITKAT
    ANDROID_KITKAT_WATCH = 20, // Build.VERSION_CODES.KITKAT_WATCH
    ANDROID_LOLLIPOP = 21, // Build.VERSION_CODES.LOLLIPOP
    ANDROID_LOLLIPOP_MR1 = 22, // Build.VERSION_CODES.LOLLIPOP_MR1
    ANDROID_MARSHMALLOW = 23, // Build.VERSION_CODES.M
    ANDROID_NOUGAT = 24, // Build.VERSION_CODES.N
    ANDROID_N_MR1 = 25, // Build.VERSION_CODES.N_MR1
};

#if defined(OVR_OS_ANDROID)
// JVM's AttachCurrentThread() resets the thread name if the thread name is not passed as an
// argument. Use these wrappers to attach the current thread to a JVM without resetting the current
// thread name. These will fatal error when unsuccesful.
jint ovr_AttachCurrentThread(JavaVM* vm, JNIEnv** jni, void* args);
jint ovr_DetachCurrentThread(JavaVM* vm);
#else
inline jint ovr_AttachCurrentThread(JavaVM* vm, JNIEnv** jni, void* args) {
    OVR_UNUSED(vm);
    OVR_UNUSED(jni);
    OVR_UNUSED(args);
    return 0;
}
inline jint ovr_DetachCurrentThread(JavaVM* vm) {
    OVR_UNUSED(vm);
    return 0;
}
#endif

#if defined(OVR_OS_ANDROID)
//==============================================================
// TempJniEnv
//
// Gets a temporary JNIEnv for the current thread and releases the
// JNIEnv on destruction if the calling thread was not already attached.
//==============================================================
class TempJniEnv {
   public:
    explicit TempJniEnv(JavaVM* vm, const char* /*file*/ = "<unspecified>", int /*line*/ = -1);
    ~TempJniEnv();

    operator JNIEnv*() {
        return jni_;
    }
    JNIEnv* operator->() {
        return jni_;
    }

   private:
    JavaVM* vm_;
    JNIEnv* jni_;
    bool privateEnv_;
};

#define JNI_TMP_ENV(envVar, vm) TempJniEnv envVar(vm, __FILE__, __LINE__)

#endif

//==============================================================
// JavaObject
//
// Releases a java object local reference on destruction.
//==============================================================
class JavaObject {
   public:
    JavaObject(JNIEnv* jni_, jobject const object_) : Jni(jni_), Object(object_) {
        OVR_ASSERT(Jni != NULL);
    }
    ~JavaObject() {
#if defined(OVR_OS_ANDROID)
        if (Jni->ExceptionOccurred()) {
            OVR_LOG("JNI exception before DeleteLocalRef!");
            Jni->ExceptionClear();
        }
        OVR_ASSERT(Jni != NULL && Object != NULL);
        Jni->DeleteLocalRef(Object);
        if (Jni->ExceptionOccurred()) {
            OVR_LOG("JNI exception occurred calling DeleteLocalRef!");
            Jni->ExceptionClear();
        }
#endif
        Jni = NULL;
        Object = NULL;
    }

    jobject GetJObject() const {
        return Object;
    }

    JNIEnv* GetJNI() const {
        return Jni;
    }

   protected:
    void SetJObject(jobject const& obj) {
        Object = obj;
    }

   private:
    JNIEnv* Jni;
    jobject Object;
};

//==============================================================
// JavaClass
//
// Releases a java class local reference on destruction.
//==============================================================
class JavaClass : public JavaObject {
   public:
    JavaClass(JNIEnv* jni_, jobject const class_) : JavaObject(jni_, class_) {}

    jclass GetJClass() const {
        return static_cast<jclass>(GetJObject());
    }
};

#if defined(OVR_OS_ANDROID)
//==============================================================
// JavaFloatArray
//
// Releases a java float array local reference on destruction.
//==============================================================
class JavaFloatArray : public JavaObject {
   public:
    JavaFloatArray(JNIEnv* jni_, jfloatArray const floatArray_) : JavaObject(jni_, floatArray_) {}

    jfloatArray GetJFloatArray() const {
        return static_cast<jfloatArray>(GetJObject());
    }
};

//==============================================================
// JavaObjectArray
//
// Releases a java float array local reference on destruction.
//==============================================================
class JavaObjectArray : public JavaObject {
   public:
    JavaObjectArray(JNIEnv* jni_, jobjectArray const floatArray_) : JavaObject(jni_, floatArray_) {}

    jobjectArray GetJobjectArray() const {
        return static_cast<jobjectArray>(GetJObject());
    }
};
#endif

//==============================================================
// JavaString
//
// Creates a java string on construction and releases it on destruction.
//==============================================================
class JavaString : public JavaObject {
   public:
    JavaString(JNIEnv* jni_, char const* string_) : JavaObject(jni_, NULL) {
#if defined(OVR_OS_ANDROID)
        SetJObject(GetJNI()->NewStringUTF(string_));
        if (GetJNI()->ExceptionOccurred()) {
            OVR_LOG("JNI exception occurred calling NewStringUTF!");
        }
#else
        OVR_UNUSED(string_);
#endif
    }

    JavaString(JNIEnv* jni_, jstring string_) : JavaObject(jni_, string_) {
        OVR_ASSERT(string_ != NULL);
    }

    jstring GetJString() const {
        return static_cast<jstring>(GetJObject());
    }
};

//==============================================================
// JavaUTFChars
//
// Gets a java string object as a buffer of UTF characters and
// releases the buffer on destruction.
// Use this only if you need to store a string reference and access
// the string as a char buffer of UTF8 chars.  If you only need
// to store and release a reference to a string, use JavaString.
//==============================================================
class JavaUTFChars : public JavaString {
   public:
    JavaUTFChars(JNIEnv* jni_, jstring const string_) : JavaString(jni_, string_), UTFString(NULL) {
#if defined(OVR_OS_ANDROID)
        UTFString = GetJNI()->GetStringUTFChars(GetJString(), NULL);
        if (GetJNI()->ExceptionOccurred()) {
            OVR_LOG("JNI exception occurred calling GetStringUTFChars!");
        }
#endif
    }

    ~JavaUTFChars() {
#if defined(OVR_OS_ANDROID)
        OVR_ASSERT(UTFString != NULL);
        GetJNI()->ReleaseStringUTFChars(GetJString(), UTFString);
        if (GetJNI()->ExceptionOccurred()) {
            OVR_LOG("JNI exception occurred calling ReleaseStringUTFChars!");
        }
#endif
    }

    char const* ToStr() const {
        return UTFString;
    }
    operator char const *() const {
        return UTFString;
    }

   private:
    char const* UTFString;
};

#if defined(OVR_OS_ANDROID)
#include <unistd.h>
#include <pthread.h>

inline int ovr_GetBuildVersionSDK(JNIEnv* jni) {
    int buildVersionSDK = 0;

    jclass versionClass = jni->FindClass("android/os/Build$VERSION");
    if (versionClass != 0) {
        jfieldID sdkIntFieldID = jni->GetStaticFieldID(versionClass, "SDK_INT", "I");
        if (sdkIntFieldID != 0) {
            buildVersionSDK = jni->GetStaticIntField(versionClass, sdkIntFieldID);
        }
        jni->DeleteLocalRef(versionClass);
    }
    return buildVersionSDK;
}

// Use this EVERYWHERE and you can insert your own catch here if you have string references leaking.
// Even better, use the JavaString / JavaUTFChars classes instead and they will free resources for
// you automatically.
inline jobject ovr_NewStringUTF(JNIEnv* jni, char const* str) {
    return jni->NewStringUTF(str);
}
inline char const* ovr_GetStringUTFChars(JNIEnv* jni, jstring javaStr, jboolean* isCopy) {
    char const* str = jni->GetStringUTFChars(javaStr, isCopy);
    return str;
}

// Every Context or its subclass (ex: Activity) has a ClassLoader
inline jclass ovr_GetClassLoader(JNIEnv* jni, jobject contextObject) {
    JavaClass contextClass(jni, jni->GetObjectClass(contextObject));
    jmethodID getClassLoaderMethodId =
        jni->GetMethodID(contextClass.GetJClass(), "getClassLoader", "()Ljava/lang/ClassLoader;");
    jclass localClass =
        static_cast<jclass>(jni->CallObjectMethod(contextObject, getClassLoaderMethodId));

    if (localClass == 0) {
        OVR_FAIL("getClassLoaderFailed failed");
    }

    return localClass;
}

// FindClass uses the current callstack to determine which ClassLoader object to use
// to start the class search. As a result, an attempt to find an app-specific class will
// fail when FindClass is called from an arbitrary thread with the JavaVM started in
// the "system" class loader, instead of the one associated with the application.
// The following two functions can be used to find any class from any thread. These will
// fatal error if the class is not found.
inline jclass ovr_GetLocalClassReferenceWithLoader(
    JNIEnv* jni,
    jobject classLoader,
    const char* className,
    const bool fail = true,
    const bool warn = true) {
    if (jni == nullptr) {
        if (fail) {
            OVR_FAIL(
                "ovr_GetLocalClassReferenceWithLoader: JNIEnv is NULL. Classname is %s", className);
        } else if (warn) {
            OVR_WARN(
                "ovr_GetLocalClassReferenceWithLoader: JNIEnv is NULL. Classname is %s", className);
        }
        return nullptr;
    }
    // This is a lambda because if it were at file scope it would need to be inlined since
    // this is a header, and that could mean the errorMsg[256] declaration could take up
    // 256 bytes on the stack each time the function is called (which is currently 3x in
    // this function). Whether or not that is actually the case is up to the compiler and
    // optimizations, but C++ doesn't guarantee stack variables only take up space within
    // the code block in which they are declared.
    auto checkJNIException =
        [](JNIEnv* jniLambdaScope, const bool warnLambdaScope, const char* fmt, ...) {
            if (jniLambdaScope->ExceptionOccurred()) {
                // something else caused a JNI exception before this and didn't clear it
                if (warnLambdaScope && fmt != nullptr) {
                    char errorMsg[256];
                    va_list argPtr;
                    va_start(argPtr, fmt);
                    vsnprintf(errorMsg, sizeof(errorMsg), fmt, argPtr);
                    va_end(argPtr);
                    OVR_LOG("JNI exception: %s", errorMsg);
                }
                jniLambdaScope->ExceptionClear();
            }
        };

    checkJNIException(
        jni,
        warn,
        "UNEXPECTED before FindClass( '%s' ) in ovr_GetLocalClassReferenceWithLoader",
        className);

    JavaClass classLoaderClass(jni, jni->FindClass("java/lang/ClassLoader"));
    jmethodID loadClassMethodId = jni->GetMethodID(
        classLoaderClass.GetJClass(), "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    JavaString classNameString(jni, jni->NewStringUTF(className));
    jclass localClass = static_cast<jclass>(
        jni->CallObjectMethod(classLoader, loadClassMethodId, classNameString.GetJString()));

    if (localClass == 0) {
        if (fail) {
            OVR_FAIL("FindClass( %s ) failed", className);
        } else if (warn) {
            OVR_WARN("FindClass( %s ) failed", className);
        }
        checkJNIException(jni, warn, "class '%s' not found.", className);
    } else {
        // We shouldn't get an exception if the class was found.
        checkJNIException(
            jni,
            warn,
            "UNEXPECTED after loadClass( '%s' ) in ovr_GetLocalClassReferenceWithLoader!",
            className);
    }

    return localClass;
}
inline jclass
ovr_GetGlobalClassReferenceWithLoader(JNIEnv* jni, jobject classLoader, const char* className) {
    JavaClass localClass(jni, ovr_GetLocalClassReferenceWithLoader(jni, classLoader, className));

    // Turn it into a global reference so it can be safely used on other threads.
    jclass globalClass = (jclass)jni->NewGlobalRef(localClass.GetJClass());
    return globalClass;
}

// The following two functions can be used to find any class from any thread, but they do
// need the activity object to explicitly start the class search using the class loader
// associated with the application. These will fatal error if the class is not found.

// This can be called from any thread but does need the activity object.
inline jclass ovr_GetLocalClassReference(
    JNIEnv* jni,
    jobject activityObject,
    const char* className,
    const bool fail = true,
    const bool warn = true) {
    JavaObject classLoaderObject(jni, ovr_GetClassLoader(jni, activityObject));

    return ovr_GetLocalClassReferenceWithLoader(
        jni, classLoaderObject.GetJObject(), className, fail, warn);
}
// This can be called from any thread but does need the activity object.
inline jclass ovr_GetGlobalClassReference(
    JNIEnv* jni,
    jobject activityObject,
    const char* className,
    const bool fail = true,
    const bool warn = true) {
    JavaClass localClass(
        jni, ovr_GetLocalClassReference(jni, activityObject, className, fail, warn));

    // Turn it into a global reference so it can be safely used on other threads.
    jclass globalClass = (jclass)jni->NewGlobalRef(localClass.GetJClass());
    return globalClass;
}

// These will fatal error if the method is not found.
inline jmethodID
ovr_GetMethodID(JNIEnv* jni, jclass jniclass, const char* name, const char* signature) {
    const jmethodID methodId = jni->GetMethodID(jniclass, name, signature);
    if (!methodId) {
        OVR_FAIL("couldn't get %s, %s", name, signature);
    }
    return methodId;
}
inline jmethodID
ovr_GetStaticMethodID(JNIEnv* jni, jclass jniclass, const char* name, const char* signature) {
    const jmethodID method = jni->GetStaticMethodID(jniclass, name, signature);
    if (!method) {
        OVR_FAIL("couldn't get %s, %s", name, signature);
    }
    return method;
}

// Get the code path of the current package.
inline const char* ovr_GetPackageCodePath(
    JNIEnv* jni,
    jobject activityObject,
    char* packageCodePath,
    int const maxLen) {
    if (packageCodePath == NULL || maxLen < 1) {
        return packageCodePath;
    }

    packageCodePath[0] = '\0';

    JavaClass activityClass(jni, jni->GetObjectClass(activityObject));
    jmethodID getPackageCodePathId =
        jni->GetMethodID(activityClass.GetJClass(), "getPackageCodePath", "()Ljava/lang/String;");
    if (getPackageCodePathId == 0) {
        OVR_LOG(
            "Failed to find getPackageCodePath on class %llu, object %llu",
            (long long unsigned int)activityClass.GetJClass(),
            (long long unsigned int)activityObject);
        return packageCodePath;
    }

    JavaUTFChars result(jni, (jstring)jni->CallObjectMethod(activityObject, getPackageCodePathId));
    if (!jni->ExceptionOccurred()) {
        const char* path = result.ToStr();
        if (path != NULL) {
            OVR::OVR_sprintf(packageCodePath, maxLen, "%s", path);
        }
    } else {
        jni->ExceptionClear();
        OVR_LOG("Cleared JNI exception");
    }

    return packageCodePath;
}

// If the specified package is found, returns true and the full path for the specified package is
// copied into packagePath. If the package was not found, false is returned and packagePath will be
// an empty string.
inline bool ovr_GetInstalledPackagePath(
    JNIEnv* jni,
    jobject activityObject,
    char const* packageName,
    char* packagePath,
    int const packagePathSize) {
    OVR_ASSERT(
        packagePath != NULL && packagePathSize > 1 && packageName != NULL &&
        packageName[0] != '\0');

    packagePath[0] = '\0';

    /// Java - for reference
#if 0
	import android.content.pm.PackageManager;
	import android.content.pm.ApplicationInfo;
	import android.content.Context;

	public static String getInstalledPackagePath( Context context, String packageName )
	{
		final String notInstalled = "";
		Log.d( TAG, "Searching installed packages for '" + packageName + "'" );
		try {
			ApplicationInfo info = context.getPackageManager().getApplicationInfo( packageName, 0 );
			return info.sourceDir != null ? info.sourceDir : notInstalled;
		} catch ( NameNotFoundException e ) {
			Log.w( TAG, "Package '" + packageName + "' not installed", e );
		}
		return notInstalled;
	}
#endif

    /// JNI equivalent
    jclass activityClass = jni->GetObjectClass(activityObject);
    if (activityClass == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath activityClass == NULL");
        return false;
    }

    jmethodID getPackageManagerMethod = jni->GetMethodID(
        activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    if (getPackageManagerMethod == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath getPackageManagerMethod == NULL");
        return false;
    }

    jobject packageManager = jni->CallObjectMethod(activityObject, getPackageManagerMethod);
    if (packageManager == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath packageManager == NULL");
        return false;
    }

    jclass PackageManagerClass = jni->FindClass("android/content/pm/PackageManager");
    if (PackageManagerClass == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath PackageManagerClass == NULL");
        return false;
    }

    jmethodID getApplicationInfoMethod = jni->GetMethodID(
        PackageManagerClass,
        "getApplicationInfo",
        "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;");
    if (getApplicationInfoMethod == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath getApplicationInfoMethod == NULL");
        return false;
    }

    jobject packageNameObj = jni->NewStringUTF(packageName);
    if (packageNameObj == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath packageNameObj == NULL");
        return false;
    }

    jint jZero = 0;
    jobject applicationInfo =
        jni->CallObjectMethod(packageManager, getApplicationInfoMethod, packageNameObj, jZero);
    if (applicationInfo == NULL) {
        OVR_LOG(
            "ovr_GetInstalledPackagePath applicationInfo == NULL packageName='%s'", packageName);
        return false;
    }

    if (jni->ExceptionOccurred()) {
        OVR_LOG(
            "ovr_GetInstalledPackagePath getApplicationInfo threw an exception: package not found.");
        jni->ExceptionClear();
        return false;
    }

    jclass ApplicationInfoClass = jni->GetObjectClass(applicationInfo);
    if (ApplicationInfoClass == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath ApplicationInfoClass == NULL");
        return false;
    }

    jfieldID sourceDirField =
        jni->GetFieldID(ApplicationInfoClass, "sourceDir", "Ljava/lang/String;");
    if (sourceDirField == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath sourceDirField == NULL");
        return false;
    }

    jstring sourceDir = (jstring)jni->GetObjectField(applicationInfo, sourceDirField);
    if (sourceDir == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath sourceDir == NULL");
        return false;
    }

    const char* sourceDir_ch = jni->GetStringUTFChars(sourceDir, 0);
    if (sourceDir_ch == NULL) {
        OVR_LOG("ovr_GetInstalledPackagePath sourceDir_ch == NULL");
        return false;
    }

    /// If all goes well ... :)
    OVR::OVR_strcpy(packagePath, packagePathSize, sourceDir_ch);

#if defined(OVR_BUILD_DEBUG)
    OVR_LOG("ovr_GetInstalledPackagePath packagePath = '%s'", packagePath);
#endif
    jni->ReleaseStringUTFChars(sourceDir, sourceDir_ch);
    return true;
}

// Get the current package name, for instance "com.oculus.systemactivities".
inline const char* ovr_GetCurrentPackageName(
    JNIEnv* jni,
    jobject activityObject,
    char* packageName,
    int const maxLen) {
    packageName[0] = '\0';

    JavaClass curActivityClass(jni, jni->GetObjectClass(activityObject));
    jmethodID getPackageNameId =
        jni->GetMethodID(curActivityClass.GetJClass(), "getPackageName", "()Ljava/lang/String;");
    if (getPackageNameId != 0) {
        JavaUTFChars result(jni, (jstring)jni->CallObjectMethod(activityObject, getPackageNameId));
        if (!jni->ExceptionOccurred()) {
            const char* currentPackageName = result.ToStr();
            if (currentPackageName != NULL) {
                OVR::OVR_sprintf(packageName, maxLen, "%s", currentPackageName);
            }
        } else {
            jni->ExceptionClear();
            OVR_LOG("Cleared JNI exception");
        }
    }
    // OVR_LOG( "ovr_GetCurrentPackageName() = %s", packageName );
    return packageName;
}

// Get the current activity name, for instance "com.oculus.systemactivities.PlatformActivity".
inline const char* ovr_GetCurrentActivityName(
    JNIEnv* jni,
    jobject activityObject,
    char* activityName,
    int const maxLen) {
    activityName[0] = '\0';

    JavaClass curActivityClass(jni, jni->GetObjectClass(activityObject));
    jmethodID getClassMethodId =
        jni->GetMethodID(curActivityClass.GetJClass(), "getClass", "()Ljava/lang/Class;");
    if (getClassMethodId != 0) {
        JavaObject classObj(jni, jni->CallObjectMethod(activityObject, getClassMethodId));
        JavaClass activityClass(jni, jni->GetObjectClass(classObj.GetJObject()));

        jmethodID getNameMethodId =
            jni->GetMethodID(activityClass.GetJClass(), "getName", "()Ljava/lang/String;");
        if (getNameMethodId != 0) {
            JavaUTFChars utfCurrentClassName(
                jni, (jstring)jni->CallObjectMethod(classObj.GetJObject(), getNameMethodId));
            const char* currentClassName = utfCurrentClassName.ToStr();
            if (currentClassName != NULL) {
                OVR::OVR_sprintf(activityName, maxLen, "%s", currentClassName);
            }
        }
    }
    return activityName;
}

// Get the current package's signing identies
inline jobject
ovr_GetPackageSignatures(JNIEnv* jni, jobject activityObject, const char* packageName) {
    OVR_LOG("ovr_GetCurrentPackageSignature ");
    JavaClass curActivityClass(jni, jni->GetObjectClass(activityObject));
    JavaClass packageManagerClass(jni, jni->FindClass("android/content/pm/PackageManager"));
    JavaClass packageInfoClass(jni, jni->FindClass("android/content/pm/PackageInfo"));

    jmethodID getPackageManagerMethodId = jni->GetMethodID(
        curActivityClass.GetJClass(), "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jmethodID getPackageInfoMethodId = jni->GetMethodID(
        packageManagerClass.GetJClass(),
        "getPackageInfo",
        "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jfieldID signaturesFieldId = jni->GetFieldID(
        packageInfoClass.GetJClass(), "signatures", "[Landroid/content/pm/Signature;");

    JavaString packageNameStringObject(jni, jni->NewStringUTF(packageName));

    JavaObject packageManager(
        jni, jni->CallObjectMethod(activityObject, getPackageManagerMethodId));

    JavaObject packageInfo(
        jni,
        jni->CallObjectMethod(
            packageManager.GetJObject(),
            getPackageInfoMethodId,
            packageNameStringObject.GetJObject(),
            64 // PackageManager.GET_SIGNATURES
            ));

    if (jni->ExceptionOccurred()) {
        OVR_WARN("Could not find package info for %s:", packageName);
        jni->ExceptionDescribe();
        jni->ExceptionClear();
        return NULL;
    }

    return jni->GetObjectField(packageInfo.GetJObject(), signaturesFieldId);
}

// For instance packageName = "com.oculus.systemactivities".
inline bool ovr_IsCurrentPackage(JNIEnv* jni, jobject activityObject, const char* packageName) {
    char currentPackageName[128];
    ovr_GetCurrentPackageName(jni, activityObject, currentPackageName, sizeof(currentPackageName));
    const bool isCurrentPackage = (strcasecmp(currentPackageName, packageName) == 0);
    return isCurrentPackage;
}

// For instance activityName = "com.oculus.systemactivities.PlatformActivity".
inline bool ovr_IsCurrentActivity(JNIEnv* jni, jobject activityObject, const char* activityName) {
    char currentClassName[128];
    ovr_GetCurrentActivityName(jni, activityObject, currentClassName, sizeof(currentClassName));
    const bool isCurrentActivity = (strcasecmp(currentClassName, activityName) == 0);
    return isCurrentActivity;
}

#else
inline jclass
ovr_GetLocalClassReference(JNIEnv* jni, jobject activityObject, const char* className) {
    OVR_UNUSED(jni);
    OVR_UNUSED(activityObject);
    OVR_UNUSED(className);
    return nullptr;
}
inline jclass
ovr_GetGlobalClassReference(JNIEnv* jni, jobject activityObject, const char* className) {
    OVR_UNUSED(jni);
    OVR_UNUSED(activityObject);
    OVR_UNUSED(className);
    return nullptr;
}
inline jmethodID
ovr_GetStaticMethodID(JNIEnv* jni, jclass jniclass, const char* name, const char* signature) {
    OVR_UNUSED(jni);
    OVR_UNUSED(jniclass);
    OVR_UNUSED(name);
    OVR_UNUSED(signature);
    return nullptr;
}
inline bool ovr_IsCurrentActivity(JNIEnv* jni, jobject activityObject, const char* activityName) {
    OVR_UNUSED(jni);
    OVR_UNUSED(activityObject);
    OVR_UNUSED(activityName);
    return false;
}
inline bool ovr_IsCurrentPackage(JNIEnv* jni, jobject activityObject, const char* packageName) {
    OVR_UNUSED(jni);
    OVR_UNUSED(activityObject);
    OVR_UNUSED(packageName);
    return false;
}
#endif

#if defined(OVR_OS_ANDROID)
#include "JniUtils-inl.h"
#endif // defined(OVR_OS_ANDROID)

#endif // OVR_JniUtils_h
