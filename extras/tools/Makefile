TOOLS = /Users/abhishekpanthee/VLC-Remake/extras/tools
MAKEFLAGS += -j10
CMAKEFLAGS += --parallel=10
PREFIX=$(abspath ./build)
PATH=${PREFIX}/bin:/Users/abhishekpanthee/VLC-Remake/extras/tools/build/bin::/opt/homebrew/opt/python@3.12/libexec/bin:/bin:/sbin:/usr/bin:/usr/sbin

.zstd .gperf .ninja .meson .help2man .gettext .nasm .flex .bison .xz .sed .tar .cmake .pkg-config .libtool .m4 .automake .autoconf :
	@echo "Available in the system"
	touch $@

.SECONDEXPANSION:
.ant .configguess : $$(subst .,.build,$$@)
	touch $@

all:  .buildconfigguess .buildant
	@echo "You are ready to build VLC and its contribs"

include $(TOOLS)/tools.mak

fetch: .getconfigguess .getant
