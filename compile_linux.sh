#! /bin/sh

scons arch=x86_64 target=template_debug
scons arch=x86_64 target=template_release

scons arch=x86_32 target=template_debug
scons arch=x86_32 target=template_release
