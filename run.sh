#!/bin/bash

# This script does the following:
# 	(1) Go to Datalot's Dropcam stream. Capture a 2 second video and save it as a FLV video.
# 	(2) Extract the first frame from the video and save it as an image file.
#	(3) Run a script to do color detection on the above image file and tweet the food trucks found.

# Step (1)
rtmpdump -r "rtmpe://stream-bravo.dropcam.com:1935/nexus" -a "nexus" -f "LNX 11,2,202,280" -W "https://www.dropcam.com/e/5a6a099c3d894d0284560626c7944d50?autoplay=true" -p "https://www.dropcam.com" -C S:none -C B:1 -y "5a6a099c3d894d0284560626c7944d50" -o lot.flv -B 2;

# Step (2)
ffmpeg -i lot.flv -an -vcodec png -vframes 1 -ss 00:00:01 -y lot_capture.png

# Step (3)
./findmesometrucks lot_capture.png 1
