#pragma once

template <typename T>
inline T* attributeAt(char* buffer, int byteStride, int index) {
	return reinterpret_cast<T*>(buffer + static_cast<size_t>(index) * static_cast<size_t>(byteStride));
}

template <typename T>
inline const T* attributeAt(const char* buffer, int byteStride, int index) {
	return reinterpret_cast<const T*>(buffer + static_cast<size_t>(index) * static_cast<size_t>(byteStride));
}

#define MFX_CHECK(call) \
  { \
    OfxStatus status = call; \
    assert(kOfxStatOK == status); \
    if (kOfxStatOK != status) \
      printf("ERROR: Mfx suite call '" #call "' failed with status %d!\n", status); \
  }
