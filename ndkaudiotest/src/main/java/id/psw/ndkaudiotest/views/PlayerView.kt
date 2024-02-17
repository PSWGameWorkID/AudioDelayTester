package id.psw.ndkaudiotest.views

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.util.AttributeSet
import android.view.View
import id.psw.ndkaudiotest.PlayerActivity
import id.psw.ndkaudiotest.R

class PlayerView(context: Context, attrs: AttributeSet) : View(context, attrs) {

    private var thickLineColor = Color.BLACK
    private var thinLineColor = Color.BLACK
    private var dotLineColor = Color.BLACK
    private var zeroLineColor = Color.BLACK
    private var thickLineWidth = 8f
    private var thinLineWidth = 2f
    private var dotLineWidth = 4f
    private var dotLineSeparate = 8f
    private var zeroLineWidth = 1f

    private var player : PlayerActivity? = null

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
        .apply {
            style = Paint.Style.STROKE
        }

    fun setPlayerActivity(ctx:PlayerActivity){
        player = ctx
    }

    init {
        val styles = context.obtainStyledAttributes(attrs, R.styleable.PlayerView)
        thickLineColor = styles.getColor(R.styleable.PlayerView_thickLineColor, thickLineColor)
        thinLineColor = styles.getColor(R.styleable.PlayerView_thinLineColor, thinLineColor)
        dotLineColor = styles.getColor(R.styleable.PlayerView_dottedLineColor, dotLineColor)
        zeroLineColor = styles.getColor(R.styleable.PlayerView_zeroLineColor, zeroLineColor)
        thickLineWidth = styles.getDimension(R.styleable.PlayerView_thickLineWidth, thickLineWidth)
        thinLineWidth = styles.getDimension(R.styleable.PlayerView_thinLineWidth, thinLineWidth)
        dotLineWidth = styles.getDimension(R.styleable.PlayerView_dottedLineWidth, dotLineWidth)
        dotLineSeparate = styles.getDimension(R.styleable.PlayerView_dottedLineSeparation, dotLineSeparate)
        zeroLineWidth = styles.getDimension(R.styleable.PlayerView_zeroLineWidth, zeroLineWidth)
        styles.recycle()
    }

    private val p = Path()

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        player?.onUpdate()
        val nowMs = (player?.currentTime ?: 200) % 3000L
        val tSy = zeroLineWidth
        val bSy = height - zeroLineWidth


        player?.hitsAt?.forEach {
            val z00ms = it
            val musMs = (it + (player?.musicDelayMs ?: 0))
            val efxMs = (it + (player?.effectDelayMs ?: 30))

            val musX = (musMs / 3000.0f) * width
            paint.color = thickLineColor
            paint.strokeWidth = thickLineWidth
            canvas.drawLine(musX, tSy, musX, bSy, paint)

            val efxX = (efxMs / 3000.0f) * width
            paint.color = thinLineColor
            paint.strokeWidth = thinLineWidth
            canvas.drawLine(efxX, tSy, efxX, bSy, paint)

            val z00X = (z00ms / 3000.0f) * width

            paint.style = Paint.Style.FILL
            p.reset()

            p.moveTo(z00X, zeroLineWidth)
            p.lineTo(z00X - zeroLineWidth / 2.0f, 0.0f)
            p.lineTo(z00X + zeroLineWidth / 2.0f, 0.0f)
            p.close()

            p.moveTo(z00X, height - zeroLineWidth)
            p.lineTo(z00X - zeroLineWidth / 2.0f, height.toFloat())
            p.lineTo(z00X + zeroLineWidth / 2.0f, height.toFloat())
            p.close()

            paint.color = zeroLineColor
            paint.strokeWidth = zeroLineWidth
            canvas.drawPath(p, paint)
            paint.style = Paint.Style.STROKE
        }

        p.reset()
        val nowX = (nowMs / 3000.0f) * width
        var y = 0.0f
        while(y < height){
            p.moveTo(nowX, y)
            p.lineTo(nowX, y + (dotLineSeparate / 2))
            y += dotLineSeparate
        }

        paint.color = dotLineColor
        paint.strokeWidth = dotLineWidth
        canvas.drawPath(p, paint)

        postInvalidate()
    }
}