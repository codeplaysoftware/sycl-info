pkgname=('sycl-info' 'target-selector')
pkgbase=sycl
pkgver=0.1
pkgrel=1
epoch=0
arch=('i686' 'x86_64' 'arm' 'armv6h' 'armv7h' 'aarch64')
url="https://github.com/codeplaysoftware/sycl-info"
license=('Apache-2.0')
groups=('sycl')
makedepends=('opencl-headers' 'nlohmann-json' 'ruby-ronn' 'cmake' 'ninja')
checkdepends=('doctest')
source=('https://github.com/bfgroup/Lyra/archive/1.1.tar.gz')
sha256sums=('c2d70a926f698fb7decb99c7215bb55ab770100f9574c290998bf91416bd8217')

prepare() {
    mkdir -p "$startdir/build-pkgbuild"

    mkdir -p "$startdir/build-pkgbuild/lyra/include"
    mv "$srcdir/Lyra-1.1/include/lyra" "$startdir/build-pkgbuild/lyra/include"
}

build() {
    cd "$startdir/build-pkgbuild"
    cmake -DBUILD_DOCS=ON \
          -DBUILD_SHARED_LIBS=OFF \
          -DBUILD_TESTING=OFF \
          -DLyra_ROOT="$startdir/build-pkgbuild/lyra" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_LIBDIR=lib \
          -DCMAKE_INSTALL_PREFIX=/usr \
          -GNinja \
          ..
    ninja -v
}

check() {
    cd "$startdir/build-pkgbuild"
    ctest .
}

package_sycl-info() {
    pkgdesc="Prints metadata about available SYCL implementations"
    depends=('target-selector')
    optdepends=('computecpp' 'trisycl-git' 'hipsycl-git')

    install -Dm 0644 "$startdir/LICENSES.TXT" "$pkgdir/usr/share/licenses/$pkgname/LICENSES.TXT"
    DESTDIR="$pkgdir/" ninja install
}

package_target-selector() {
    pkgdesc="SYCL target selector"
    depends=('ocl-icd')

    install -Dm 0644 "$startdir/LICENSES.TXT" "$pkgdir/usr/share/licenses/$pkgname/LICENSES.TXT"
    DESTDIR="$pkgdir/" ninja install
}
