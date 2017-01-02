#include <gst/gst.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#define GETTEXT_PACKAGE "gtk20"
#include <glib/gi18n-lib.h>

typedef struct
{
  guint number;
  GMainLoop *loop;
  GstBuffer *buffer;
  GstMapInfo map;

  GstElement *pipeline, *source, *decoder, *convert, *pitch, *sink;
  

  guint sourceid;        /* To control the GSource */
  guint count;

} Entries;


static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *convert = (GstElement *) data;
  gboolean result;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (convert, "sink");

  result = gst_pad_link (pad, sinkpad);
  if (result == FALSE) {
	g_print("failed to link pads\n");
  } else {
        g_print("linked pads\n");
  }

  gst_object_unref (sinkpad);
}


int
main (int   argc,
      char *argv[])
{

  GstBus *bus;
  guint bus_watch_id;
  gboolean result;
  GOptionContext *ctx;
  GError *err = NULL;
  static gchar *filename;

  Entries *w = g_slice_new (Entries);

  GOptionEntry options[] = {
    {"filename", 'f', 0, G_OPTION_ARG_FILENAME, &filename,
      "input music file", NULL},
    {NULL}
  };
  ctx = g_option_context_new ("[jamplayer arguments]");
  g_option_context_add_main_entries (ctx, options, GETTEXT_PACKAGE);
  g_option_context_add_group (ctx, gst_init_get_option_group ());
  if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
    g_print ("Error initializing: %s\n", GST_STR_NULL (err->message));
    exit (1);
  }
  g_option_context_free (ctx);

  if (filename == NULL) {
    g_print("fail\n");
  }

  w->loop = g_main_loop_new (NULL, FALSE);

  w->pipeline = gst_pipeline_new ("jamplayer");
  w->source   = gst_element_factory_make ("filesrc",       "source");
  w->decoder   = gst_element_factory_make("decodebin", "decoder");
  w->convert  = gst_element_factory_make("audioconvert", "convert");
  w->pitch    = gst_element_factory_make("pitch", "pitch"); 
  w->sink    = gst_element_factory_make("autoaudiosink", "sink");

  //expand on this should create func to do it
  if (!w->pipeline) {
	g_printerr ("pipeline fail.\n");
  }

  g_object_set (G_OBJECT (w->source), "location", "/home/phil/Downloads/11 - Foxey Lady.mp3", NULL);
   g_object_set (G_OBJECT (w->pitch), "tempo", 0.5, NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (w->pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, w->loop);
  gst_object_unref (bus);

  gst_bin_add_many (GST_BIN (w->pipeline),
                    w->source, w->decoder, w->convert, w->pitch, w->sink,
                    NULL
		    );

  result =  gst_element_link_many(w->source, w->decoder, NULL);
  if (result == FALSE) {
	g_print("failed to link elements! 1\n");
	return -1;
  }
/*
  result =  gst_element_link_many(w->decoder, w->convert, NULL);
  if (result == FALSE) {
	g_print("failed to link elements! 2\n");
	return -1;
  }
*/

 result =  gst_element_link_many(w->convert, w->pitch, w->sink, NULL);
  if (result == FALSE) {
	g_print("failed to link elements! 3\n");
	return -1;
  }

 g_signal_connect (w->decoder, "pad-added", G_CALLBACK (on_pad_added), w->convert);

  gst_element_set_state (w->pipeline, GST_STATE_PLAYING);

  g_main_loop_run (w->loop);

  g_print ("Returned, stopping playback\n");
  gst_element_set_state (w->pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (w->pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (w->loop);

  return 0;

}
