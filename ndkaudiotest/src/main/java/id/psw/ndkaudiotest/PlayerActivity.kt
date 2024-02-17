package id.psw.ndkaudiotest

import android.app.Activity
import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import id.psw.ndkaudiotest.backends.AAudioBackend
import id.psw.ndkaudiotest.backends.AudioBackendBase
import id.psw.ndkaudiotest.backends.MediaPlayerBackend
import id.psw.ndkaudiotest.backends.SLESBackend
import id.psw.ndkaudiotest.views.DelayAdjustView
import id.psw.ndkaudiotest.views.PlayerView
import java.util.Timer
import kotlin.math.roundToInt
import kotlin.math.roundToLong

class PlayerActivity : Activity() {

    internal var currentTime = 0L
    internal var musicDelayMs = 0L
    internal var effectDelayMs = 0L
    internal val hitsAt = arrayOf(660, 2160)
    private lateinit var scheduler : Timer
    private lateinit var backend : AudioBackendBase
    private lateinit var finalDelayText : TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.player_activity)
        val pv = findViewById<PlayerView>(R.id.player_view)

        bindButtons()

        val text = intent.getStringExtra(Constants.EXQ_BACKEND_TYPE)
        backend = when(text){
            Constants.BK_AAUDIO -> AAudioBackend()
            Constants.BK_MEDIAPLAYER -> MediaPlayerBackend()
            Constants.BK_OPENSL_ES -> SLESBackend()
            else -> {
                Toast.makeText(applicationContext,
                    getString(R.string.err_unknown_backend), Toast.LENGTH_LONG).show()
                finish()
                return
            }
        }
        backend.init(this)
        title = getString(R.string.app_name)
        actionBar?.subtitle = backend.title
        pv.setPlayerActivity(this)
    }

    private fun bindButtons() {
        val fxd = findViewById<DelayAdjustView>(R.id.delay_adjust_effect)
        val msd = findViewById<DelayAdjustView>(R.id.delay_adjust_music)
        finalDelayText = findViewById(R.id.delay_adjust_final)
        fxd.apply(getString(R.string.hit_effect_delay), effectDelayMs){
            effectDelayMs = it
            updateResultDelay()
        }
        msd.apply(getString(R.string.music_hit_delay), musicDelayMs){
            musicDelayMs = it
            updateResultDelay()
        }
        updateResultDelay()
    }

    private fun updateResultDelay() {
        val finalDelay = ((musicDelayMs + (effectDelayMs - musicDelayMs)) / 2.0f).roundToLong()
        finalDelayText.text = getString(R.string.ms).format(finalDelay)
    }

    override fun onStart() {
        super.onStart()
        backend.playBgm()
    }

    override fun onDestroy() {
        super.onDestroy()
        backend.destroy()
    }

    internal fun onUpdate(){
        val nCTime = currentTime
        currentTime = backend.getCurrentTime()

        val lastTimeSub = nCTime % 3000
        val currTimeSub = currentTime % 3000

        hitsAt.forEach {
            val hit = it + musicDelayMs
            if(lastTimeSub < hit && hit < currTimeSub ){
                backend.playSfx()
            }
        }

    }
}