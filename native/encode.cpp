#include <png.h>
#include <zlib.h>
#include <node_buffer.h>
#include <iostream>
#include "encode.hpp"

#include "is-png.hpp"

using namespace node;
using namespace v8;
using namespace std;









NAN_METHOD(encode) {
    // 1st Parameter: The input buffer to encode.
    Local<Object> inputBuffer = Local<Object>::Cast(info[0]);
    uint8_t *input = reinterpret_cast<uint8_t*>(Buffer::Data(inputBuffer));
    // 2nd Parameter: The width of the image to encode.
    const auto width = static_cast<uint32_t>(Nan::To<uint32_t>(info[1]).ToChecked());
    // 3rd Parameter: The height of the image to encode.
    const auto height = static_cast<uint32_t>(Nan::To<uint32_t>(info[2]).ToChecked());
    // 4th Parameter: Whether to use alpha channel or not.
    const auto alpha = static_cast<bool>(Nan::To<bool>(info[3]).ToChecked());
    // 5th Parameter: Compression level, default to best compression
    const auto compression = static_cast<uint32_t>(Nan::To<uint32_t>(info[4]).FromMaybe(Z_BEST_COMPRESSION));

	const auto input_type = static_cast<uint32_t>(Nan::To<uint32_t>(info[5]).FromMaybe(INPUT_DATA_RGBA));
	vector<uint8_t> encoded;
    // calculate derived parameters.
	try {
		encoded = encode_png(input, width, height, alpha, compression, input_type);
	}
	catch (const char* msg) {
		Nan::ThrowTypeError(msg);
	}
    // Return created encoded image as a buffer. Needs to be a copy as the vector from above will be freed.
    info.GetReturnValue().Set(Nan::CopyBuffer(reinterpret_cast<char*>(&encoded[0]), encoded.size()).ToLocalChecked());
}



NAN_MODULE_INIT(InitEncode) {
    Nan::Set(target, Nan::New("__native_encode").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(encode)).ToLocalChecked());
}
