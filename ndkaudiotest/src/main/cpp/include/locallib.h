#pragma once
#include "dtypes.h"
#include <android/asset_manager_jni.h>

#define ASSET_BUFFER(name) \
    static u8* _data_##name ;\
    static u64 _data_##name##_len;\

u32 readAsset(AAssetManager* mgr, u8** retval, u64* length, const char* name);
void freeAsset(u8** data, u64* length);