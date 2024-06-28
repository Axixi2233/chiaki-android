[Oboe Docs Home](README.md)

# Tech Note: Disconnected Streams and Plugin Issues

When Oboe is using **OpenSL ES**, and a headset is plugged in or out, then OpenSL ES will automatically switch between devices.
This is convenient but can cause problems because the new device may have different burst sizes and different latency.

When Oboe is using **AAudio**, and a headset is plugged in or out, then
the stream is no longer available and becomes "disconnected".
The app should then be notified in one of two ways. 

1) If the app is using an error callback then the AudioStreamErrorCallback methods will be called.
It will launch a thread, which will call onErrorBeforeClose().
Then it stops and closes the stream.
Then onErrorAfterClose() will be called.
An app may choose to reopen a stream in the onErrorAfterClose() method.

2) If an app is using read()/write() calls then they will return an error when a disconnect occurs.
The app should then stop() and close() the stream.
An app may then choose to reopen a stream.

## Bug in P and Q

On some versions of Android P (9), and some early versions of Q (10), the disconnect message does not reach AAudio and the app will not
know that the device has changed. There is a "TEST DISCONNECT" option in
[OboeTester](https://github.com/google/oboe/tree/main/apps/OboeTester/docs)
that can be used to diagnose this problem.

## Workaround for not Disconnecting Properly

As a workaround you can listen for a Java [Intent.ACTION_HEADSET_PLUG](https://developer.android.com/reference/android/content/Intent#ACTION_HEADSET_PLUG),
which is fired when a head set is plugged in or out. If your min SDK is LOLLIPOP or later then you can use [AudioManager.ACTION_HEADSET_PLUG](https://developer.android.com/reference/android/media/AudioManager#ACTION_HEADSET_PLUG) instead.

    // Receive a broadcast Intent when a headset is plugged in or unplugged.
    public class PluginBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Close the stream if it was not disconnected.
        }
    }
    
    private BroadcastReceiver mPluginReceiver = new PluginBroadcastReceiver();
    
You can register for the Intent when your app resumes and unregister when it pauses.
    
    @Override
    public void onResume() {
        super.onResume();
        IntentFilter filter = new IntentFilter(Intent.ACTION_HEADSET_PLUG);
        this.registerReceiver(mPluginReceiver, filter);
    }

    @Override
    public void onPause() {
        this.unregisterReceiver(mPluginReceiver);
        super.onPause();
    }

## Internal Notes

* Oboe Issues
  * [#381](https://github.com/google/oboe/issues/381) Connecting headphones does not trigger any event. S9
  * [#893](https://github.com/google/oboe/issues/893) onErrorBeforeClose and onErrorAfterClose not called, S10
  * [#908](https://github.com/google/oboe/issues/908) Huawei MAR-LX3A
  * [#1350](https://github.com/google/oboe/issues/1350) SM-G977B fails "Test Disconnects"
* This issue is tracked internally as b/111711159.
* A fix in AOSP is [here](https://android-review.googlesource.com/c/platform/frameworks/av/+/836184)
* A fix was also merged into pi-dev on July 30, 2018.

### Results from Test Disconnect in OboeTester

| Device | Build | Result |
|:--|:--|:--|
| Pixel 1 | QQ1A.190919.002 | ALL PASS |
| Samsung S10e | PPR1.180610.011 | MMAP Output plugIN failed |
| Huawei MAR-LX3A | 10.0.0.216 | MMAP In/Out fails, Legacy In fails. |
