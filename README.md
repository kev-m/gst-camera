# GStreamer Video as WMF Virtual Camera
This project was inspired by the fact that modern video conference tools, such as Teams or Google Meet, do not allow me to select a lower camera resolution.

My webcam has a wide-angle lens which is very suited to a conference room with multiple participants, but in a one-on-one video call, I occupy only a small portion of the video.

My solution? Create a virtual camera with a reduced video resolution and use that for video conferencing.

## Windows Virtual Camera
The Windows [IMFVirtualCamera interface]([url](https://learn.microsoft.com/en-us/windows/win32/api/mfvirtualcamera/nn-mfvirtualcamera-imfvirtualcamera)) supports 
creating a software camera that can be discovered and used by apps as if it was a hardware capture device.

## Gstreamer
[GStreamer]([url](https://gstreamer.freedesktop.org/)) is a fantastic mdeia processing framework that supports constructing arbitrary pipelines to achieve a variety of tasks.

My idea is to create a virtual camera from a gstreamer pipeline:
```bash
C:> gst-launch-1.0.exe mfvideosrc ! "video/x-raw, format=NV12, width=800, height=600, framerate=30/1" ! queue ! mf_camera
```

This plugin, once it works, will open up a host of fun media sources that can be published as a camera for other apps to use.

# Installation
Bare-bones instructions. You need to know what you are doing before you start!
- Install Visual Studio (I use VS 2022).
- Checkout this project and load the solution.
- Build the DLL.
- Register the DLL with `regsvr32 gst-camera\x64\Debug\mf_camera.dll` (you must run this as administrator).

# Problems
Right now, the code does not work.

1. The `MediaSourceActivate` is not being invoked.
2. The virtual camera does not get registered properly.
3. The virtual camera is not available to other applications.

## Questions

1. When is the most appropriate place in the gstreamer life-cycle to register the virtual camera?
2. How do I set up the `MediaSource` and `MediaSourceActivate` class with the media format coming from the gstreamer pipeline?
3. How do I synchronise adding frames from the gstreamer pipeline with generating them in the `MediaSource`?
