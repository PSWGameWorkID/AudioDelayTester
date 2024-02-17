package id.psw.ndkaudiotest.backends

import android.content.Context
import android.content.res.AssetManager
import id.psw.ndkaudiotest.R

class SLESBackend : AudioBackendBase() {

    companion object {
        init {
            System.loadLibrary("api_sles")
        }
    }

    private external fun cppInit(amgr: AssetManager)
    private external fun cppPlayBgm()
    private external fun cppPlaySfx()
    private external fun cppDestroy()
    private external fun cppGetTime() : Long

    private var _title = "OpenSL ES"
    override val title: String get() = _title

    override fun init(ctx: Context) {
        _title = ctx.getString(R.string.backend_aaudio)
        cppInit(ctx.assets)
    }

    override fun playBgm() {
        cppPlayBgm()
    }

    override fun getCurrentTime(): Long {
        return cppGetTime()
    }

    override fun playSfx() {
        cppPlaySfx()
    }

    override fun destroy() {
        cppDestroy()
    }
}