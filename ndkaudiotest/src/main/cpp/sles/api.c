#include <dtypes.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <locallib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#include <pthread.h>

ASSET_BUFFER(sfx_pcm)
ASSET_BUFFER(bgm_pcm)

#define TheResultOf ESL.LastResult =
#define IsChecked checkOpenSlError(__FILE__, __LINE__);
#define TAG sles_tag_name

static const char* sles_tag_name = "SLESBackendJNI";

typedef struct {
    SLObjectItf PlayerObject;
    SLPlayItf Player;
    SLVolumeItf Volume;
    SLSeekItf  Seek;
    SLAndroidSimpleBufferQueueItf BufferQueue;
    pthread_mutex_t BufferMutex;
} player_data_t ;

static struct {
    SLObjectItf EngineObject;
    SLEngineItf Engine;

    SLObjectItf MixerObject;
    SLOutputMixItf Mixer;

    u32 PlayerBufSize;

    player_data_t Bgm;
    player_data_t Sfx;

    SLresult  LastResult;
} ESL = {
    .PlayerBufSize =  0,
    .Bgm.BufferMutex = PTHREAD_MUTEX_INITIALIZER,
    .Sfx.BufferMutex = PTHREAD_MUTEX_INITIALIZER,
};

static void checkOpenSlError(const char*, int);
void onBgmQueueCallback(const SLAndroidSimpleBufferQueueItf, void*);
void onSfxQueueCallback(const SLAndroidSimpleBufferQueueItf, void*);
void onBgmStateCallback(SLPlayItf caller, void *pContext, SLuint32 event);

static void initEngine(){

    TheResultOf slCreateEngine(&ESL.EngineObject, 0, NULL, 0, NULL, NULL); IsChecked
    TheResultOf (*ESL.EngineObject)->Realize(ESL.EngineObject, SL_BOOLEAN_FALSE); IsChecked
    TheResultOf (*ESL.EngineObject)->GetInterface(ESL.EngineObject, SL_IID_ENGINE, &ESL.Engine); IsChecked

    const SLInterfaceID  ids[0] = {};
    const SLboolean req[0] = {};

    TheResultOf (*ESL.Engine)->CreateOutputMix(ESL.Engine, &ESL.MixerObject, 0, ids, req); IsChecked
    TheResultOf (*ESL.MixerObject)->Realize(ESL.MixerObject, SL_BOOLEAN_FALSE); IsChecked
}

static void initPlayer(){
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
    SLDataFormat_PCM pcm = {
            .formatType = SL_DATAFORMAT_PCM,
            .numChannels = 1,
            .samplesPerSec = SL_SAMPLINGRATE_44_1,
            .bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16,
            .containerSize = SL_PCMSAMPLEFORMAT_FIXED_16,
            .channelMask = SL_SPEAKER_FRONT_CENTER,
            .endianness = SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource audioSrc = {&loc_bufq, &pcm};

    SLDataLocator_OutputMix mix = {
            .locatorType =SL_DATALOCATOR_OUTPUTMIX,
            .outputMix = ESL.MixerObject
    };

    SLDataSink audioSnk = {
            .pLocator = &mix,
            .pFormat = NULL
    };

    SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_SEEK};
    SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

    // Create Player
    TheResultOf (*ESL.Engine)->CreateAudioPlayer(ESL.Engine, &ESL.Bgm.PlayerObject, &audioSrc, &audioSnk, 2, ids, req); IsChecked
    TheResultOf (*ESL.Engine)->CreateAudioPlayer(ESL.Engine, &ESL.Sfx.PlayerObject, &audioSrc, &audioSnk, 2, ids, req); IsChecked

    // Realize
    TheResultOf (*ESL.Bgm.PlayerObject)->Realize(ESL.Bgm.PlayerObject, SL_BOOLEAN_FALSE); IsChecked
    TheResultOf (*ESL.Sfx.PlayerObject)->Realize(ESL.Sfx.PlayerObject, SL_BOOLEAN_FALSE); IsChecked

    // Get Player Interface
    TheResultOf (*ESL.Bgm.PlayerObject)->GetInterface(ESL.Bgm.PlayerObject, SL_IID_PLAY, &ESL.Bgm.Player); IsChecked
    TheResultOf (*ESL.Sfx.PlayerObject)->GetInterface(ESL.Sfx.PlayerObject, SL_IID_PLAY, &ESL.Sfx.Player); IsChecked

    (*ESL.Bgm.Player)->RegisterCallback(ESL.Bgm.Player, onBgmStateCallback, NULL);
    // All Events
    (*ESL.Bgm.Player)->SetCallbackEventsMask(ESL.Bgm.Player, SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADSTALLED);

    // Get Player Volume Interface
    TheResultOf (*ESL.Bgm.PlayerObject)->GetInterface(ESL.Bgm.PlayerObject, SL_IID_VOLUME, &ESL.Bgm.Volume); IsChecked
    TheResultOf (*ESL.Sfx.PlayerObject)->GetInterface(ESL.Sfx.PlayerObject, SL_IID_VOLUME, &ESL.Sfx.Volume); IsChecked

    // Get Player Buffer Queue Interface
    TheResultOf (*ESL.Bgm.PlayerObject)->GetInterface(ESL.Bgm.PlayerObject, SL_IID_BUFFERQUEUE, &ESL.Bgm.BufferQueue); IsChecked
    TheResultOf (*ESL.Sfx.PlayerObject)->GetInterface(ESL.Sfx.PlayerObject, SL_IID_BUFFERQUEUE, &ESL.Sfx.BufferQueue); IsChecked

    // Register Callback
    TheResultOf (*ESL.Bgm.BufferQueue)->RegisterCallback(ESL.Bgm.BufferQueue, onBgmQueueCallback, NULL); IsChecked
    TheResultOf (*ESL.Sfx.BufferQueue)->RegisterCallback(ESL.Sfx.BufferQueue, onSfxQueueCallback, NULL); IsChecked

    pthread_mutex_init(&ESL.Sfx.BufferMutex, NULL);
    pthread_mutex_init(&ESL.Bgm.BufferMutex, NULL);

    // Set Volume to Max
    SLmillibel sfxMaxVol = 0xFFFF;
    SLmillibel bgmMaxVol = 0xFFFF;
    TheResultOf (*ESL.Sfx.Volume)->GetMaxVolumeLevel(ESL.Sfx.Volume, &sfxMaxVol); IsChecked
    TheResultOf (*ESL.Bgm.Volume)->GetMaxVolumeLevel(ESL.Bgm.Volume, &bgmMaxVol); IsChecked
    TheResultOf (*ESL.Sfx.Volume)->SetVolumeLevel(ESL.Sfx.Volume, sfxMaxVol); IsChecked
    TheResultOf (*ESL.Bgm.Volume)->SetVolumeLevel(ESL.Bgm.Volume, bgmMaxVol); IsChecked

    (*ESL.Bgm.BufferQueue)->Enqueue(ESL.Bgm.BufferQueue, _data_bgm_pcm, _data_bgm_pcm_len);
    (*ESL.Sfx.BufferQueue)->Enqueue(ESL.Sfx.BufferQueue, _data_sfx_pcm, _data_sfx_pcm_len);

    // TheResultOf (*ESL.Bgm.Seek)->SetLoop(ESL.Bgm.Seek, SL_BOOLEAN_TRUE, 0L, SL_TIME_UNKNOWN); IsChecked
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_SLESBackend_cppInit(JNIEnv *env, jobject thiz, jobject amgr) {
    AAssetManager* mgr = AAssetManager_fromJava(env, amgr);
    readAsset(mgr, &_data_sfx_pcm, &_data_sfx_pcm_len, "sfx.pcm");
    readAsset(mgr, &_data_bgm_pcm, &_data_bgm_pcm_len, "bgm.pcm");
    initEngine();
    initPlayer();
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_SLESBackend_cppPlayBgm(JNIEnv *env, jobject thiz) {
    (*ESL.Bgm.Player)->SetPlayState(ESL.Bgm.Player, SL_PLAYSTATE_PLAYING);
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_SLESBackend_cppPlaySfx(JNIEnv *env, jobject thiz) {
    (*ESL.Sfx.Player)->SetPlayState(ESL.Sfx.Player, SL_PLAYSTATE_PLAYING);
}

JNIEXPORT void JNICALL
Java_id_psw_ndkaudiotest_backends_SLESBackend_cppDestroy(JNIEnv *env, jobject thiz) {
    TheResultOf (*ESL.Sfx.Player)->SetPlayState(ESL.Sfx.Player, SL_PLAYSTATE_STOPPED); IsChecked
    TheResultOf (*ESL.Bgm.Player)->SetPlayState(ESL.Bgm.Player, SL_PLAYSTATE_STOPPED); IsChecked
    (*ESL.Sfx.PlayerObject)->Destroy(ESL.Sfx.PlayerObject); IsChecked
    (*ESL.Bgm.PlayerObject)->Destroy(ESL.Bgm.PlayerObject); IsChecked
    (*ESL.MixerObject)->Destroy(ESL.MixerObject); IsChecked
    (*ESL.EngineObject)->Destroy(ESL.EngineObject); IsChecked

    freeAsset(&_data_bgm_pcm , &_data_bgm_pcm_len);
    freeAsset(&_data_sfx_pcm, &_data_sfx_pcm_len);
    pthread_mutex_destroy(&ESL.Bgm.BufferMutex);
    pthread_mutex_destroy(&ESL.Sfx.BufferMutex);
}

static void checkOpenSlError(const char* file, int line){
    if(ESL.LastResult == SL_RESULT_SUCCESS) return;
#define CaseOf(x)  case x: resultName = #x; break;

    const char* resultName = "UNKNOWN_ERROR";
    switch(ESL.LastResult){
        CaseOf(SL_RESULT_SUCCESS)
        CaseOf(SL_RESULT_PRECONDITIONS_VIOLATED)
        CaseOf(SL_RESULT_PARAMETER_INVALID)
        CaseOf(SL_RESULT_MEMORY_FAILURE)
        CaseOf(SL_RESULT_RESOURCE_ERROR)
        CaseOf(SL_RESULT_RESOURCE_LOST)
        CaseOf(SL_RESULT_IO_ERROR)
        CaseOf(SL_RESULT_BUFFER_INSUFFICIENT)
        CaseOf(SL_RESULT_CONTENT_CORRUPTED)
        CaseOf(SL_RESULT_CONTENT_UNSUPPORTED)
        CaseOf(SL_RESULT_CONTENT_NOT_FOUND)
        CaseOf(SL_RESULT_PERMISSION_DENIED)
        CaseOf(SL_RESULT_FEATURE_UNSUPPORTED)
        CaseOf(SL_RESULT_INTERNAL_ERROR)
        CaseOf(SL_RESULT_UNKNOWN_ERROR)
        CaseOf(SL_RESULT_OPERATION_ABORTED)
    }

    __android_log_print(ANDROID_LOG_ERROR, TAG, "[%x08] %s at %s:%d", ESL.LastResult, resultName, file, line);
#undef CaseOf
}

void onBgmQueueCallback(const SLAndroidSimpleBufferQueueItf queue, void* ctx)
{
    TheResultOf (*queue)->Clear(queue); IsChecked
    TheResultOf (*queue)->Enqueue(queue, _data_bgm_pcm, _data_bgm_pcm_len); IsChecked
}

void onSfxQueueCallback(const SLAndroidSimpleBufferQueueItf queue, void* ctx)
{
    TheResultOf (*queue)->Enqueue(queue, _data_sfx_pcm, _data_sfx_pcm_len); IsChecked
}

void onBgmStateCallback(SLPlayItf player, void *pContext, SLuint32 event){

    {
#define CaseOf(x)  case x: resultName = #x; break;

        const char* resultName = "UNKNOWN_ERROR";
        switch(ESL.LastResult){
            CaseOf(SL_PLAYEVENT_HEADATEND)
            CaseOf(SL_PLAYEVENT_HEADATMARKER)
            CaseOf(SL_PLAYEVENT_HEADATNEWPOS)
            CaseOf(SL_PLAYEVENT_HEADMOVING)
            CaseOf(SL_PLAYEVENT_HEADSTALLED)
        }

        __android_log_print(ANDROID_LOG_ERROR, TAG, "Got Event [%x08] %s ",event, resultName);
#undef CaseOf
    }

    if(event == SL_PLAYEVENT_HEADATEND)
    {
        TheResultOf (*ESL.Bgm.BufferQueue)->Clear(ESL.Bgm.BufferQueue); IsChecked
        TheResultOf (*ESL.Bgm.BufferQueue)->Enqueue(ESL.Bgm.BufferQueue, _data_bgm_pcm, _data_bgm_pcm_len); IsChecked
        TheResultOf (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING); IsChecked
    }
}

JNIEXPORT jlong JNICALL
Java_id_psw_ndkaudiotest_backends_SLESBackend_cppGetTime(JNIEnv *env, jobject thiz) {
    SLmillisecond sms = 0;
    TheResultOf (*ESL.Bgm.Player)->GetPosition(ESL.Bgm.Player, &sms); IsChecked;
    return sms;
}
