EAPI=2
inherit eutils toolchain-funcs flag-o-matic games

DESCRIPTION="Battle for Wesnoth - A fantasy turn-based strategy game"
HOMEPAGE="http://www.wesnoth.org/"
SRC_URI="mirror://sourceforge/wesnoth/${P}.tar.bz2"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64 ~ppc ~ppc64 ~sparc ~x86 ~x86-fbsd"
IUSE="+client +editor lite nls server tinygui tools"

RDEPEND=">=media-libs/libsdl-1.2.7
	media-libs/sdl-net
	dev-libs/boost
	client? (
		>=media-libs/libsdl-1.2.7[X]
		>=media-libs/sdl-mixer-1.2[vorbis]
		>=media-libs/sdl-image-1.2[png,jpeg]
		>=media-libs/sdl-ttf-2.0.8
		x11-libs/libX11
		x11-libs/pango
	)
	virtual/libintl
	tools? (
		>=media-libs/libsdl-1.2.7[X]
		>=media-libs/sdl-mixer-1.2[vorbis]
		>=media-libs/sdl-image-1.2
		>=media-libs/sdl-ttf-2.0.8
		x11-libs/libX11
	)"
DEPEND="${RDEPEND}
	client? (
		tinygui? ( media-gfx/imagemagick[jpeg,png] )
	)
	nls? ( sys-devel/gettext )
	>=dev-util/scons-0.96.93"

use_target() {
	useq $1 && echo $2
}

use_variable() {
	echo ${2:-$1}=$(useq $1 && echo ${3:-yes} || echo ${4:-no})
}

pkg_setup() {
	games_pkg_setup
}

src_prepare() {
	if use server ; then
		sed \
			-e "s:GAMES_BINDIR:${GAMES_BINDIR}:" \
			-e "s:GAMES_STATEDIR:${GAMES_STATEDIR}:" \
			-e "s/GAMES_USER_DED/${GAMES_USER_DED}/" \
			-e "s/GAMES_GROUP/${GAMES_GROUP}/" "${FILESDIR}"/wesnothd.rc \
			> "${T}"/wesnothd \
			|| die "sed failed"
	fi
}

src_configure() {
	true
}

src_compile() {
	local myconf="build=release desktop_entry=yes"

	local SCONSOPTS=`echo ${MAKEOPTS} | \
		sed s/-j$/-j256/\;s/-j[[:space:]]/-j256\ / |
		sed s/--jobs$/--jobs=256/\;s/--jobs[[:space:]]/--jobs=256\ /`

	einfo "running scons with ${SCONSOPTS}"

	filter-flags -ftracer -fomit-frame-pointer
	if [[ $(gcc-major-version) -eq 3 ]] ; then
		filter-flags -fstack-protector
		append-flags -fno-stack-protector
	fi
	if use server ; then
		myconf="${myconf} server_uid=${GAMES_USER_DED}"
		myconf="${myconf} server_gid=${GAMES_GROUP}"
	fi

	if has ccache $FEATURES; then
		myconf="${myconf} ccache=yes"
	else
		myconf="${myconf} ccache=no"
	fi

	scons $myconf \
		${SCONSOPTS/-l[0-9]} \
		$(use_target client wesnoth) \
		$(use_target server wesnothd) \
		$(use_target server campaignd) \
		$(use_target tools cutter) \
		$(use_target tools exploder) \
		$(use_variable editor) \
		$(use_variable tinygui gui tiny normal) \
		$(use_variable lite lowmem) \
		$(use_variable nls) \
		localedirname=/usr/share/locale \
		prefix=/usr/games \
		prefsdir=.wesnoth-1.5 \
		icondir=/usr/share/icons \
		desktopdir=/usr/share/applications \
		docdir=/usr/share/doc/${PF} \
		default_targets=none ||
		die "scons build failed"
}

src_install() {
	scons install destdir=${D} --implicit-deps-unchanged ||
	die "scons install failed"
	dodoc changelog
	if use server; then
		keepdir "${GAMES_STATEDIR}/run/wesnothd"
		doinitd "${T}"/wesnothd || die "doinitd failed"
	fi
	prepgamesdirs
}
