{
    "targets" : [
        {
            "target_name" : "node-libpng",
            "dependencies" : [
                "./deps/libpng.gyp:libpng"
            ],
            
            "cflags_cc!": [ '-fno-exceptions' ],
            "cflags" : [
                "-Wall",
                "-Wno-unused-parameter",
                "-Wno-missing-field-initializers",
                "-Wextra",
                "-fexceptions"

            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "./deps/libpng",
                "./deps/zlib"
            ],
            "sources" : [
				
                "./native/module.cpp",
                "./native/encode.cpp",
                "./native/is-png.cpp",
                "./native/png-image.cpp",
                "./native/resize.cpp",
                "./native/copy.cpp",
                "./native/fill.cpp"
            ]
        }
    ]
}

