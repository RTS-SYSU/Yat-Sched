#!/bin/sh

tmux split-window
timestamp=$(date +%s)
st-trace-schedule $timestamp