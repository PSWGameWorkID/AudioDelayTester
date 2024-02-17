package id.psw.ndkaudiotest.backends

import android.content.Context
import android.content.res.AssetManager
import id.psw.ndkaudiotest.R

class AAudioBackend : AudioBackendBase() {
    companion object {
        init {
            System.loadLibrary("api_aaudio")
        }
    }
    private external fun cppInit(amgr: AssetManager)
    private external fun cppPlayBgm()
    private external fun cppPlaySfx()
    private external fun cppDestroy()
    private external fun cppGetTime() : Long

    private var _title = "AAudio"
    override val title: String get() = _title

    override fun init(ctx: Context) {
        _title = ctx.getString(R.string.backend_aaudio)
        cppInit(ctx.assets)
    }

    override fun getCurrentTime(): Long {
        return cppGetTime()
    }

    override fun playBgm() {
        cppPlayBgm()
    }

    override fun playSfx() {
        cppPlaySfx()
    }

    override fun destroy() {
        cppDestroy()
    }
}