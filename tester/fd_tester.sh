#!/bin/bash
while true; do lsof | grep webserv | wc -l;sleep 0.4;done;