#pragma once

template <typename T>
T* attributeAt(char* buffer, int byteStride, int index) {
	return reinterpret_cast<T*>(buffer + static_cast<size_t>(index) * static_cast<size_t>(byteStride));
}

#define MFX_CHECK(call) \
  { \
    OfxStatus status = call; \
    assert(kOfxStatOK == status); \
    if (kOfxStatOK != status) \
      printf("ERROR: Mfx suite call '" #call "' failed with status %d!\n", status); \
  }
