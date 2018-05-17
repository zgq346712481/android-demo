//
// Created by wujingchao on 17/12/11.
//

#include "util/Log.h"
#include "SelectSocketServer.h"
#include "EPollSocketServer.h"

#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>

#include <string>
#include <sstream>
#include <memory>
#include <dlfcn.h>

#define LOG_TAG "native-lib"
#define NELEM(x) (sizeof(x)/sizeof(x[0]))

int registerNatives(JNIEnv *pEnv);

//extern "C" 加上extern C是防止C++编译器对方法名进行重整(mangled)
extern "C"
jstring Java_com_wujingchao_android_demo_os_PipeDemo_readFromChildPipe(JNIEnv *env, jobject instance) {
    int pipe_fd[2] = {0};//first for read, second for write
    if (pipe(pipe_fd) == -1) {
        const char* msg = "create pipe error";
        LOGE(msg);
        return env->NewStringUTF(msg);
    }
    pid_t childPid = fork();
    if (childPid == -1) {
        const char* msg = "fork error";
        LOGE(msg);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    if (childPid == 0) {//child process
        close(pipe_fd[0]);//close  read
        write(pipe_fd[1], "PIPE", 4);
        close(pipe_fd[1]);
        exit(0);
    } else {
        std::shared_ptr<std::string> ptr =  std::make_shared<std::string>("shared_ptr str");
        LOGD(ptr->c_str());
        //just test c++...
        std::ostringstream ostream;
        ostream << childPid;
        std::string tmp("child process : " + ostream.str());
        LOGD(tmp.c_str());

        char buff[5] = {0};
        close(pipe_fd[1]);
        while (read(pipe_fd[0], buff, sizeof(buff) - 1) > 0) {
            LOGE(buff);
        }
        close(pipe_fd[0]);
        waitpid(childPid, NULL, 0);//reap child process
        return env->NewStringUTF(buff);

    }
}

extern "C"
void Java_com_wujingchao_android_demo_os_PipeDemo_allocManyLocalRef(JNIEnv *env, jobject instance) {
    for (int i = 0; i < 1024; i++) {
        jclass jclass1 = env->FindClass("android/os/Looper");//local reference的数量是有限制的
    }
}

extern "C"
void Java_com_wujingchao_android_demo_os_PipeDemo_testStr(JNIEnv *env, jobject instance, jstring jstr) {
    jboolean isCopy;
    jsize strLen = env->GetStringLength(jstr);
    LOGD("string len = %d ", strLen);
    const jchar* cjchar1 = env->GetStringCritical(jstr, &isCopy);//unicode
    env->ReleaseStringCritical(jstr, cjchar1);

    const char* cchar1 = env->GetStringUTFChars(jstr, &isCopy);//modified utf8
    LOGD(cchar1);
    env->ReleaseStringUTFChars(jstr, cchar1);
}

//不需要extern "C",即使被方法名重整了也没有关系
jboolean startSelectServerNative(JNIEnv *env, jobject instance, jstring ip, jint port) {
    if (ip == nullptr) {
        SelectSocketServer selectSocketServer(nullptr, port);
        selectSocketServer.startServer();
    } else {
        const char* cip = env->GetStringUTFChars(ip, nullptr);
        SelectSocketServer selectSocketServer(cip, port);
        selectSocketServer.startServer();
        env->ReleaseStringUTFChars(ip, cip);
    }
    return JNI_TRUE;
}

jboolean startEPollServerNative(JNIEnv *env, jobject instance, jstring ip, jint port) {
    if (ip == nullptr) {

    } else {

    }
    return JNI_TRUE;
}


/**
 *  const char* name;
    const char* signature;
    void*       fnPtr;
 */

static JNINativeMethod gMethod[] = {
        {"startSelectServerNative", "(Ljava/lang/String;I)Z", (void*)startSelectServerNative},
        {"startEPollServerNative", "(Ljava/lang/String;I)Z", (void*)startEPollServerNative}
};


jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad");

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }


    bail:
    return JNI_VERSION_1_4;
}

int registerNatives(JNIEnv *env) {
    jclass IOMultiplexingDemoActivityClazz = env->FindClass("com/wujingchao/android/demo/os/IOMultiplexingDemoActivity");
    env->RegisterNatives(IOMultiplexingDemoActivityClazz, gMethod, NELEM(gMethod));
    return 0;
}





