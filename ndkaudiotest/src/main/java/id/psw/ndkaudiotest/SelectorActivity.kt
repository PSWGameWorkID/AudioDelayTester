package id.psw.ndkaudiotest

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.widget.LinearLayout
import android.widget.ScrollView
import id.psw.ndkaudiotest.views.BackendSelectorItemView

class SelectorActivity : Activity() {
    val items = mapOf(
        R.string.backend_opensl_es to Constants.BK_OPENSL_ES,
        R.string.backend_aaudio to Constants.BK_AAUDIO,
        R.string.backend_mediaplayer to Constants.BK_MEDIAPLAYER
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.selector_activity)
        title = getString(R.string.title_backend)

        val scrollItems = findViewById<LinearLayout>(R.id.items)
        items.forEach { (k, v) ->
            val view = BackendSelectorItemView(this)
            view.set(getString(k)){
                startPlayerActivity(v)
            }
            scrollItems.addView(view)
        }
    }

    private fun startPlayerActivity(id:String){
        val i = Intent(this, PlayerActivity::class.java)
        i.putExtra(Constants.EXQ_BACKEND_TYPE, id)
        startActivity(i)
    }
}