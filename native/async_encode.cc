
#include <png.h>
#include <zlib.h>
#include <node_buffer.h>
#include <iostream>
#include "encode.hpp"

#include <nan.h>

using namespace node;
using namespace v8;
using namespace std;

class EncodeWorker : public Nan::AsyncWorker {
public:
	EncodeWorker(
		         Nan::Callback *callback,
			     uint8_t *input,
		         uint32_t width,
				 uint32_t height,
				 bool alpha,
			     uint32_t compression,
		         uint32_t input_type)
		: Nan::AsyncWorker(callback), 
		        async_lock(new uv_mutex_t),
		        input(input), 
		        width(width),
			    height(height),
		        alpha(alpha),
				compression(compression),
				input_type(input_type)
	{}

	~EncodeWorker() {
	
		
	}

	void Execute() {

		try {
			encoded = encode_png(input, width, height, alpha, compression, input_type);
		}
		catch (const char* msg) {
			Nan::ThrowTypeError(msg);
		}

	//	uv_mutex_lock(async_lock);
		/*const auto colorType = alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
		const auto rowBytes = (alpha ? 4 : 3) * width;
		// Create libpng write struct. Fail if unable to create.
		auto errorHandler = [](png_structp pngPtr, png_const_charp message) {
			Nan::ThrowError(message);
		};
		auto warningHandler = [](png_structp pngPtr, png_const_charp message) {};
		png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, errorHandler, warningHandler);
		if (!pngPtr) {
			Nan::ThrowError("Unable to initialize libpng for writing.");
			return;
		}
		// Create libpng info struct. Fail if unable to create.
		png_infop infoPtr = png_create_info_struct(pngPtr);
		if (!infoPtr) {
			Nan::ThrowError("Unable to initialize libpng info struct.");
			return;
		}
		// libpng will jump to this if an error occured while reading.
		if (setjmp(png_jmpbuf(pngPtr))) {
			Nan::ThrowTypeError("Error encoding PNG.");
			return;
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
		// Encode the PNG.
		png_write_info(pngPtr, infoPtr);
		png_write_rows(pngPtr, &rows[0], height);
		png_write_end(pngPtr, nullptr);
		// Free libpng write struct.
		png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&pngPtr, &infoPtr);
	//	uv_mutex_unlock(async_lock);*/

	}

	void HandleOKCallback() {
		Nan::HandleScope scope;

		// info.GetReturnValue().Set(Nan::CopyBuffer(reinterpret_cast<char*>(&encoded[0]), encoded.size()).ToLocalChecked());


		v8::Local<v8::Object> res = Nan::CopyBuffer(reinterpret_cast<char*>(&encoded[0]), encoded.size()).ToLocalChecked();
	

		v8::Local<v8::Value> argv[] = {
			res
		};

		callback->Call(1, argv, async_resource);
	}

private:
	uv_mutex_t *async_lock;
	uint8_t *input;
	uint32_t width;
	uint32_t height;
	bool alpha;
	uint32_t compression;
	uint32_t input_type;
	vector<uint8_t> encoded;
};


NAN_METHOD(encodeAsync) {
	Local<Object> inputBuffer = Local<Object>::Cast(info[0]);
	uint8_t *input = reinterpret_cast<uint8_t*>(Buffer::Data(inputBuffer));
	// 2nd Parameter: The width of the image to encode.
	uint32_t width = static_cast<uint32_t>(Nan::To<uint32_t>(info[1]).ToChecked());
	// 3rd Parameter: The height of the image to encode.
	uint32_t height = static_cast<uint32_t>(Nan::To<uint32_t>(info[2]).ToChecked());
	// 4th Parameter: Whether to use alpha channel or not.
	bool alpha = static_cast<bool>(Nan::To<bool>(info[3]).ToChecked());
	// 5th Parameter: Compression level, default to best compression
	uint32_t compression = static_cast<uint32_t>(Nan::To<uint32_t>(info[4]).FromMaybe(Z_BEST_COMPRESSION));

	uint32_t input_type = static_cast<uint32_t>(Nan::To<uint32_t>(info[5]).FromMaybe(INPUT_DATA_RGBA));

	Nan::Callback* callback = new Nan::Callback(
		v8::Local<v8::Function>::Cast(info[6])
	);

	EncodeWorker* worker = new EncodeWorker(
		callback,
		input,
		width,
		height,
		alpha,
		compression,
		input_type
	);

	Nan::AsyncQueueWorker(worker);

	return;
}

NAN_MODULE_INIT(InitAsyncEncode) {
    Nan::Set(target, Nan::New("__native_encode_async").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(encodeAsync)).ToLocalChecked());
}



