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

_gitroot=http://github.com/ragnarok2040/image.git
_gitname=image

build() {
  # Setup PKGBUILD ps2dev environment
  source /etc/profile.d/ps2dev.sh

  cd $srcdir

  if [ -d "${srcdir}/${_gitname}" ]; then
    msg "Found existing copy..."
    msg "Updating..."
    cd ${_gitname}
    git pull --rebase
    cd $srcdir
  else
    msg "Checking out fresh copy..."
    git clone ${_gitroot}
  fi

  msg "Cleaning up sources..."
  rm -rf image-build || return 1

  msg "Copying sources..."
  cp -r image image-build

  cd image-build

  msg "Starting make..."
  make || return 1
  make DESTDIR=$pkgdir install || return 1

}
