package id.psw.ndkaudiotest.views

import android.content.Context
import android.text.Editable
import android.text.TextWatcher
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import id.psw.ndkaudiotest.R

class DelayAdjustView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr), TextWatcher {
    init {
        init(context)
    }

    private lateinit var btnAdd10 : Button
    private lateinit var btnAdd100 : Button
    private lateinit var btnMin10 : Button
    private lateinit var btnMin100 : Button
    private lateinit var label : TextView
    private lateinit var numView : EditText

    fun init(ctx:Context){
        LayoutInflater.from(ctx).inflate(R.layout.delay_adjust_view, this)
        numView = findViewById(R.id.delay_value)
        label = findViewById(R.id.delay_label)
        btnAdd10 = findViewById(R.id.delay_add10)
        btnAdd100 = findViewById(R.id.delay_add100)
        btnMin10 = findViewById(R.id.delay_min10)
        btnMin100 = findViewById(R.id.delay_min100)

        btnAdd10.setOnClickListener { changeValue(+10) }
        btnAdd100.setOnClickListener { changeValue(+100) }
        btnMin10.setOnClickListener { changeValue(-10) }
        btnMin100.setOnClickListener { changeValue(-100) }
    }

    // Just a stupid thing
    private fun changeValue(delta: Long) {
        var cbv = numView.text.toString().toLongOrNull(10) ?: return
        cbv += delta
        numView.setText(cbv.toString())
        onValueChange.invoke(cbv)
    }

    private var onValueChange : (Long) -> Unit = ::defaultCallback

    private fun defaultCallback(i:Long){
        // do nothing
    }

    fun apply(title:String, initialValue:Long, onValueChange : (Long) -> Unit){
        label.text = title
        numView.setText(initialValue.toString(10))
        numView.addTextChangedListener(this)
        this.onValueChange = onValueChange
    }

    override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {
    }

    override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
    }

    override fun afterTextChanged(s: Editable?) {
        if(s == null) return
        val cbv = s.toString().toLongOrNull(10) ?: return
        onValueChange.invoke(cbv)
    }
}