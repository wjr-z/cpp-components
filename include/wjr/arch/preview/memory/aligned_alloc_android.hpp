#ifndef WJR_ARCH_PREVIEW_MEMORY_ALIGNED_ALLOC_ANDROID_HPP__
#define WJR_ARCH_PREVIEW_MEMORY_ALIGNED_ALLOC_ANDROID_HPP__

#include <malloc.h>

#include <wjr/memory/align.hpp>

namespace wjr {

WJR_MALLOC inline void *aligned_alloc(size_t size, size_t alignment) noexcept {
    WJR_ASSERT(has_single_bit(alignment));
    return ::memalign(alignment, size);
}

inline void aligned_free(void *ptr) noexcept { ::free(ptr); }

} // namespace wjr

#endif // WJR_ARCH_PREVIEW_MEMORY_ALIGNED_ALLOC_ANDROID_HPP__