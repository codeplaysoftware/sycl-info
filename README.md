# sycl-info

Welcome to `sycl-info`!

This is the readme for sycl-info, a tool designed to report the SYCL platforms
that are available to the system.

## Getting sycl-info

<table>
    <tr>
        <td><b>Packages</b></td>
        <td></td>
    </tr>
    <tr>
        <td><b>CMake</b><br>v3.5</td>
        <td><pre>
find_package(sycl-info REQUIRED)
add_custom_target(print-sycl-info SYCL::sycl-info)<br>
find_package(target-selector REQUIRED)
target_link_libraries(app PRIVATE Codeplay::target-selector)
</pre></td>
    <tr>
        <td><b>PkgConfig</b></td>
        <td>
<pre>
sycl-info
pkg-config target-selector --cflags --libs
</pre>
        </td>
    </tr>
</table>

### Using sycl-info

The [sycl-info man page](./sycl-info/doc/sycl-info.1.md) for instruction on how to use sycl-info.

This is also generated as an HTML and additionally as a man page on Linux.

You can find the generated html document here:

```
<sycl_info_install_dir>/package/share/doc/sycl-info.1.html
```

#### Linux:

On Linux you can view the man page by doing:

```
man <sycl_info_install_dir>/package/share/man/man1/sycl-info.1
```

## Building

### Options

| CMake | Conan | Default | Notes |
|:------|:------|:--------|:------|
| `BUILD_TESTING` | `build_testing` | `OFF`/`False` | Builds the unit tests. Requires doctest. |
| `BUILD_DOCS` | `build_docs` | `OFF`/`False` | Builds the man page. Requires ronn. |
| `BUILD_SHARED_LIBS` | `shared` | `OFF`/`False` | |

### Conan (>= 1.18)

Building:

```bash
git clone https://github.com/codeplaysoftware/sycl-info.git
mkdir -p build install
conan install sycl-info/ --build missing -if build/ \
    --profile default --profile sycl-info/config/conan/profiles/build-tools \
    # Optional flags:
    # [-obuild_testing=False] [-obuild_docs=False]
conan build sycl-info -bf build/ -pf install/
conan package sycl-info -bf build/ -pf install/
```

### Plain CMake

For an example of how a complete script looks like refer to the PKGBUILD.

#### Dependencies

* OpenCL 1.2 or later
    * [Headers](https://github.com/KhronosGroup/OpenCL-Headers)
    * [ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)

#### Build Dependencies

* [CMake ≥ 3.13.0](https://cmake.org/)
* [Lyra ≥ 1.1](https://github.com/bfgroup/Lyra)
* [nlohmann/json ≥ 3.6.1](https://github.com/nlohmann/json)
* [ronn](https://github.com/rtomayko/ronn) (Only if `-DBUILD_DOCS=ON`)

#### Check Dependencies

* [doctest ≥ 2.2.0](https://github.com/onqtam/doctest) (Only if `-DBUILD_TESTING=ON`)

```bash
mkdir -p dep-install/include
git clone https://github.com/KhronosGroup/OpenCL-Headers OpenCL-Headers
git clone https://github.com/KhronosGroup/OpenCL-ICD-Loader OpenCL-ICD-Loader
git clone https://github.com/nlohmann/json json
git clone https://github.com/bfgroup/Lyra Lyra
git clone https://github.com/onqtam/doctest doctest # Only needed for the tests
gem install ronn # Only needed to build the man page
cp -r OpenCL-Headers/CL install/include/
cp -r Lyra/include/lyra install/include

for dep in OpenCL-ICD-Loader json doctest; do
    mkdir ${dep}-build
    cmake -S $dep -B ${dep}-build \
        -DCMAKE_INSTALL_PREFIX=install \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build ${dep}-build --target install
done

mkdir build install
git clone https://this/repo.git sycl-info
cmake -S sycl-info -B build \
    # Optional flags:
    # [-DBUILD_TESTING=OFF] [-DBUILD_DOCS=OFF] \
    -DCMAKE_PREFIX_PATH=dep-install \
    -DCMAKE_INSTALL_PREFIX=install \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build/ --target install
```

### Arch Linux

Note that this PKGBUILD serves a full example of how the project is supposed to
be packaged for a Linux distribution and will be moved to the AUR in the future.

```bash
makepkg -si
```

## Packaging

After sycl-info is built it can be packaged into a `.deb`, `.rpm`, or a tarball using CPack:

**Tarball**:
```bash
conan install ..
    # Optional, but recommended:
    # -obuild_docs=True -oshared=True
conan build ..
source ./activate_run.sh
cpack -G TXZ
```

**Source Tarball**:
```bash
conan install ..
conan build .. -c # configure only
source ./activate_run.sh
cpack -G TXZ --config CPackSourceConfig.cmake
```

**Ubuntu**:
```bash
conan install ..
    # Optional, but recommended:
    # -obuild_docs=True -oshared=True
conan build ..
source ./activate_run.sh
cpack -G DEB
sudo dpkg -i packages/sycl-info-0.1-Linux.deb packages/target-selector-0.1-Linux.deb
sycl-info --help
man sycl-info
```

The generated packages will end up in `build/packages`.
