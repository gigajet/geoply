REM Thay dòng -L"..." thành chỗ để thư viện static SFML
REM Thay dòng -I"..." thành chỗ include sfml
REM Nếu không dùng MinGW thì thay đổi.

g++ main.cpp geo.h img.h -o GeoPlay.exe -O2 -L"C:/sfml/lib" -I"C:/sfml/include"  -std=c++17 -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -lcomdlg32
