#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:?usage: build-native-packages.sh <version> <tag> <ofscc> <runtime> <out-dir>}"
TAG="${2:?usage: build-native-packages.sh <version> <tag> <ofscc> <runtime> <out-dir>}"
OFSCC_BIN="${3:?usage: build-native-packages.sh <version> <tag> <ofscc> <runtime> <out-dir>}"
RUNTIME_LIB="${4:?usage: build-native-packages.sh <version> <tag> <ofscc> <runtime> <out-dir>}"
OUT_DIR="${5:?usage: build-native-packages.sh <version> <tag> <ofscc> <runtime> <out-dir>}"
mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PKG_NAME="obsidian-fault-script"
SUMMARY="Obsidian Fault Script self-hosted compiler and standard library"
DESCRIPTION="Obsidian Fault Script (OFS) language toolchain: command wrapper, self-hosted native compiler, runtime library, standard library, and compiler sources."
LICENSE_NAME="BSL-1.0"
ARCH_DEB="amd64"
ARCH_RPM="x86_64"
ARCH_ARCH="x86_64"

need() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "missing required tool: $1" >&2
    exit 1
  fi
}

install_payload() {
  local dest="$1"
  install -d "$dest/usr/bin"
  install -d "$dest/usr/lib/ofs"
  install -d "$dest/usr/share/ofs/stdlib"
  install -d "$dest/usr/share/ofs/ofscc-src"
  install -d "$dest/usr/share/doc/$PKG_NAME"
  install -d "$dest/usr/share/licenses/$PKG_NAME"

  sed "s/__OFS_VERSION__/${VERSION}/g" "$ROOT/packaging/linux/ofs" > "$dest/usr/bin/ofs"
  install -m 0755 "$OFSCC_BIN" "$dest/usr/lib/ofs/ofscc"
  install -m 0644 "$RUNTIME_LIB" "$dest/usr/lib/ofs/libofs_runtime.a"
  ln -s ../lib/ofs/ofscc "$dest/usr/bin/ofscc"
  chmod 0755 "$dest/usr/bin/ofs"

  cp "$ROOT"/ofs/stdlib/*.ofs "$dest/usr/share/ofs/stdlib/"
  cp -r "$ROOT"/ofscc/* "$dest/usr/share/ofs/ofscc-src/"
  install -m 0644 "$ROOT/LICENSE" "$dest/usr/share/licenses/$PKG_NAME/LICENSE"
  install -m 0644 "$ROOT/README.md" "$dest/usr/share/doc/$PKG_NAME/README.md" 2>/dev/null || true
}

build_deb() {
  need dpkg-deb
  local work="$OUT_DIR/deb-root"
  rm -rf "$work"
  install_payload "$work"
  install -d "$work/DEBIAN"
  cat > "$work/DEBIAN/control" <<EOF
Package: $PKG_NAME
Version: $VERSION
Section: devel
Priority: optional
Architecture: $ARCH_DEB
Maintainer: Obsidian Fault Script <noreply@github.com>
Depends: clang, libc6
Description: $SUMMARY
 $DESCRIPTION
EOF
  dpkg-deb --build --root-owner-group "$work" "$OUT_DIR/ofs-debian-ubuntu-x64-installer-${TAG}.deb"
}

build_rpm() {
  need rpmbuild
  local top="$OUT_DIR/rpmbuild"
  local payload="$OUT_DIR/rpm-payload"
  rm -rf "$top" "$payload"
  install_payload "$payload"
  mkdir -p "$top/BUILD" "$top/BUILDROOT" "$top/RPMS" "$top/SOURCES" "$top/SPECS" "$top/SRPMS"
  tar czf "$top/SOURCES/$PKG_NAME-$VERSION.tar.gz" -C "$payload" .
  cat > "$top/SPECS/$PKG_NAME.spec" <<EOF
Name:           $PKG_NAME
Version:        $VERSION
Release:        1%{?dist}
Summary:        $SUMMARY
License:        $LICENSE_NAME
URL:            https://github.com/Samwns/Obsidian-Fault-Script
Source0:        %{name}-%{version}.tar.gz
Requires:       clang

%description
$DESCRIPTION

%prep

%build

%install
mkdir -p %{buildroot}
tar xzf %{SOURCE0} -C %{buildroot}

%files
/usr/bin/ofs
/usr/bin/ofscc
/usr/lib/ofs/ofscc
/usr/lib/ofs/libofs_runtime.a
/usr/share/ofs/stdlib
/usr/share/ofs/ofscc-src
/usr/share/doc/$PKG_NAME
/usr/share/licenses/$PKG_NAME/LICENSE
EOF
  rpmbuild --define "_topdir $top" -bb "$top/SPECS/$PKG_NAME.spec"
  cp "$top/RPMS/$ARCH_RPM"/*.rpm "$OUT_DIR/ofs-fedora-x64-installer-${TAG}.rpm"
}

build_arch() {
  need bsdtar
  need zstd
  local work="$OUT_DIR/arch-root"
  rm -rf "$work"
  install_payload "$work"
  local installed_size
  installed_size="$(du -sb "$work" | awk '{print $1}')"
  cat > "$work/.PKGINFO" <<EOF
pkgname = $PKG_NAME
pkgbase = $PKG_NAME
pkgver = $VERSION-1
pkgdesc = $SUMMARY
url = https://github.com/Samwns/Obsidian-Fault-Script
builddate = $(date +%s)
packager = Obsidian Fault Script CI
size = $installed_size
arch = $ARCH_ARCH
license = $LICENSE_NAME
depend = clang
EOF
  (cd "$work" && bsdtar --format=mtree --options='!all,use-set,type,uid,gid,mode,time,size,sha256,link' -cf .MTREE $(find . -mindepth 1 ! -name .MTREE ! -name .PKGINFO | sort))
  (cd "$work" && bsdtar --zstd -cf "$OUT_DIR/ofs-arch-x64-installer-${TAG}.pkg.tar.zst" .)
}

mkdir -p "$OUT_DIR"
build_deb
build_rpm
build_arch
ls -lh "$OUT_DIR"/*"${TAG}"*
