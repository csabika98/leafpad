#!/bin/bash
#
# Build Leafpad.app for macOS.
#
# The bundle is not relocatable: the binary keeps linking against the GTK+ 3
# dylibs under the Homebrew prefix, so it runs on a machine that has them and
# is not something to hand to someone else. What the bundle does buy is a Dock
# icon, a name, double-click launch and Finder "Open With" association.
#
# Usage:  macos/build-app.sh [--prefix /opt/homebrew] [--install [dir]]
#
#   --install   also place the bundle in /Applications (or the given
#               directory), which is where a Dock entry should point: the
#               copy in this tree is covered by .gitignore and would be
#               removed by "git clean -xdf".
#
set -euo pipefail

BREW_PREFIX="${BREW_PREFIX:-$(brew --prefix)}"
INSTALL_DIR=""

while [ $# -gt 0 ]; do
	case "$1" in
	--prefix)  BREW_PREFIX="$2"; shift 2 ;;
	--install)
		if [ $# -ge 2 ] && [ "${2#--}" = "$2" ]; then
			INSTALL_DIR="$2"; shift 2
		else
			INSTALL_DIR="/Applications"; shift
		fi ;;
	*) echo "unknown option: $1" >&2; exit 2 ;;
	esac
done

cd "$(dirname "$0")/.."
ROOT="$PWD"
APP="$ROOT/Leafpad.app"
RES="$APP/Contents/Resources"
VERSION=$(sed -n 's/^AC_INIT(leafpad, \([^,]*\).*/\1/p' configure.ac)

export PATH="$BREW_PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$BREW_PREFIX/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

echo "==> Leafpad $VERSION -> $APP"

# --- configure & build, installing into the bundle ---------------------------
if [ ! -f configure ]; then
	echo "==> bootstrapping autotools"
	glib-gettextize --force --copy >/dev/null
	intltoolize --copy --force --automake >/dev/null
	autoreconf -fi >/dev/null
fi

# Replace the contents, not the bundle directory itself. The Dock and Finder
# hold a bookmark to Leafpad.app; deleting and recreating it changes the inode
# and leaves them pointing at something that no longer exists.
rm -rf "$APP/Contents"
mkdir -p "$APP/Contents/MacOS" "$RES"

echo "==> configure"
./configure --prefix="$RES" >/dev/null

echo "==> make"
make >/dev/null
make install >/dev/null

# make install puts the binary in Resources/bin; the bundle executable is the
# launcher below, which sets the environment first.
mv "$RES/bin/leafpad" "$RES/bin/leafpad-bin"

# --- icon --------------------------------------------------------------------
echo "==> icon"
ICONSET=$(mktemp -d)/leafpad.iconset
mkdir -p "$ICONSET"
SVG="$ROOT/data/icons/scalable/leafpad.svg"

render() { rsvg-convert -w "$1" -h "$1" "$SVG" -o "$ICONSET/$2"; }
render 16   icon_16x16.png
render 32   icon_16x16@2x.png
render 32   icon_32x32.png
render 64   icon_32x32@2x.png
render 128  icon_128x128.png
render 256  icon_128x128@2x.png
render 256  icon_256x256.png
render 512  icon_256x256@2x.png
render 512  icon_512x512.png
render 1024 icon_512x512@2x.png

iconutil -c icns "$ICONSET" -o "$RES/leafpad.icns"
rm -rf "$(dirname "$ICONSET")"

# --- Info.plist --------------------------------------------------------------
cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
	"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleName</key>              <string>Leafpad</string>
	<key>CFBundleDisplayName</key>       <string>Leafpad</string>
	<key>CFBundleExecutable</key>        <string>Leafpad</string>
	<key>CFBundleIdentifier</key>        <string>org.tarot.leafpad</string>
	<key>CFBundleIconFile</key>          <string>leafpad</string>
	<key>CFBundlePackageType</key>       <string>APPL</string>
	<key>CFBundleShortVersionString</key><string>$VERSION</string>
	<key>CFBundleVersion</key>           <string>$VERSION</string>
	<key>NSHighResolutionCapable</key>   <true/>
	<key>LSMinimumSystemVersion</key>    <string>11.0</string>
	<key>CFBundleDocumentTypes</key>
	<array>
		<dict>
			<key>CFBundleTypeName</key><string>Text Document</string>
			<key>CFBundleTypeRole</key><string>Editor</string>
			<key>LSHandlerRank</key><string>Alternate</string>
			<key>LSItemContentTypes</key>
			<array>
				<string>public.plain-text</string>
				<string>public.text</string>
				<string>public.source-code</string>
				<string>public.data</string>
			</array>
		</dict>
	</array>
</dict>
</plist>
PLIST

# --- launcher ----------------------------------------------------------------
# Finder starts apps with a bare environment, so the GTK data paths that a
# shell would have provided are set here.
cat > "$APP/Contents/MacOS/Leafpad" <<'LAUNCHER'
#!/bin/bash
BUNDLE="$(cd "$(dirname "$0")/../.." && pwd)"
RES="$BUNDLE/Contents/Resources"
BREW="__BREW_PREFIX__"

export XDG_DATA_DIRS="$RES/share:$BREW/share:/usr/local/share:/usr/share"
export XDG_CONFIG_DIRS="$BREW/etc/xdg:/etc/xdg"
export GDK_PIXBUF_MODULE_FILE="$BREW/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"

# Adwaita is compiled into GTK+ 3, so only the icon theme needs pointing at.
export GTK_THEME="${GTK_THEME:-Adwaita}"

exec "$RES/bin/leafpad-bin" "$@"
LAUNCHER

sed -i '' "s|__BREW_PREFIX__|$BREW_PREFIX|" "$APP/Contents/MacOS/Leafpad"
chmod +x "$APP/Contents/MacOS/Leafpad"

# --- icon caches the bundle needs at runtime ---------------------------------
if [ -d "$RES/share/icons/hicolor" ]; then
	"$BREW_PREFIX/opt/gtk+3/bin/gtk3-update-icon-cache" \
		--force --quiet "$RES/share/icons/hicolor" 2>/dev/null || true
fi

LSREGISTER=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister

# Refresh LaunchServices so the icon and document types are picked up now
# rather than whenever it next rescans.
touch "$APP"
"$LSREGISTER" -f "$APP" 2>/dev/null || true

echo "==> done: $APP"

# --- optional install --------------------------------------------------------
if [ -n "$INSTALL_DIR" ]; then
	DEST="$INSTALL_DIR/Leafpad.app"
	echo "==> installing to $DEST"

	# Same reasoning as above: replace Contents rather than the bundle
	# directory, so a Dock entry pointing at $DEST keeps resolving.
	if pgrep -f "$DEST/Contents/" >/dev/null 2>&1; then
		echo "    (Leafpad is running from $DEST -- quit it first)" >&2
		exit 1
	fi
	mkdir -p "$DEST"
	rm -rf "$DEST/Contents"
	cp -R "$APP/Contents" "$DEST/Contents"

	touch "$DEST"
	"$LSREGISTER" -f "$DEST" 2>/dev/null || true
	echo "==> done: $DEST"
	echo "    open $DEST"
else
	echo "    open $APP"
fi
