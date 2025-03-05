# Vibration Firmware

This is a part of firmware for detecting and calculating the signal sent by the vibration sensor.

## Overview
To measure athletes' performance using a training pad, athletes need to hit targets in the corresponding pattern given by the coach. On the target, there is a sensor that will send the signal when the sensor detects vibration. This part of the firmware will calculate the signal from the sensor to decide whether the target was hit and met the condition provided by coach or not with an algorithm created by observing the characteristic of the signal.

## Main Feature
- detect the hit signal from the sensor
- determine the power from hitting
- compare the calculated power with the condition sent by coach via the mobile application

## File
1. Training_Pad_Vibration_Firmware.c: a part of firmware for detecting and calculating the signal sent by the vibration sensor.

## Discliamer
This code may cannot complied or built due to lacking of function provided in the full project.  
