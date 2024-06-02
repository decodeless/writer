// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <decodeless/pmr_allocator.hpp>
#include <decodeless/writer.hpp>

namespace decodeless {

// DANGER: Allocated objects must all be trivially copyable/destructible. Due to
// the std::pmr::memory_resource interface, there is no validation here to
// support that restriction.

class pmr_file_writer {
public:
    using memory_resource_type =
        memory_resource_adapter<truncating_linear_memory_resource<mapped_file_memory_resource>>;
    static constexpr size_t INITIAL_SIZE =
        truncating_linear_memory_resource<mapped_file_memory_resource>::INITIAL_SIZE;
    pmr_file_writer(const fs::path& path, size_t maxSize, size_t initialSize = INITIAL_SIZE)
        : m_linearResource(initialSize, mapped_file_memory_resource(path, maxSize)) {}
    memory_resource_type& resource() { return m_linearResource; }
    void*                 data() const { return m_linearResource.backing_resource().data(); }
    size_t                size() const { return m_linearResource.backing_resource().size(); }

    template <class T, class... Args>
    T* create(Args&&... args) {
        return decodeless::create::object<T>(m_linearResource.backing_resource(),
                                             std::forward<Args>(args)...);
    }

    template <class T>
    std::span<T> createArray(size_t size) {
        return decodeless::create::array<T>(m_linearResource.backing_resource(), size);
    }

#ifdef __cpp_lib_ranges
    template <class T, std::ranges::input_range Range>
    std::span<T> createArray(Range&& range) {
        return decodeless::create::array<T>(m_linearResource.backing_resource(),
                                            std::forward<Range>(range));
    }

    template <std::ranges::input_range Range>
    std::span<std::ranges::range_value_t<Range>> createArray(Range&& range) {
        return decodeless::create::array(m_linearResource.backing_resource(),
                                         std::forward<Range>(range));
    }
#endif

private:
    memory_resource_type m_linearResource;
};

class pmr_memory_writer {
public:
    using memory_resource_type =
        memory_resource_adapter<truncating_linear_memory_resource<mapped_memory_memory_resource>>;
    static constexpr size_t INITIAL_SIZE =
        truncating_linear_memory_resource<mapped_memory_memory_resource>::INITIAL_SIZE;
    pmr_memory_writer(size_t maxSize, size_t initialSize = INITIAL_SIZE)
        : m_linearResource(initialSize, mapped_memory_memory_resource(maxSize)) {}
    memory_resource_type& resource() { return m_linearResource; }
    void*                 data() const { return m_linearResource.backing_resource().data(); }
    size_t                size() const { return m_linearResource.backing_resource().size(); }

    template <class T, class... Args>
    T* create(Args&&... args) {
        return decodeless::create::object<T>(m_linearResource.backing_resource(),
                                             std::forward<Args>(args)...);
    }

    template <class T>
    std::span<T> createArray(size_t size) {
        return decodeless::create::array<T>(m_linearResource.backing_resource(), size);
    }

#ifdef __cpp_lib_ranges
    template <class T, std::ranges::input_range Range>
    std::span<T> createArray(Range&& range) {
        return decodeless::create::array<T>(m_linearResource.backing_resource(),
                                            std::forward<Range>(range));
    }

    template <std::ranges::input_range Range>
    std::span<std::ranges::range_value_t<Range>> createArray(Range&& range) {
        return decodeless::create::array(m_linearResource.backing_resource(),
                                         std::forward<Range>(range));
    }
#endif

private:
    memory_resource_type m_linearResource;
};

} // namespace decodeless
