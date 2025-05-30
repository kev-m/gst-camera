/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2025  <<user@hostname.org>>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

 /**
  * SECTION:element-mf_camera
  *
  * FIXME:Describe mf_camera here.
  *
  * <refsect2>
  * <title>Example launch line</title>
  * |[
  * gst-launch -v -m fakesrc ! mf_camera ! fakesink silent=TRUE
  * ]|
  * </refsect2>
  */

#include "pch.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gst_mf_camera.h" 

#include "GSTVirtualCamera.h"

GST_DEBUG_CATEGORY_STATIC(gst_mfcamera_debug);
#define GST_CAT_DEFAULT gst_mfcamera_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("ANY")
);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("ANY")
);

#define gst_mfcamera_parent_class parent_class
G_DEFINE_TYPE(Gstmfcamera, gst_mfcamera, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE(mfcamera, GST_PACKAGE_NAME, GST_RANK_NONE,
    GST_TYPE_MF_CAMERA);

static void gst_mfcamera_set_property(GObject* object,
    guint prop_id, const GValue* value, GParamSpec* pspec);
static void gst_mfcamera_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec);

static gboolean gst_mfcamera_sink_event(GstPad* pad,
    GstObject* parent, GstEvent* event);
static GstFlowReturn gst_mfcamera_chain(GstPad* pad,
    GstObject* parent, GstBuffer* buf);

/* GObject vmethod implementations */

/* initialize the mf_camera's class */
static void
gst_mfcamera_class_init(GstmfcameraClass* klass)
{
    GObjectClass* gobject_class;
    GstElementClass* gstelement_class;

    gobject_class = (GObjectClass*)klass;
    gstelement_class = (GstElementClass*)klass;

    gobject_class->set_property = gst_mfcamera_set_property;
    gobject_class->get_property = gst_mfcamera_get_property;

    g_object_class_install_property(gobject_class, PROP_SILENT,
        g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
            FALSE, G_PARAM_READWRITE));

    gst_element_class_set_details_simple(gstelement_class,
        GST_PACKAGE_NAME,
        PL_CLASS,           // Classification
        GL_DESCRIPTION,     // Description
		"Kevin Meyer");	    // Author

    gst_element_class_add_pad_template(gstelement_class,
        gst_static_pad_template_get(&src_factory));
    gst_element_class_add_pad_template(gstelement_class,
        gst_static_pad_template_get(&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_mfcamera_init(Gstmfcamera* filter)
{
    filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
    gst_pad_set_event_function(filter->sinkpad,
        GST_DEBUG_FUNCPTR(gst_mfcamera_sink_event));
    gst_pad_set_chain_function(filter->sinkpad,
        GST_DEBUG_FUNCPTR(gst_mfcamera_chain));
    GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

    filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS(filter->srcpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

    filter->silent = FALSE;

    //
    // Register here?
    // Try and acquire WMF Camera
    // regsvr32 C:\Dev\WindowsWDK\gst-camera\x64\Debug\mf_camera.dll (you must run this as administrator)
    if (FAILED(RegisterVirtualCamera()))
        g_print("Failed to register virtual camera.\n");
    else
        g_print("Successfully registered virtual camera!\n");

}

static void
gst_mfcamera_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec)
{
    Gstmfcamera* filter = GST_MF_CAMERA(object);

    switch (prop_id) {
    case PROP_SILENT:
        filter->silent = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
gst_mfcamera_get_property(GObject* object, guint prop_id,
    GValue* value, GParamSpec* pspec)
{
    Gstmfcamera* filter = GST_MF_CAMERA(object);

    switch (prop_id) {
    case PROP_SILENT:
        g_value_set_boolean(value, filter->silent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/* GstElement vmethod implementations */
/* this function handles sink events */
static gboolean
gst_mfcamera_sink_event(GstPad* pad, GstObject* parent,
    GstEvent* event)
{
    Gstmfcamera* filter;
    gboolean ret;

    filter = GST_MF_CAMERA(parent);

    GST_LOG_OBJECT(filter, "Received %s event: %" GST_PTR_FORMAT,
        GST_EVENT_TYPE_NAME(event), event);

    printf("gst_mfcamera_sink_event: %d\n", GST_EVENT_TYPE(event));

    switch (GST_EVENT_TYPE(event)) {
	case GST_EVENT_STREAM_START:
	{
        // Or register here?

        /* and forward */
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    case GST_EVENT_CAPS:
    {
        GstCaps* caps;

        gst_event_parse_caps(event, &caps);
        /* do something with the caps */

        /* and forward */
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    default:
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_mfcamera_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    Gstmfcamera* filter;

    filter = GST_MF_CAMERA(parent);

    if (filter->silent == FALSE)
    {
        g_print("I'm plugged, therefore I'm in!!\n");
    }

    // Prepare to inject the buffer to WMF
    // Get the video frame data from the GstBuffer
    GstMapInfo map;
    if (!gst_buffer_map(buf, &map, GST_MAP_READ)) {
        GST_ERROR("Failed to map GstBuffer");
        return GST_FLOW_ERROR;
    }

    // Get the frame size and format
    gsize frame_size = map.size;
    const void* frame_data = map.data;

    // Get the timestamp (if available)
    GstClockTime pts = GST_BUFFER_PTS(buf);

    // Process the buffer (convert to IMFSample)
    HRESULT hr = ProcessGstBuffer(frame_data, frame_size, pts);

    // Unmap the buffer
    gst_buffer_unmap(buf, &map);

    /* just push out the incoming buffer without touching it */
	// TODO Only push if the srcpad is connected in the pipeline
	//if (filter->srcpad != NULL)
	//{
    //  if (filter->silent == FALSE)
    //      g_print("I'm forwarding...\n");
    //  return gst_pad_push(filter->srcpad, buf);
	//}

    return GST_FLOW_OK; // gst_pad_push(filter->srcpad, buf);
}


static int count = 0;

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
mf_camera_init(GstPlugin* mf_camera)
{
    printf("mf_camera_init: %d\n", ++count);
    /* debug category for filtering log messages
     *
     * exchange the string 'Template mf_camera' with your description
     */
    GST_DEBUG_CATEGORY_INIT(gst_mfcamera_debug, GST_PACKAGE_NAME, 0, "Template mf_camera");

    return GST_ELEMENT_REGISTER(mfcamera, mf_camera);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE GST_PACKAGE_NAME
#endif

 /* gstreamer looks for this structure to register mf_cameras
  *
  * exchange the string 'Template mf_camera' with your mf_camera description
  */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mf_camera,
    GST_PACKAGE_NAME,
    mf_camera_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)

// gst-launch-1.0.exe -v mfvideosrc ! video/x-raw, format=NV12, width=800, height=600, framerate=30/1, pixel-aspect-ratio=1/1 ! queue ! mf_camera
// gst-launch-1.0.exe -v fakesrc ! mf_camera
// gst-launch-1.0.exe -v fakesrc ! mf_camera silent=TRUE
// gst-launch-1.0.exe -v --gst-plugin-path=C:\Dev\WindowsWDK\gst-camera\x64\Debug fakesrc ! mf_camera silent=0

// gst-launch-1.0.exe -v --gst-plugin-path=C:\Dev\WindowsWDK\gst-camera\x64\Debug mfvideosrc ! "video/x-raw, format=NV12, width=800, height=600, framerate=30/1" ! queue ! mf_camera