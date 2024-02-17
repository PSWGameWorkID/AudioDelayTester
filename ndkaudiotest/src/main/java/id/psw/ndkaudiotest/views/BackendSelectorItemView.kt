package id.psw.ndkaudiotest.views

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.TextView
import id.psw.ndkaudiotest.R

class BackendSelectorItemView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr) {
    init {
        init(context)
    }

    fun init(ctx:Context){
        LayoutInflater.from(ctx).inflate(R.layout.selector_item, this)
    }

    fun set(displayName:String, onClick:() -> Unit){
        findViewById<TextView>(R.id.backend_name).text = displayName
        setOnClickListener { onClick() }
    }
}