# pcaudiolib-example

Plays a raw audio file.


## Usage

```
pcaudiolib-exaple <audio-file>
```

The file must contain
[raw audio data](https://en.wikipedia.org/wiki/Raw_audio_format):

  * With signed 16-bit PCM encoding,
  * In Little-endian byte order,
  * Has one channel, and
  * Has a sample rate of 44100 Hz.

If you require a raw audio file for testing,
a sample audio file is included.
There is likely to be stutters in the audio output,
since there is no attempt in preloading the audio buffer.


### Warning

The example application plays the raw data directly.
In particular, no format checking is attempted.

Picking a file with a wrong format is likely to produce __loud__ static.

Make sure to turn the volume down before testing to be on the safe side.


## Requirements

Building `pcaudiolib-example` requires

  * A [C++17](https://en.cppreference.com/w/cpp/compiler_support#cpp17)
    compiler,
  * [CMake](https://cmake.org) version 3.12,
  * A working implementation
    of [`pcaudiolib`](https://github.com/espeak-ng/pcaudiolib), and
  * [Doxygen](http://doxygen.nl) if HTML documentation is desired,
    though it is likely unnecessary since this is an application.


## Purpose

This application is made for testing `pcaudiolib` API.
In particular, it is for testing
an [experimental build](https://github.com/BoniLindsley/pcaudiolib)
targetting Windows [MSYS2](https://www.msys2.org/) MinGW64 using CMake.


## Acknowledgement and Licenses

This application is released under [GPL version 3](LICENSE.txt),
as required by the dependency `pcaudiolib`.

The include audio sample `drama.pcm`
is from [OpenGameArt](https://opengameart.org/content/cc0-scraps),
licensed under [CC0](https://creativecommons.org/publicdomain/zero/1.0),
converted to raw audio data using [FFmpeg](https://ffmpeg.org)
with the command

```
ffmpeg -i drama.mp3 -acodec pcm_s16le -f s16le -ac 1 -ar 44100 drama.pcm
```
