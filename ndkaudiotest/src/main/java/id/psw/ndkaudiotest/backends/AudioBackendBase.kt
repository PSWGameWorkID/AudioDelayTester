package id.psw.ndkaudiotest.backends

import android.content.Context

abstract class AudioBackendBase {
    abstract fun init(ctx: Context)
    abstract fun playBgm()
    abstract fun getCurrentTime() : Long
    abstract fun playSfx()
    abstract fun destroy()
    abstract val title : String
}