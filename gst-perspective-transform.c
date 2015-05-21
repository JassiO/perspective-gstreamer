#define GLIB_DISABLE_DEPRECATION_WARNINGS
#include <gst/gst.h>

#include <stdio.h>
#include <stdlib.h>

#include <err.h>    /* err */
#include <stdio.h>  /* fopen, fgetln, fputs, fwrite */
#include <stdlib.h>

// Homography matrix
float m[9];

static void read_in_homography(){

    FILE *fp;

    fp = fopen("homography.txt","r"); // read mode

    for (int i = 0; i < 9; ++i) {
        fscanf(fp, "%f", &m[i]);
        printf("%f\n", m[i]);
    }

    fclose (fp);
}

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

    guint i;

    /* Initialize a 2D perspective matrix, you can use
     * cvGetPerspectiveTransform() from OpenCV to build it
     * from a quad-to-quad transformation */

    read_in_homography();
    
    /*gdouble xp, yp, w, xi, yi, x, y;

    /* Matrix multiplication 
    xp = (m[0] * x + m[1] * y + m[2]);
    yp = (m[3] * x + m[4] * y + m[5]);
    w = (m[6] * x + m[7] * y + m[8]);

    /* Perspective division 
        xi = xp / w;
    yi = yp / w;

    g_message(x, y, xi, yi);*/

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
