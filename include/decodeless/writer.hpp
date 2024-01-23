// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <decodeless/allocator.hpp>
#include <decodeless/mappedfile.hpp>
#include <filesystem>

namespace decodeless {

namespace fs = std::filesystem;

// Note: truncates the file on construction!
class mapped_file_memory_resource {
public:
    mapped_file_memory_resource(const fs::path& path, size_t maxSize)
        : m_mappedFile(path, maxSize) {
        // Truncate the file if it already exists
        if (m_mappedFile.size() > 0)
            m_mappedFile.resize(0);
    }

    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t align) {
        if (m_mappedFile.size() != 0)
            throw std::bad_alloc();
        m_mappedFile.resize(bytes);
        // m_mappedFile mapping alignment should easily be big enough
        if ((reinterpret_cast<uintptr_t>(m_mappedFile.data()) & (align - 1)) != 0)
            throw std::bad_alloc();
        return m_mappedFile.data();
    }

    [[nodiscard]] void* reallocate(void* ptr, std::size_t bytes) {
        if (ptr != m_mappedFile.data())
            throw std::bad_alloc();
        m_mappedFile.resize(bytes);
        return m_mappedFile.data();
    }

    constexpr void deallocate(void* p, std::size_t bytes) {
        // Do nothing
        (void)p;
        (void)bytes;
    }

    size_t max_size() const { return m_mappedFile.capacity(); }
    void   resize(size_t size) { m_mappedFile.resize(size); }

private:
    resizable_file m_mappedFile;
};

template <TriviallyDestructible T>
struct mapped_file_allocator : public aligned_allocator_ref<T, mapped_file_memory_resource> {
    using aligned_allocator_ref<T, mapped_file_memory_resource>::aligned_allocator_ref;
    [[nodiscard]] constexpr T* reallocate(T* ptr, std::size_t bytes) {
        return static_cast<T*>(this->m_resource.reallocate(ptr, bytes));
    }
    constexpr size_t max_size() const { return this->m_resource.max_size(); }
};

static_assert(CanReallocate<mapped_file_allocator<std::byte>>);
static_assert(HasMaxSize<mapped_file_allocator<std::byte>>);

class Writer {
public:
    static constexpr size_t INITIAL_SIZE =
        linear_memory_resource<mapped_file_allocator<std::byte>>::INITIAL_SIZE;
    Writer(const fs::path& path, size_t maxSize, size_t initialSize = INITIAL_SIZE)
        : m_fileResource(path, maxSize)
        , m_linearResource(initialSize, m_fileResource) {}
    ~Writer() {
        // Truncate down to what was allocated
        m_fileResource.resize(m_linearResource.bytesAllocated());
    }

    // TODO: not impossible. depends on linear_memory_resource
    Writer& operator=(Writer&& other) = delete;

    template <class T, class... Args>
    T* create(Args&&... args) {
        return decodeless::create<T>(resource(), std::forward<Args>(args)...);
    }

    template <class T>
    std::span<T> createArray(size_t size) {
        return decodeless::createArray<T>(resource(), size);
    }

#ifdef __cpp_lib_ranges
    template <std::ranges::input_range Range>
    std::span<std::ranges::range_value_t<Range>> createArray(const Range& range) {
        return decodeless::createArray(resource(), range);
    }
#endif

    linear_memory_resource<mapped_file_allocator<std::byte>>& resource() {
        return m_linearResource;
    }
    size_t size() const { return m_linearResource.bytesAllocated(); }

private:
    // Backing allocator
    mapped_file_memory_resource m_fileResource;

    // Linear allocator, allocating blocks within m_file_resource
    linear_memory_resource<mapped_file_allocator<std::byte>> m_linearResource;
};

} // namespace decodeless
