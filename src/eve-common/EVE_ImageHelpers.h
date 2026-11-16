#ifndef EVEMU_IMAGE_HELPERS_H
#define EVEMU_IMAGE_HELPERS_H

// A lightweight check that our legacy Character/*.jpg cache does not send
// PNG data or an empty file with Content-Type: image/jpeg. This only checks
// the JPEG Start-of-Image marker, not full image integrity.
namespace EvEImage {

template <typename ByteContainer>
inline bool HasJpegSignature(const ByteContainer& bytes) {
    return bytes.size() >= 4
        && static_cast<unsigned char>(bytes[0]) == 0xFF
        && static_cast<unsigned char>(bytes[1]) == 0xD8
        && static_cast<unsigned char>(bytes[2]) == 0xFF;
}

} // namespace EvEImage

#endif
