// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.metallic.chiaki.settings

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.metallic.chiaki.common.AppDatabase
import com.metallic.chiaki.common.LogFile
import com.metallic.chiaki.common.LogManager
import com.metallic.chiaki.common.RegisteredHost
import com.metallic.chiaki.common.ext.toLiveData
import io.reactivex.disposables.CompositeDisposable
import io.reactivex.rxkotlin.addTo
import io.reactivex.schedulers.Schedulers

class SettingsLogsViewModel(val logManager: LogManager): ViewModel()
{
	private val _sessionLogs = MutableLiveData<List<LogFile>>(logManager.files)
	val sessionLogs: LiveData<List<LogFile>> get() = _sessionLogs

	private fun updateLogs()
	{
		_sessionLogs.value = logManager.files
	}

	fun deleteLog(file: LogFile)
	{
		file.file.delete()
		updateLogs()
	}
}