/* Copyright 2021 topjohnwu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://apache.org
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

struct AppSpecializeArgs {
    jint uid;
    jint gid;
    jintArray gids;
    jint runtime_flags;
    jobjectArray rlimits;
    jint mount_external;
    jstring se_info;
    jstring nice_name;
    jintArray fds_to_close;
    jintArray fds_to_ignore;
    jboolean is_child_zygote;
    jstring instruction_set;
    jstring app_data_dir;
    jboolean is_top_app;
    jobjectArray pkg_data_info_list;
    jobjectArray whitelisted_data_info_list;
    jboolean bind_mount_app_data_dirs;
    jboolean bind_mount_app_storage_dirs;

    // Tambahan untuk kompatibilitas versi game modern
    jstring process;
};

class Api;

class ModuleBase {
public:
    virtual ~ModuleBase() {}
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize() {}
    virtual void postServerSpecialize() {}
};

class Api {
public:
    virtual ~Api() {}
    virtual int getApiVersion() = 0;
    virtual int getFlags() = 0;
    virtual void setOption(int option, bool value) = 0;
    virtual int connectCompanion() = 0;
    virtual void exemptFd(int fd) = 0;
};

typedef ModuleBase* (*RegisterModuleType)(Api*, JNIEnv*);

} // namespace zygisk

// Makro sakti yang dicari oleh compiler kamu di Gambar 9 agar tidak error lagi
#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[visibility("default")]] zygisk::ModuleBase* zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    return new clazz(); \
}
