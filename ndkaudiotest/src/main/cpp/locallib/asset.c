#include <dtypes.h>
#include <locallib.h>
#include <malloc.h>

u32 readAsset(AAssetManager* mgr, u8** retval, u64* length, const char* name){
    AAsset* a = AAssetManager_open(mgr, name, AASSET_MODE_BUFFER);
    if(a == NULL) return 0;

    *length = AAsset_getLength(a);
    *retval = malloc(*length);
    AAsset_read(a, *retval, *length);
    AAsset_close(a);

    return 1;
}

void freeAsset(u8** data, u64* length){
    *length = 0;
    if(*data != NULL){
        free(*data);
    }
    *data = NULL;
}
