package id.psw.ndkaudiotest.backends

import android.content.Context
import android.content.res.AssetFileDescriptor
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.media.SoundPool
import android.os.Build
import id.psw.ndkaudiotest.R
import java.io.File

/**
 * Backend for Android Runtime MediaPlayer and SoundPool combo
 */
class MediaPlayerBackend : AudioBackendBase() {
    // 2 media players to make almost gapless loop
    private lateinit var mediaPlayer1 : MediaPlayer
    private lateinit var mediaPlayer2 : MediaPlayer
    private lateinit var soundPool: SoundPool
    private var fxId : Int = 0
    private lateinit var fxFd : AssetFileDescriptor
    private lateinit var bgmFd : AssetFileDescriptor

    private var isMediaPlayer1Active = false

    private var _title = "MediaPlayer/SoundPool Combo"
    override val title: String get() = _title

    override fun init(ctx: Context) {
        _title = ctx.getString(R.string.backend_mediaplayer)
        val atb = AudioAttributes.Builder()
            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
            .setUsage(AudioAttributes.USAGE_GAME)


        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            atb.setAllowedCapturePolicy(AudioAttributes.ALLOW_CAPTURE_BY_ALL)
        }

        val aattr = atb.build()
        mediaPlayer1 = MediaPlayer()
        mediaPlayer2 = MediaPlayer()
        soundPool =
            SoundPool.Builder()
                .setAudioAttributes(aattr)
                .setMaxStreams(100)
                .build()

        fxFd = ctx.assets.openFd("sfx.wav")
        bgmFd = ctx.assets.openFd("bgm.wav")
        val iis = ctx.assets.open("sfx.wav")
        val sfxWav = File(ctx.dataDir, "sfx.wav")
        val sfxWavOut = sfxWav.outputStream()
        iis.copyTo(sfxWavOut)
        iis.close()
        sfxWavOut.flush()
        sfxWavOut.close()

        fxId = soundPool.load(sfxWav.absolutePath, 1)

        mediaPlayer1.setDataSource(bgmFd)
        mediaPlayer2.setDataSource(bgmFd)
        mediaPlayer1.prepare()
        mediaPlayer2.prepare()
        mediaPlayer1.setOnCompletionListener {
            mediaPlayer2.start()
            isMediaPlayer1Active = false
            mediaPlayer1.seekTo(0)
        }
        mediaPlayer2.setOnCompletionListener {
            mediaPlayer1.start()
            isMediaPlayer1Active = true
            mediaPlayer2.seekTo(0)
        }
        isMediaPlayer1Active = true
    }

    override fun playBgm() {
        mediaPlayer1.start()
    }

    override fun getCurrentTime(): Long {
        return (if(isMediaPlayer1Active)
            mediaPlayer1.currentPosition
        else
            mediaPlayer2.currentPosition).toLong()
    }

    override fun playSfx() {
        soundPool.play(fxId, 1.0f, 1.0f, 1, 0, 1.0f)
    }

    override fun destroy() {
        mediaPlayer1.stop()
        mediaPlayer2.stop()
        soundPool.stop(fxId)
        mediaPlayer1.release()
        mediaPlayer2.release()
        soundPool.release()
        fxFd.close()
        bgmFd.close()
    }
}