#ifndef WJR_MEMORY_MEMORY_POOL_HPP__
#define WJR_MEMORY_MEMORY_POOL_HPP__

#include <wjr/crtp/trivially_allocator_base.hpp>
#include <wjr/memory/details.hpp>

namespace wjr {

template <int __inst>
class __malloc_alloc_template__ {
private:
    static void *_S_oom_malloc(size_t);
    static void *_S_oom_realloc(void *, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
    static void (*__malloc_alloc_oom_handler)();
#endif

public:
    WJR_NOINLINE static void *allocate(size_t __n) {
        void *__result = malloc(__n);
        if (WJR_LIKELY(0 != __result))
            return __result;
        return _S_oom_malloc(__n);
    }

    static void deallocate(void *__p, size_t /* __n */) { free(__p); }

    static void *reallocate(void *__p, size_t /* old_sz */, size_t __new_sz) {
        void *__result = realloc(__p, __new_sz);
        if (WJR_LIKELY(0 != __result))
            return __result;
        return _S_oom_realloc(__p, __new_sz);
    }

    static void (*__set_malloc_handler(void (*__f)()))() {
        void (*__old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = __f;
        return (__old);
    }
};

// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (*__malloc_alloc_template__<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int __inst>
void *__malloc_alloc_template__<__inst>::_S_oom_malloc(size_t __n) {
    void (*__my_malloc_handler)();
    void *__result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if (__result)
            return (__result);
    }
}

template <int __inst>
void *__malloc_alloc_template__<__inst>::_S_oom_realloc(void *__p, size_t __n) {
    void (*__my_malloc_handler)();
    void *__result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;

        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result)
            return (__result);
    }
}

namespace memory_pool_details {

static constexpr uint8_t __small_index_table[128] = {
    0,  1,  2,  3,  4,  4,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

static constexpr uint16_t __size_table[11 + 8] = {
    8,    16,   24,   32,   48,   64,    96,    128,   256,   512,
    1024, 2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384,
};

} // namespace memory_pool_details

template <int inst>
class __default_alloc_template__ {
private:
    using allocator_type = __malloc_alloc_template__<0>;

    union obj {
        union obj *free_list_link;
        char client_data[1];
    };

    struct object {
        obj *volatile free_list[19] = {nullptr};
        char *start_free = nullptr;
        char *end_free = nullptr;
        size_t heap_size = 0;
    };

    static object &get_instance() noexcept {
        static thread_local object instance;
        return instance;
    }

    static obj *volatile *get_free_list() noexcept { return get_instance().free_list; }
    static char *&get_start_free() noexcept { return get_instance().start_free; }
    static char *&get_end_free() noexcept { return get_instance().end_free; }
    static size_t &get_heap_size() noexcept { return get_instance().heap_size; }

    static inline size_t __round_up(size_t bytes) {
        return (((bytes) + 2048 - 1) & ~(2048 - 1));
    }

    static WJR_INTRINSIC_INLINE uint8_t __get_index(size_t bytes) {
        if (bytes <= 1024) {
            return memory_pool_details::__small_index_table[(bytes - 1) >> 3];
        }

        return 11 + (bytes - 1) / 2048;
    }

    static inline uint16_t __get_size(uint8_t idx) {
        return memory_pool_details::__size_table[idx];
    }

    // Returns an object of size n, and optionally adds to size n free list.
    WJR_NOINLINE static void *refill(uint8_t idx) noexcept;

    // Allocates a chunk for nobjs of size "size".  nobjs may be reduced
    // if it is inconvenient to allocate the requested number.
    static char *chunk_alloc(uint8_t idx, int &nobjs) noexcept;

public:
    // n must be > 0
    WJR_INTRINSIC_INLINE static allocation_result<void *> allocate(size_t n) noexcept {
        if (n > (size_t)16384) {
            return {allocator_type::allocate(n), n};
        }

        const size_t idx = __get_index(n);
        const size_t size = __get_size(idx);
        obj *volatile *my_free_list = get_free_list() + idx;
        obj *result = *my_free_list;
        if (WJR_LIKELY(result != nullptr)) {
            *my_free_list = result->free_list_link;
            return {result, size};
        }
        return {refill(idx), size};
    }

    // p must not be 0
    WJR_INTRINSIC_INLINE static void deallocate(void *p, size_t n) noexcept {
        if (n > (size_t)16384) {
            allocator_type::deallocate(p, n);
            return;
        }

        obj *q = (obj *)p;
        obj *volatile *my_free_list = get_free_list() + __get_index(n);
        q->free_list_link = *my_free_list;
        *my_free_list = q;
    }
};

//----------------------------------------------
// We allocate memory in large chunks in order to
// avoid fragmentingthe malloc heap too much.
// We assume that size is properly aligned.
// We hold the allocation lock.
//----------------------------------------------
template <int inst>
char *__default_alloc_template__<inst>::chunk_alloc(uint8_t idx, int &nobjs) noexcept {
    const size_t size = __get_size(idx);
    char *result;
    size_t total_bytes = size * nobjs;
    auto bytes_left = static_cast<size_t>(get_end_free() - get_start_free());

    if (bytes_left >= total_bytes) {
        result = get_start_free();
        get_start_free() += total_bytes;
        return (result);
    }

    if (bytes_left >= size) {
        nobjs = static_cast<int>(bytes_left / size);
        total_bytes = size * nobjs;
        result = get_start_free();
        get_start_free() += total_bytes;
        return (result);
    }

    // Try to make use of the left-over piece.
    if (bytes_left > 0) {
        WJR_ASSERT(!(bytes_left & 7));

        char *start_free = get_start_free();
        uint8_t __idx = __get_index(bytes_left);
        for (;; --__idx) {
            const auto __size = __get_size(__idx);
            if (bytes_left >= __size) {
                obj *volatile *my_free_list = get_free_list() + __idx;
                ((obj *)start_free)->free_list_link = *my_free_list;
                *my_free_list = (obj *)start_free;

                if (bytes_left == __size) {
                    break;
                }

                start_free += __size;
                bytes_left -= __size;
            }
        }
    }

    do {
        obj *volatile *my_free_list;
        if (idx < 11) {
            for (int i = 18; i > 11; --i) {
                my_free_list = get_free_list() + i;
                obj *p = *my_free_list;
                // split the chunk
                if (p != nullptr) {
                    *my_free_list = p->free_list_link;
                    get_start_free() = (char *)(p);
                    char *__e = (char *)(p) + 2048;
                    get_end_free() = __e;
                    obj *e = (obj *)(__e);
                    --my_free_list;
                    e->free_list_link = *my_free_list;
                    *my_free_list = e;
                    return (chunk_alloc(idx, nobjs));
                }
            }
        }
    } while (0);

    const size_t bytes_to_get = 2 * total_bytes + __round_up(get_heap_size() >> 4);
    get_start_free() = (char *)malloc(bytes_to_get);

    WJR_ASSERT(get_start_free() != nullptr);

    get_heap_size() += bytes_to_get;
    get_end_free() = get_start_free() + bytes_to_get;
    return (chunk_alloc(idx, nobjs));
}

//----------------------------------------------
// Returns an object of size n, and optionally adds
// to size n free list.We assume that n is properly aligned.
// We hold the allocation lock.
//----------------------------------------------
template <int inst>
void *__default_alloc_template__<inst>::refill(uint8_t idx) noexcept {
    int nobjs = idx < 6 ? 32 : idx < 8 ? 16 : idx < 11 ? 8 : 4;
    char *chunk = chunk_alloc(idx, nobjs);
    obj *current_obj;
    obj *next_obj;

    if (1 == nobjs) {
        return (chunk);
    }

    obj *volatile *my_free_list = get_free_list() + idx;

    const size_t n = __get_size(idx);

    // Build free list in chunk
    obj *result = (obj *)chunk;

    *my_free_list = current_obj = (obj *)(chunk + n);
    nobjs -= 2;
    while (nobjs) {
        --nobjs;
        next_obj = (obj *)((char *)current_obj + n);
        current_obj->free_list_link = next_obj;
        current_obj = next_obj;
    }
    current_obj->free_list_link = 0;
    return (result);
}

template <typename Ty>
class memory_pool {
private:
    using allocator_type = __default_alloc_template__<0>;

public:
    using value_type = Ty;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;
    using is_trivially_allocator = std::true_type;

    template <typename Other>
    struct rebind {
        using other = memory_pool<Other>;
    };

    constexpr memory_pool() noexcept = default;
    constexpr memory_pool(const memory_pool &) noexcept = default;
    template <typename Other>
    constexpr memory_pool(const memory_pool<Other> &) noexcept {}
    ~memory_pool() = default;
    memory_pool &operator=(const memory_pool &) noexcept = default;

    WJR_NODISCARD WJR_CONSTEXPR20 allocation_result<Ty *>
    allocate_at_least(size_type n) const noexcept {
        if (WJR_UNLIKELY(0 == n)) {
            return {nullptr, 0};
        }

        const auto ret = allocator_type::allocate(n * sizeof(Ty));
        return {static_cast<Ty *>(ret.ptr), ret.count};
    }

    WJR_NODISCARD WJR_CONSTEXPR20 WJR_MALLOC Ty *allocate(size_type n) const noexcept {
        return allocate_at_least(n).ptr;
    }

    WJR_CONSTEXPR20 void deallocate(Ty *ptr, size_type n) const noexcept {
        if (WJR_UNLIKELY(0 == n)) {
            return;
        }

        return allocator_type::deallocate(static_cast<void *>(ptr), sizeof(Ty) * n);
    }

    constexpr size_t max_size() const { return static_cast<size_t>(-1) / sizeof(Ty); }
};

template <typename T, typename U>
constexpr bool operator==(const memory_pool<T> &, const memory_pool<U> &) {
    return true;
}

template <typename T, typename U>
constexpr bool operator!=(const memory_pool<T> &, const memory_pool<U> &) {
    return false;
}

} // namespace wjr

#endif // WJR_MEMORY_MEMORY_POOL_HPP__