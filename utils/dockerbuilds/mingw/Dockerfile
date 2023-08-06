FROM wesnoth/wesnoth:mingw-master
ENV DEBIAN_FRONTEND=noninteractive

COPY get_dlls.py /scripts/get_dlls.py

RUN wget https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-pango-1.50.11-1-any.pkg.tar.zst && wget https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-pango-1.50.11-1-any.pkg.tar.zst.sig && pacman-cross --noconfirm -U file:///mingw-w64-x86_64-pango-1.50.11-1-any.pkg.tar.zst

ENTRYPOINT cd /output && scons -j `nproc` arch=x86-64 prefix=/windows/mingw64 gtkdir=/windows/mingw64 host=x86_64-w64-mingw32 -Y /wesnoth && python3 /scripts/get_dlls.py && rm -rf packaging && ln -sf /wesnoth/doc /wesnoth/packaging /wesnoth/data /wesnoth/fonts /wesnoth/images /wesnoth/sounds /wesnoth/README.md /wesnoth/copyright /wesnoth/COPYING /wesnoth/changelog.md /wesnoth/cwesnoth.cmd . && scons -Y /wesnoth windows-installer
