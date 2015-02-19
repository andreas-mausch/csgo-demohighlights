#pragma once

struct IWICStream;
struct IWICImagingFactory;
struct IWICBitmapDecoder;
struct IWICBitmapFrameDecode;
struct AnimatedGifFrame;

#include <string>

class ImageDecoder
{
private:
	IWICImagingFactory *factory;

	HBITMAP loadImage(IWICBitmapDecoder *decoder);

	IWICStream *createStreamFromBytes(unsigned char *bytes, int size);
	IWICBitmapDecoder *createDecoderFromStream(IWICStream *stream);
	HBITMAP convertFrameToBitmap(IWICBitmapFrameDecode *frame);

public:
	ImageDecoder();
	~ImageDecoder();

	HBITMAP loadImage(const std::string &filename);
	HBITMAP loadImageFromResource(const WCHAR *name, const WCHAR *type);
	HBITMAP loadImage(unsigned char *bytes, int size);
	std::string loadString(const WCHAR *name, const WCHAR *type);

};
