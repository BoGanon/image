# Contributor: ragnarok2040 at gmail dot com
# ps2sdk-ports-libimage PKGBUILD

pkgname=ps2sdk-ports-libimage-git
pkgver=20100806
pkgrel=1
pkgdesc="libimage for ps2sdk"
arch=('i686' 'x86_64')
license=('Custom')
url="http://www.ps2dev.org/"
depends=('ps2sdk-git' 'ps2sdk-ports-libpng' 'ps2sdk-ports-zlib')
makedepends=('cross-ps2-binutils>=2.14'
             'cross-ps2-ee-newlib>=1.10.0'
             'cross-ps2-ee-gcc'
             'cross-ps2-iop-gcc'
             'make' 'gcc' 'patch' 'subversion')
options=('!strip' '!libtool')
source=("image.tar.gz")
md5sums=('b700873bb4334cb88f5e279d587393c3')

_gitroot=http://github.com/ragnarok2040/libimage.git
_gitname=libimage

build() {
  # Setup PKGBUILD ps2dev environment
  source /etc/profile.d/ps2dev.sh

  cd $srcdir

  msg "Cleaning up sources..."
  rm -rf image-build || return 1

  msg "Copying sources..."
  cp -r image image-build

  cd image-build

  msg "Starting make..."
  make || return 1
  make DESTDIR=$pkgdir install || return 1

}
