FROM archlinux

# Add ownstuff third party repository because nsis is unavailable from default archlinux repos
COPY ownstuff.conf ownstuff.conf
RUN cat < ownstuff.conf >> /etc/pacman.conf

RUN pacman --noconfirm -Syu && pacman --noconfirm -S wget

# Trust ownstuff repo key to avoid prompts when installing nsis
RUN pacman-key --init && \
    pacman-key --recv-keys B9E36A7275FC61B464B67907E06FE8F53CDC6A4C && \
    pacman-key --lsign-key B9E36A7275FC61B464B67907E06FE8F53CDC6A4C

RUN wget https://github.com/msys2/msys2-installer/releases/download/2023-10-26/msys2-base-x86_64-20231026.tar.xz  && \
    tar xJvf msys2-base-x86_64-20231026.tar.xz  && \
    cp /msys64/etc/pacman.d/* /etc/pacman.d/ && \
    cp /msys64/usr/share/pacman/keyrings/* /usr/share/pacman/keyrings/

# setup msys2 keys
RUN pacman-key --populate msys2

RUN pacman --root /msys64 --config /msys64/etc/pacman.conf --noconfirm -Syu

COPY bin/pacman-cross /bin/pacman-cross
