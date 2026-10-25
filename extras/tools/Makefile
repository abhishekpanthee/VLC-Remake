TOOLS = /Users/abhishekpanthee/VLC-Remake/extras/tools
MAKEFLAGS += -j10
CMAKEFLAGS += --parallel=10
PREFIX=$(abspath ./build)
PATH=${PREFIX}/bin:/Users/abhishekpanthee/VLC-Remake/extras/tools/build/bin:::/bin:/sbin:/usr/bin:/usr/sbin

.gperf .flex .sed .tar :
	@echo "Available in the system"
	touch $@

.SECONDEXPANSION:
.zstd .ninja .meson .help2man .gettext .nasm .bison .xz .ant .zstd .cmake .pkg-config .libtool .m4 .automake .autoconf .configguess : $$(subst .,.build,$$@)
	touch $@

all:  .buildconfigguess .buildautoconf .buildautomake .buildm4 .buildlibtool .buildpkg-config .buildcmake .buildzstd .buildant .buildxz .buildbison .buildnasm .buildgettext .buildmeson .buildninja .buildzstd
	@echo "You are ready to build VLC and its contribs"

include $(TOOLS)/tools.mak

fetch: .getconfigguess .getautoconf .getautomake .getm4 .getlibtool .getpkg-config .getcmake .getzstd .getant .getxz .getbison .getnasm .getgettext .getmeson .getninja .getzstd
