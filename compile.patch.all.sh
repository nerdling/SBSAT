#!/bin/sh

find src -name Makefile.in -exec ./compile.patch.sh \{\} \;
