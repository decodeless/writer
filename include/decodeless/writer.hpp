// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <decodeless/allocator.hpp>
#include <decodeless/allocator_construction.hpp>
#include <decodeless/mappedfile.hpp>
#include <filesystem>

namespace decodeless {

namespace fs = std::filesystem;

template <resizable_mapped_memory ResizableMemory>
class growable_memory_resource : public ResizableMemory {
public:
    using ResizableMemory::ResizableMemory;

    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t align) {
        if (ResizableMemory::size() != 0)
            throw std::bad_alloc();
        ResizableMemory::resize(bytes);
        // m_mappedFile mapping alignment should easily be big enough
        if ((reinterpret_cast<uintptr_t>(ResizableMemory::data()) & (align - 1)) != 0)
            throw std::bad_alloc();
        return ResizableMemory::data();
    }

    [[nodiscard]] void* reallocate(void* ptr, std::size_t bytes,
                                   [[maybe_unused]] std::size_t align) {
        if (ptr != ResizableMemory::data())
            throw std::bad_alloc();
        ResizableMemory::resize(bytes);
        return ResizableMemory::data();
    }

    constexpr void deallocate(void* p, std::size_t bytes) {
        // Do nothing
        (void)p;
        (void)bytes;
    }

    size_t max_size() const { return ResizableMemory::capacity(); }
    void   resize(size_t size) { ResizableMemory::resize(size); }
};

// Note: truncates the file/memory on construction!
class mapped_file_memory_resource : public growable_memory_resource<resizable_file> {
public:
    mapped_file_memory_resource(const fs::path& path, size_t maxSize)
        : growable_memory_resource<resizable_file>(path, maxSize) {
        // Truncate the file if it already exists
        if (this->size() > 0)
            this->resize(0);
    }
};

static_assert(realloc_memory_resource<mapped_file_memory_resource>);
static_assert(has_max_size<mapped_file_memory_resource>);

class mapped_memory_memory_resource : public growable_memory_resource<resizable_memory> {
public:
    mapped_memory_memory_resource(size_t maxSize)
        : growable_memory_resource<resizable_memory>(0, maxSize) {}
};

static_assert(realloc_memory_resource<mapped_memory_memory_resource>);
static_assert(has_max_size<mapped_memory_memory_resource>);

template <trivially_destructible T>
struct mapped_file_allocator : public memory_resource_ref<T, mapped_file_memory_resource> {
    using memory_resource_ref<T, mapped_file_memory_resource>::memory_resource_ref;
    [[nodiscard]] constexpr T* reallocate(T* ptr, std::size_t bytes) {
        return static_cast<T*>(this->m_resource->reallocate(ptr, bytes));
    }
    constexpr size_t max_size() const { return this->m_resource->max_size(); }
};

static_assert(realloc_allocator<mapped_file_allocator<std::byte>>);
static_assert(has_max_size<mapped_file_allocator<std::byte>>);

template <trivially_destructible T>
struct mapped_memory_allocator : public memory_resource_ref<T, mapped_memory_memory_resource> {
    using memory_resource_ref<T, mapped_memory_memory_resource>::memory_resource_ref;
    [[nodiscard]] constexpr T* reallocate(T* ptr, std::size_t bytes) {
        return static_cast<T*>(this->m_resource->reallocate(ptr, bytes));
    }
    constexpr size_t max_size() const { return this->m_resource->max_size(); }
};

static_assert(realloc_allocator<mapped_memory_allocator<std::byte>>);
static_assert(has_max_size<mapped_memory_allocator<std::byte>>);

// Wrapper around linear_memory_resource to truncate
template <memory_resource_or_allocator ParentAllocator>
class truncating_linear_memory_resource : public linear_memory_resource<ParentAllocator> {
public:
    using linear_memory_resource<ParentAllocator>::linear_memory_resource;
    ~truncating_linear_memory_resource() {
        // Truncate down to what was allocated
        if (m_truncateBackingOnDestruct)
            this->truncate();
    }
    truncating_linear_memory_resource(truncating_linear_memory_resource&& other) noexcept
        : linear_memory_resource<ParentAllocator>(std::move(other)) {
        other.m_truncateBackingOnDestruct = false;
    }
    truncating_linear_memory_resource&
    operator=(truncating_linear_memory_resource&& other) noexcept {
        other.m_truncateBackingOnDestruct = false;
        return linear_memory_resource<ParentAllocator>::operator=(std::move(other));
    }

private:
    // Avoid resizing invalid objects after std::move()
    bool m_truncateBackingOnDestruct = true;
};

class file_writer {
public:
    using memory_resource_type = truncating_linear_memory_resource<mapped_file_memory_resource>;
    static constexpr size_t INITIAL_SIZE = memory_resource_type::INITIAL_SIZE;
    file_writer(const fs::path& path, size_t maxSize, size_t initialSize = INITIAL_SIZE)
        : m_linearResource(initialSize, mapped_file_memory_resource(path, maxSize)) {}
    memory_resource_type& resource() { return m_linearResource; }
    void*                 data() const { return m_linearResource.data(); }
    size_t                size() const { return m_linearResource.size(); }

    template <class T, class... Args>
    T* create(Args&&... args) {
        return decodeless::create::object<T>(resource(), std::forward<Args>(args)...);
    }

    template <class T>
    std::span<T> createArray(size_t size) {
        return decodeless::create::array<T>(resource(), size);
    }

#ifdef __cpp_lib_ranges
    template <class T, std::ranges::input_range Range>
    std::span<T> createArray(Range&& range) {
        return decodeless::create::array<T>(resource(), std::forward<Range>(range));
    }

    template <std::ranges::input_range Range>
    std::span<std::ranges::range_value_t<Range>> createArray(Range&& range) {
        return decodeless::create::array(resource(), std::forward<Range>(range));
    }
#endif

private:
    memory_resource_type m_linearResource;
};

class memory_writer {
public:
    using memory_resource_type = truncating_linear_memory_resource<mapped_memory_memory_resource>;
    static constexpr size_t INITIAL_SIZE = memory_resource_type::INITIAL_SIZE;
    memory_writer(size_t maxSize, size_t initialSize = INITIAL_SIZE)
        : m_linearResource(initialSize, mapped_memory_memory_resource(maxSize)) {}
    memory_resource_type& resource() { return m_linearResource; }
    void*                 data() const { return m_linearResource.data(); }
    size_t                size() const { return m_linearResource.size(); }

    template <class T, class... Args>
    T* create(Args&&... args) {
        return decodeless::create::object<T>(resource(), std::forward<Args>(args)...);
    }

    template <class T>
    std::span<T> createArray(size_t size) {
        return decodeless::create::array<T>(resource(), size);
    }

#ifdef __cpp_lib_ranges
    template <class T, std::ranges::input_range Range>
    std::span<T> createArray(Range&& range) {
        return decodeless::create::array<T>(resource(), std::forward<Range>(range));
    }

    template <std::ranges::input_range Range>
    std::span<std::ranges::range_value_t<Range>> createArray(const Range& range) {
        return decodeless::create::array(resource(), std::forward<Range>(range));
    }
#endif

private:
    memory_resource_type m_linearResource;
};

} // namespace decodeless
