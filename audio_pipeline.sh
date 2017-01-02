#!/bin/bash

#sample pipelines to test the gstreamer plugins

#gst-launch-1.0 playbin uri="file:///home/phil/Downloads/11 - Foxey Lady.mp3"

gst-launch-1.0 filesrc location="Downloads/11 - Foxey Lady.mp3" ! decodebin ! audioconvert ! pitch tempo=1.5 ! autoaudiosink
