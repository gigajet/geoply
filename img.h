#ifndef _img_h_
#define _img_h_

/*
  Quy ước cho tiện:
  - Các hàm chỉ lưu sf::Image ra tệp, không đụng gì tới Shape.
  Chỉ thao tác với sf::Image mà thôi, img.h không biết gì về hình học.
  
  - Ảnh BMP: 8bpp (dù chỉ dùng 2 màu đen trắng), 0 là đen, 1 là trắng.
  Có dùng color pallete. BITMAPINFOHEADER 40 bytes. Không nén.
  Nguồn wikipedia.
  - Ảnh PNG: sf::Image::saveToFile() FUCK! IT'S TOO COMPLICATED
  - Ảnh JPG: sf::Image::saveToFile() FUCK! IT"S TOO COMPLICATED
*/

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <Windows.h>

//Chỉ dùng 2 màu trắng và đen thôi.
void ImageToBMP (const sf::Image &img, LPCWSTR filename) {
	  unsigned char magic[2] = {0x42, 0x4d}; //BM
    unsigned long bmpSize;
    sf::Vector2u sz = img.getSize();
    BITMAPINFOHEADER dib;
    dib.biSize = 40;
    dib.biWidth = sz.x; dib.biHeight = sz.y;
    dib.biPlanes = 1;
    dib.biBitCount = 8; //8bpp
    dib.biCompression = BI_RGB;
    dib.biSizeImage = 0;
    dib.biXPelsPerMeter = 2835;
    dib.biYPelsPerMeter = 2835;
    dib.biClrUsed = 2;
    dib.biClrImportant = 0;

    RGBQUAD pallete[2] = {{0,0,0,0},{0xFF,0xff,0xff,0}};

    //Find bmpSize
    bmpSize = sz.y * (((sz.x * dib.biBitCount + 31) / 32) * 4) + sizeof(BITMAPINFOHEADER)
              + 14 //Header
              + 8; //2 Color in the pallete
	unsigned long pixelArrayOffset = 14 + 8 + sizeof(BITMAPINFOHEADER);
	unsigned char appSpecific[4] = {0xff, 0xff, 0xff, 0xff};

	//Now start writing
	HANDLE handle = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD numwritten;
	WriteFile(handle, magic, 2, &numwritten, NULL);
	WriteFile(handle, (char*)&bmpSize, sizeof(unsigned long), &numwritten, NULL);
	WriteFile(handle, appSpecific, 4, &numwritten, NULL);
	WriteFile(handle, (char*)&pixelArrayOffset, sizeof(unsigned long), &numwritten, NULL);

	WriteFile(handle, (char*)&dib, sizeof(BITMAPINFOHEADER), &numwritten, NULL);
	WriteFile(handle, (char*)&pallete[0], 8, &numwritten, NULL);
	
	for (int i=(int)sz.y-1; i>=0; --i) {
    int byteWrittenRow = 0;
		for (int j=0; j<(int)sz.x; ++j) {
			sf::Color col = img.getPixel(j, i);
      BYTE b;
      if (col==sf::Color::Black)
        b = 0;
      else b = 1;
      WriteFile(handle, &b, 1, &numwritten, NULL);
      byteWrittenRow += (int)numwritten;
		}
    while (byteWrittenRow%4 != 0) { //Padding row with 00
      char x = 0x00;
      WriteFile(handle, &x, 1, &numwritten, NULL);
      byteWrittenRow += (int)numwritten;
    }
	}
	CloseHandle(handle);
}


#endif