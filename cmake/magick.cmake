if (NOT MAGICK_ROOT)
    find_package(ImageMagick COMPONENTS MagickCore Magick++ REQUIRED)
    return()
endif()

add_library(ImageMagickCore INTERFACE)
add_library(ImageMagick::MagickCore ALIAS ImageMagickCore)

file(GLOB MAGICK_LIBRARIES "${MAGICK_ROOT}/Artifacts/lib/*.lib")

target_link_libraries(ImageMagickCore INTERFACE ${MAGICK_LIBRARIES})
target_include_directories(ImageMagickCore INTERFACE "${MAGICK_ROOT}/ImageMagick")

add_library(ImageMagick++ INTERFACE)
add_library(ImageMagick::Magick++ ALIAS ImageMagick++)

target_include_directories(ImageMagick++ INTERFACE "${MAGICK_ROOT}/ImageMagick/Magick++/lib")
