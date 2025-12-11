# dxf2jeo
Converts a 2D .dxf file into Geometric Json .jeo file

## Build
```
python -m venv .venv
.venv\Scripts\activate.bat
pip install "cmake<4" conan
set CONAN_HOME=%CD%\.conan
conan profile detect
conan create conan/recipes/libdxfrw/all/conanfile.py --version 2.2.0 --user dxf2jeo --channel stable --settings build_type=Release
conan install . --build=missing --settings build_type=Release
cmake --preset conan-default
cmake --build --preset conan-release
ctest --preset conan-release
cmake --install build --prefix install
```