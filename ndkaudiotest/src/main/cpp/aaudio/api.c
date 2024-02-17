#include <dtypes.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <locallib.h>
#include <aaudio/AAudio.h>
#include <android/log.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

ASSET_BUFFER(sfx_pcm)
ASSET_BUFFER(bgm_pcm)

#define TheResultOf AState.LastResult =
#define IsChecked checkLastError(__FILE__, __LINE__);
#define TAG sles_tag_name

static const char* sles_tag_name = "AAudioBackendJNI";

aaudio_data_callback_result_t bgmDataCallback(AAudioStream*,void*,void*,i32);
aaudio_data_callback_result_t sfxDataCallback(AAudioStream*,void*,void*,i32);
void checkLastError(const char*, u32);

const size_t frameSize = 1 * sizeof(i16);

static struct {
    AAudioStreamBuilder* Builder;
    AAudioStream* SfxStream;
    AAudioStream* BgmStream;
    u64 SfxByteIndex;
    u64 BgmByteIndex;
    u8 PlayOnceFence;
    aaudio_result_t LastResult;
} AState;

static void createBuilder(){
    __android_log_print(ANDROID_LOG_INFO, TAG, "Creating Audio Builder ...");
    TheResultOf AAudio_createStreamBuilder(&AState.Builder); IsChecked
    AAudioStreamBuilder_setDeviceId(AState.Builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setDirection(AState.Builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setSharingMode(AState.Builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setSampleRate(AState.Builder, 44100);
    AAudioStreamBuilder_setChannelCount(AState.Builder, 1);
    AAudioStreamBuilder_setPerformanceMode(AState.Builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setFormat(AState.Builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setUsage(AState.Builder, AAUDIO_USAGE_GAME);

    __android_log_print(ANDROID_LOG_INFO, TAG, "Audio Builder Created");
}

static void createPlayers(){
    __android_log_print(ANDROID_LOG_INFO, TAG, "Creating Audio Stream ...");
    AState.BgmByteIndex = 0;
    AState.SfxByteIndex = 0;

    AAudioStreamBuilder_setDataCallback(AState.Builder, sfxDataCallback, NULL);
    TheResultOf AAudioStreamBuilder_openStream(AState.Builder, &AState.SfxStream); IsChecked
    AAudioStreamBuilder_setDataCallback(AState.Builder, bgmDataCallback, NULL);
    TheResultOf AAudioStreamBuilder_openStream(AState.Builder, &AState.BgmStream); IsChecked

    i32 sfxBurst = AAudioStream_getFramesPerBurst(AState.SfxStream);
    i32 bgmBurst = AAudioStream_getFramesPerBurst(AState.BgmStream);
    AAudioStream_setBufferSizeInFrames(AState.SfxStream, sfxBurst);
    AAudioStream_setBufferSizeInFrames(AState.BgmStream, bgmBurst);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Audio Stream Created");
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_AAudioBackend_cppInit(JNIEnv *env, jobject thiz, jobject amgr) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "Loading Assets ...");
    AAssetManager* mgr = AAssetManager_fromJava(env, amgr);
    readAsset(mgr, &_data_sfx_pcm, &_data_sfx_pcm_len, "sfx.pcm");
    readAsset(mgr, &_data_bgm_pcm, &_data_bgm_pcm_len, "bgm.pcm");
    __android_log_print(ANDROID_LOG_INFO, TAG, "Assets Loaded");

    createBuilder();
    createPlayers();
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_AAudioBackend_cppPlayBgm(JNIEnv *env, jobject thiz) {
    TheResultOf AAudioStream_requestStart(AState.BgmStream); IsChecked
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_AAudioBackend_cppPlaySfx(JNIEnv *env, jobject thiz) {
    aaudio_stream_state_t cState = AAudioStream_getState(AState.SfxStream);
    if(cState == AAUDIO_STREAM_STATE_STARTED)
    {
        AState.SfxByteIndex = 0;
    }else{
        TheResultOf AAudioStream_requestStart(AState.SfxStream); IsChecked
    }
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_AAudioBackend_cppDestroy(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "Destroying AAudio Backend Instance");
    TheResultOf AAudioStream_close(AState.SfxStream); IsChecked
    TheResultOf AAudioStream_close(AState.BgmStream); IsChecked
    TheResultOf AAudioStreamBuilder_delete(AState.Builder); IsChecked
    AState.Builder= NULL;
    AState.SfxStream = AState.BgmStream = NULL;
    __android_log_print(ANDROID_LOG_INFO, TAG, "AAudio Backend Instance Destroyed ...");
    __android_log_print(ANDROID_LOG_INFO, TAG, "Freeing Loaded Assets ...");
    freeAsset(&_data_bgm_pcm, &_data_bgm_pcm_len);
    freeAsset(&_data_sfx_pcm, &_data_sfx_pcm_len);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Loaded Assets Freed");
}
#define ddmin(a, b) (((a) <= (b)) ? (a) : (b))

aaudio_data_callback_result_t bgmDataCallback(AAudioStream* stream,void* user,void* audio,i32 frames){
    // Continue
    size_t fByteCount = frames * frameSize;

    i32 continueBytes = ddmin(_data_bgm_pcm_len - AState.BgmByteIndex, fByteCount);
    memcpy(audio, _data_bgm_pcm + AState.BgmByteIndex, continueBytes);

    if(continueBytes < fByteCount){
        i32 reBeginBytes = fByteCount - continueBytes;
        memcpy(audio + continueBytes, _data_bgm_pcm, reBeginBytes);
        AState.BgmByteIndex = reBeginBytes;
    }else{
        AState.BgmByteIndex += continueBytes;
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

aaudio_data_callback_result_t sfxDataCallback(AAudioStream* stream,void* user,void* audio,i32 frames){
    size_t fByteCount = frames * frameSize;
    i32 readBytes = ddmin(_data_sfx_pcm_len - AState.SfxByteIndex, fByteCount);
    memset(audio, 0, fByteCount);
    memcpy(audio, _data_sfx_pcm + AState.SfxByteIndex, readBytes);
    AState.SfxByteIndex += readBytes;
    u8 complete = readBytes < fByteCount;
    if(complete){
        AState.SfxByteIndex = 0;
    }

    // Ask to stop when device is ended
    return complete ? AAUDIO_CALLBACK_RESULT_STOP : AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void checkLastError(const char* file, u32 line){
    if(AState.LastResult == AAUDIO_OK) return;

    const char* resultName = AAudio_convertResultToText(AState.LastResult);

    __android_log_print(ANDROID_LOG_ERROR, TAG, "%08x::%s at %s:%d",AState.LastResult, resultName, file, line);
}

JNIEXPORT jlong JNICALL
Java_id_psw_ndkaudiotest_backends_AAudioBackend_cppGetTime(JNIEnv *env, jobject thiz) {
    return (jlong)(AState.BgmByteIndex / (float)_data_bgm_pcm_len * 3000);
}