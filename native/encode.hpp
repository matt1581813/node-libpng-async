#ifndef ENCODE_HPP
#define ENCODE_HPP

#include <nan.h>
#include <png.h>
#include <zlib.h>

using namespace std;

#define INPUT_DATA_RGBA  0
#define INPUT_DATA_ARGB  1

NAN_METHOD(encode);

NAN_MODULE_INIT(InitEncode);

static void canvas_unpremultiply_data(png_structp png, png_row_infop row_info, png_bytep data) {
	unsigned int i;

	for (i = 0; i < row_info->rowbytes; i += 4) {
		uint8_t *b = &data[i];
		uint32_t pixel;
		uint8_t  alpha;

		memcpy(&pixel, b, sizeof(uint32_t));
		alpha = (pixel & 0xff000000) >> 24;
		if (alpha == 0) {
			b[0] = b[1] = b[2] = b[3] = 0;
		}
		else {
			b[0] = (((pixel & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
			b[1] = (((pixel & 0x00ff00) >> 8) * 255 + alpha / 2) / alpha;
			b[2] = (((pixel & 0x0000ff) >> 0) * 255 + alpha / 2) / alpha;
			b[3] = alpha;
		}
	}
}


static vector<uint8_t> encode_png(uint8_t *input,uint32_t width, uint32_t height,bool alpha, uint32_t compression, uint32_t input_type) {
	vector<uint8_t> encoded;
	const auto colorType = alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	const auto rowBytes = (alpha ? 4 : 3) * width;
	// Create libpng write struct. Fail if unable to create.
	auto errorHandler = [](png_structp pngPtr, png_const_charp message) {
		Nan::ThrowError(message);
	};
	auto warningHandler = [](png_structp pngPtr, png_const_charp message) {};
	png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, errorHandler, warningHandler);

	if (!pngPtr) {
		throw "Unable to initialize libpng for writing.";
	//	Nan::ThrowError("Unable to initialize libpng for writing.");
	//	return;
	}
	// Create libpng info struct. Fail if unable to create.
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		throw "Unable to initialize libpng info struct.";
		//Nan::ThrowError("Unable to initialize libpng info struct.");
	//	return;
	}
	// libpng will jump to this if an error occured while reading.
	if (setjmp(png_jmpbuf(pngPtr))) {
		throw "Error encoding PNG.";
		//Nan::ThrowTypeError("Error encoding PNG.");
	//	return;
	}




	// This callback will be called each time libpng wants to write an encoded chunk.
	png_set_write_fn(pngPtr, &encoded, [](png_structp pngPtr, png_bytep data, png_size_t length) {
		auto encoded = reinterpret_cast<vector<uint8_t>*>(png_get_io_ptr(pngPtr));
		encoded->insert(encoded->end(), data, data + length);
	}, nullptr);
	// Use passed compression level.
	png_set_compression_level(pngPtr, compression);
	// Initialize write call with available options such as `width`, `height`, etc.
	png_set_IHDR(pngPtr, infoPtr, width, height, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	// A vector is used to address each row of the image inside the 1-dimensional `input` array.
	// Resize the vector to the amount of rows used, assigning each row to `nullptr`.
	vector<png_bytep> rows;
	rows.resize(height, nullptr);
	// Iterate over every row, and assign the pointer inside the `input` array to the element in the vector.
	// This way each element in the vector points to the beginning of the 2-dimensional row inside the 1-dimensional array.
	for (size_t row = 0; row < height; ++row) {
		rows[row] = input + row * rowBytes;
	}
	if (alpha == true && input_type == INPUT_DATA_ARGB) {
		png_set_write_user_transform_fn(pngPtr, canvas_unpremultiply_data);
	}
	// Encode the PNG.
	png_write_info(pngPtr, infoPtr);
	png_write_rows(pngPtr, &rows[0], height);
	png_write_end(pngPtr, nullptr);
	// Free libpng write struct.
	png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&pngPtr, &infoPtr);
	return encoded;
}

/* Unpremultiplies data and converts native endian ARGB => RGBA bytes */


#endif
