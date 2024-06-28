[Home](README.md)

# How to Use OboeTester

## Loopback Adapter Needed for Some Tests

Some of these tests require an [audio Loopback Adapter](https://source.android.com/devices/audio/latency/loopback) that plugs into a 3.5 mm jack.
If the phone does not have a 3.5 mm jack then you can combine that with a USB to 3.5mm adapter.

Loopback adapters can be purchased from [PassMark Software](https://www.passmark.com/products/audio-loopback-plug/). 

## Test Activities

Launch OboeTester and then select one of the test Activities.

### Test Output

Tap the green bar to expand the Settings display.

Test opening, starting, stopping and closing a stream.
Various stream parameters can be selected before opening.
While the stream is playing, the frame counts and stream state are displayed.
The latency is estimated based on the timestamp information.

Experiment with changing the buffer size to see if there are glitches when the size is high or low.

Experiment with changing the "Workload".
Watch the frame counters and the #XRuns.
The audio is expected to glitch when the workload is high because there is too much work
and the audio task misses its delivery deadlines.

The extra workload is generated by calculating a random number in a loop and then adding the result to the output at an inaudible level. This technique prevents the compiler optimizer from skipping the work. The workload fader is in arbitrary units that determines the number of loops.
You can see the affect of the changing workload in the "% cpu" report in the status area.
The extra workload will always cause glitching when you get close to 100% CPU load.
If the workload is causing glitching at a low % CPU load then there may be a problem with the callback timing.

Instructions for using TEST OUTPUT for 4 or more chanels is in
[Wiki/OboeTester_MultiChannelOutput](https://github.com/google/oboe/wiki/OboeTester_MultiChannelOutput).

### Test Input

Test input streams. Displays current volume as VU bars.

### Tap to Tone Latency

Measure touch screen latency plus audio output latency.
Open, Start, then tap on the screen with your fingertip.
The app will listen for the sound of your fingernail tapping the screen
and the resulting beep, and then measure the time between them.

If you use headphones then you can eliminate the latency caused by speaker protection.
If you use USB-MIDI input then you can eliminate the latency due to the touch screen, which is around 15-30 msec.
MIDI latency is generally under 1 msec.
This test works well for measuring the latency of Bluetooth headsets.

More instructions in the [Wiki/OboeTester_TapToTone](https://github.com/google/oboe/wiki/OboeTester_TapToTone).

### Record and Play

* Tap RECORD to record several seconds of audio. You should see the red VU meters move.
* Tap STOP then PLAY to play it back.
* Tap SHARE button to the recorded WAV file to GDrive, GMail or another app.
You can then examine the WAV file using a program like Audacity.

### Echo Input to Output

This test copies input to output and adds up to 3 seconds of delay.
This can be used to simulate high latency on a phone that supports low latency.
Try putting the phone to your ear with the added delay at 0 and try talking.
Then set it to about 700 msec and try talking on the phone. Notice how the echo can be very confusing.

The test will also display estimated "cold start latency" for full duplex streams.

### Round Trip Latency

This test works with either a [loopback adapter](https://source.android.com/devices/audio/latency/loopback) or through speakers.
Latency through the speakers will probably be higher.
It measures the input and output latency combined.

1. Set the Input or Output settings by tapping the green bar to expose the controls.
2. Set the volume somewhere above the middle. The test works at almost any volume. But the confidence will be higher at higher volumes.
3. Tap "MEASURE" to make a single measurement.
4. Or tap "AVERAGE" to run the test several times and report an average and Mean Absolute Deviation.

The test starts by setting up a stable full-duplex stream.
Then it outputs a random series of bits encoded using smoothed Manchester Encoding.
We record the Input and Output stream together for about a second.
Then we correlate the two streams by sliding the portion of the output stream that contains the random bits over the input stream at different time offsets.
The Manchaster Encoded signal provide a very sharp peak when the offset matches the combined input and output latency.

Source code for the analyzer in [LatencyAnalyzer.h](https://github.com/google/oboe/blob/main/apps/OboeTester/app/src/main/cpp/analyzer/LatencyAnalyzer.h).

### Glitch Test

This test works best with a loopback adapter. But it can also work in a quiet room.
It plays a sine wave and then tries to record and lock onto that sine wave.
If the actual input does not match the expected sine wave value then it is counted as a glitch.
The test will display the maximum time that it ran without seeing a glitch.

Look for the #XRuns display.
If #XRuns increments when a glitch occurs then the glitch is probably due to preemption of the audio task.
If #XRuns is not incrementing then the glitches may be due to AAudio MMAP tuning errors in the HAL.

### Auto Glitch Test

Measure glitches for various combinations of input and output settings.
Change the test duration to a high value and let it run for a while.
If you get glitches in one configuration then you can investigate using the previous manual Glitch Test.
The Share button will let you email the report to yourself.

### Test Disconnect

You can test whether the disconnect logic is working in Android by plugging or unplugging a headset.
Just follow the instructions in red. You will get a report at the end that you can SHARE by GMail or Drive.

### Data Paths

This checks for dead speaker and mic channels, dead Input Presets and other audio data path problems.

1. Tap "DATA PATHS" button.
1. Unplug or disconnect any headphones.
1. Set volume to medium high.
1. Place the phone on a table in a quiet room and hit START.
1. Wait a few minutes, quietly, for the test to complete. You will hear some sine tones.
1. You will get a report at the end that you can SHARE by GMail or Drive.