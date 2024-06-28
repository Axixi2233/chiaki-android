// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.metallic.chiaki.settings

import android.app.Activity
import android.content.Intent
import android.content.res.Resources
import android.net.Uri
import android.os.Bundle
import android.text.InputType
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import androidx.preference.*
import com.metallic.chiaki.R
import com.metallic.chiaki.common.Preferences
import com.metallic.chiaki.common.exportAndShareAllSettings
import com.metallic.chiaki.common.ext.viewModelFactory
import com.metallic.chiaki.common.getDatabase
import com.metallic.chiaki.common.importSettingsFromUri
import io.reactivex.disposables.CompositeDisposable
import io.reactivex.rxkotlin.addTo

class DataStore(val preferences: Preferences) : PreferenceDataStore() {
    override fun getBoolean(key: String?, defValue: Boolean) = when (key) {
        preferences.logVerboseKey -> preferences.logVerbose
        preferences.swapCrossMoonKey -> preferences.swapCrossMoon
        preferences.rumbleEnabledKey -> preferences.rumbleEnabled
        preferences.rumbleGamePadEnableKey -> preferences.rumbleGamePadEnabled
        preferences.motionEnabledKey -> preferences.motionEnabled
        preferences.touchpadOnlyEnabledKey -> preferences.touchpadOnlyEnabled
        preferences.onScreenControlsEnabledKey -> preferences.onScreenControlsEnabled
        preferences.buttonHapticEnabledKey -> preferences.buttonHapticEnabled
        preferences.screenPortraitEnabledKey -> preferences.screenPortraitEnabled
        preferences.screenCutoutModeEnabledKey -> preferences.screenCutoutModeEnabled
        else -> defValue
    }

    override fun putBoolean(key: String?, value: Boolean) {
        when (key) {
            preferences.logVerboseKey -> preferences.logVerbose = value
            preferences.swapCrossMoonKey -> preferences.swapCrossMoon = value
            preferences.rumbleEnabledKey -> preferences.rumbleEnabled = value
            preferences.rumbleGamePadEnableKey -> preferences.rumbleGamePadEnabled = value
            preferences.motionEnabledKey -> preferences.motionEnabled = value
            preferences.touchpadOnlyEnabledKey -> preferences.touchpadOnlyEnabled = value
            preferences.onScreenControlsEnabledKey -> preferences.onScreenControlsEnabled = value
            preferences.buttonHapticEnabledKey -> preferences.buttonHapticEnabled = value
            preferences.screenPortraitEnabledKey -> preferences.screenPortraitEnabled = value
            preferences.screenCutoutModeEnabledKey -> preferences.screenCutoutModeEnabled =value
        }
    }

    override fun getString(key: String, defValue: String?) = when (key) {
        preferences.resolutionKey -> preferences.resolution.value
        preferences.fpsKey -> preferences.fps.value
        preferences.bitrateKey -> preferences.bitrate?.toString() ?: ""
        preferences.codecKey -> preferences.codec.value
        preferences.touchPadKeyTitle -> preferences.touchPadKey
        else -> defValue
    }

    override fun putString(key: String, value: String?) {
        when (key) {
            preferences.resolutionKey -> {
                val resolution =
                    Preferences.Resolution.values().firstOrNull { it.value == value } ?: return
                preferences.resolution = resolution
            }

            preferences.fpsKey -> {
                val fps = Preferences.FPS.values().firstOrNull { it.value == value } ?: return
                preferences.fps = fps
            }

            preferences.bitrateKey -> preferences.bitrate = value?.toIntOrNull()
            preferences.codecKey -> {
                val codec = Preferences.Codec.values().firstOrNull { it.value == value } ?: return
                preferences.codec = codec
            }

            preferences.touchPadKeyTitle -> preferences.touchPadKey = value
        }
    }
}

class SettingsFragment : PreferenceFragmentCompat(), TitleFragment {
    companion object {
        private const val PICK_SETTINGS_JSON_REQUEST = 1
    }

    private var disposable = CompositeDisposable()
    private var exportDisposable = CompositeDisposable().also { it.addTo(disposable) }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        val context = context ?: return

        val viewModel = ViewModelProvider(
            this,
            viewModelFactory { SettingsViewModel(getDatabase(context), Preferences(context)) })
            .get(SettingsViewModel::class.java)

        val preferences = viewModel.preferences
        preferenceManager.preferenceDataStore = DataStore(preferences)
        setPreferencesFromResource(R.xml.preferences, rootKey)

        preferenceScreen.findPreference<ListPreference>(getString(R.string.preferences_resolution_key))
            ?.let {
                it.entryValues = Preferences.resolutionAll.map { res -> res.value }.toTypedArray()
                it.entries =
                    Preferences.resolutionAll.map { res -> getString(res.title) }.toTypedArray()
            }

        preferenceScreen.findPreference<ListPreference>(getString(R.string.preferences_fps_key))
            ?.let {
                it.entryValues = Preferences.fpsAll.map { fps -> fps.value }.toTypedArray()
                it.entries = Preferences.fpsAll.map { fps -> getString(fps.title) }.toTypedArray()
            }

        val bitratePreference =
            preferenceScreen.findPreference<EditTextPreference>(getString(R.string.preferences_bitrate_key))
        val bitrateSummaryProvider = Preference.SummaryProvider<EditTextPreference> {
            preferences.bitrate?.toString() ?: getString(
                R.string.preferences_bitrate_auto,
                preferences.bitrateAuto
            )
        }
        bitratePreference?.let {
            it.summaryProvider = bitrateSummaryProvider
            it.setOnBindEditTextListener { editText ->
                editText.hint =
                    getString(R.string.preferences_bitrate_auto, preferences.bitrateAuto)
                editText.inputType = InputType.TYPE_CLASS_NUMBER
                editText.setText(preferences.bitrate?.toString() ?: "")
            }
        }
        viewModel.bitrateAuto.observe(this, Observer {
            bitratePreference?.summaryProvider = bitrateSummaryProvider
        })

        preferenceScreen.findPreference<ListPreference>(getString(R.string.preferences_codec_key))
            ?.let {
                it.entryValues = Preferences.codecAll.map { codec -> codec.value }.toTypedArray()
                it.entries =
                    Preferences.codecAll.map { codec -> getString(codec.title) }.toTypedArray()
            }

        val registeredHostsPreference =
            preferenceScreen.findPreference<Preference>("registered_hosts")
        viewModel.registeredHostsCount.observe(this, Observer {
            registeredHostsPreference?.summary =
                getString(R.string.preferences_registered_hosts_summary, it)
        })

        preferenceScreen.findPreference<Preference>(getString(R.string.preferences_export_settings_key))
            ?.setOnPreferenceClickListener { exportSettings(); true }
        preferenceScreen.findPreference<Preference>(getString(R.string.preferences_import_settings_key))
            ?.setOnPreferenceClickListener { importSettings(); true }

        preferenceScreen.findPreference<Preference>(getString(R.string.preferences_about_axixi_key))
            ?.setOnPreferenceClickListener {
                try {
                    val i = Intent(Intent.ACTION_VIEW)
                    i.setData(Uri.parse("https://space.bilibili.com/16893379"))
                    requireContext().startActivity(i)
                } catch (e: Exception) {
                    e.printStackTrace()
                }
                true
            }
    }

    override fun onDestroy() {
        super.onDestroy()
        disposable.dispose()
    }

    override fun getTitle(resources: Resources): String =
        resources.getString(R.string.title_settings)

    private fun exportSettings() {
        val activity = activity ?: return
        exportDisposable.clear()
        exportAndShareAllSettings(activity).addTo(exportDisposable)
    }

    private fun importSettings() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "application/json"
        }
        startActivityForResult(intent, PICK_SETTINGS_JSON_REQUEST)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == PICK_SETTINGS_JSON_REQUEST && resultCode == Activity.RESULT_OK) {
            val activity = activity ?: return
            data?.data?.also {
                importSettingsFromUri(activity, it, disposable)
            }
        }
    }
}