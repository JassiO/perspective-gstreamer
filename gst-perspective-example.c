/* gst-perspective-example - test program for the GStreamer perspective element
 *
 * Copyright (C) 2013  Antonio Ospite <ospite@studenti.unina.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* FIXME: suppress warnings for deprecated API such as GValueArray
 * with newer GLib versions (>= 2.31.0)
 *
 * It is just not possible to switch to GArray yet because the python API in
 * 1.2.0 still passes iterable properties as GValueArray */
#define GLIB_DISABLE_DEPRECATION_WARNINGS
#include <gst/gst.h>

static gboolean on_message(GstBus *bus, GstMessage *message, gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR:
        g_error("Got ERROR");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_WARNING:
        g_warning("Got WARNING");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_EOS:
        g_main_loop_quit(loop);
        break;
    default:
        break;
    }

    return TRUE;
}

static void set_matrix(GstElement *element)
{
    GValueArray *va;
    GValue v = { 0, };
    gdouble m[9];
    guint i;

    /* Initialize a 2D perspective matrix, you can use
     * cvGetPerspectiveTransform() from OpenCV to build it
     * from a quad-to-quad transformation */
    m[0] = 1.9999999999999982;
    m[1] = 0.8333333333333287;
    m[2] = -399.99999999999926;
    m[3] = 3.9968028886505525e-15;
    m[4] = 1.9999999999999978;
    m[5] = -8.277822871605139e-13;
    m[6] = 2.428612866367518e-18;
    m[7] = 0.0020833333333333294;
    m[8] = 0.9999999999999996;

    va = g_value_array_new(1);

    g_value_init(&v, G_TYPE_DOUBLE);
    for (i = 0; i < 9; i++) {
        g_value_set_double(&v, m[i]);
        g_value_array_append(va, &v);
        g_value_reset(&v);
    }
    g_object_set(G_OBJECT(element), "matrix", va, NULL);
    g_value_array_free(va);
}

int main(void)
{
    GstElement *pipeline, *src, *capsfilter, *perspective, *conv, *sink;
    GstCaps *caps;
    GstBus *bus;
    GMainLoop *loop;
    int ret;

    gst_init(NULL, NULL);

    pipeline = gst_element_factory_make("pipeline", NULL);

    src = gst_element_factory_make("v4l2src", NULL);

    caps = gst_caps_from_string("video/x-raw");

    capsfilter = gst_element_factory_make("capsfilter", NULL);
    g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);

    perspective = gst_element_factory_make("perspective", NULL);
    set_matrix(perspective);

    conv = gst_element_factory_make("videoconvert", NULL);

    sink = gst_element_factory_make("autovideosink", NULL);
    g_return_val_if_fail(sink != NULL, -1);

    gst_bin_add_many(GST_BIN(pipeline), src, capsfilter, perspective, conv, sink, NULL);
    ret = gst_element_link_many(src, capsfilter, perspective, conv, sink, NULL);
    if (!ret) {
        g_error("Failed to link elements");
        return -2;
    }

    loop = g_main_loop_new(NULL, FALSE);

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(on_message),
             loop);
    gst_object_unref(GST_OBJECT(bus));

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_error("Failed to go into PLAYING state");
        return -3;
    }

    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    g_main_loop_unref(loop);
    gst_object_unref(pipeline);

    return 0;
}
