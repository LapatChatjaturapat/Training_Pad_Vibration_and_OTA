# OTA Firmware

This is a part of firmware for over-the-air uploading.

## Overview
For convenience in development and testing, we decide to use OTA uploading to upload new firmware to the training pad. 

## Main Feature
- upload the binary code of the new firmware via the training pad's wifi 

## File
1. Training_Pad_OTA_Firmware.c: a part of firmware for OTA uploading
2. index.html: a web hosted by the training pad, use to upload the file to the pad
3. sdkconfig.defaults: config file, contains crucial config for using OTA feature

## Discliamer
This code may cannot complied or built due to lacking of function provided in the full project.  
