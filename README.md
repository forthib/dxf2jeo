# dxf2jeo
Converts a 2D .dxf file into Geometric Json .jeo file

## Build
```
python -m venv .venv
.venv\Scripts\activate.bat
pip install cmake conan
conan install . --build=missing
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --prefix install
```