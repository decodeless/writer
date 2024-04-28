# decodeless_writer

[`decodeless`](https://github.com/decodeless) (previously no-decode) is a
collection of utility libraries for conveniently reading and writing files via
memory mapping. Components can be used individually or combined.

`decodeless_writer` combines
[`decodeless_allocator`](https://github.com/decodeless/allocator) with
[`decodeless_mappedfile`](https://github.com/decodeless/mappedfile) to provide a
cross platform class `decodeless::Writer` to easily creting a binary file using
memory mapping. The file automatically grows in size (up to a user provided
maximum, limited only by the virtual address space). The allocator provides
alignment so the file can be read directly after memory mapping.

**Example:**

This example uses relative pointers [`offset_ptr` and
`offset_span`](https://github.com/decodeless/offset_ptr) to reference other
structures in the file.

```
struct Header {
    decodeless::offset_span<char> hello;
    decodeless::offset_ptr<int>   data;
};

size_t maxSize = 4096;  // or a terabyte :) - just reserves address space
decodeless::file_writer writer("myfile.dat", maxSize);

// Add the above header to the file
TestHeader* header = writer.create<TestHeader>();

// Create some data for header members to point to
header->hello = writer.createArray("Hello World!");
header->data = writer.create<int>(42);

// Notice the size is a little bigger than expected to account for the 4-byte
// alignment requirements of 'int'.
EXPECT_EQ(writer.size(), 44);

...

// Open the file for reading (see decodeless_mappedfile)
decodeless::file reader("myfile.dat");
const Header* header = reinterpret_cast<const Header*>(reader.data());
EXPECT_EQ(*header->data, 42);
```

If a temporary in-memory file is needed, `decodeless::memory_writer` is provided
with the same interface as `decodeless::file_writer`. Note that the allocator is
necessarily a different type since it is backed by
`decodeless::resizable_memory` instead of `decodeless::resizable_file`, which
means the same function cannot write to both a `file_writer` or a
`memory_writer` without templating them. This is exactly why
[`std::pmr::polymorphic_allocator`](https://en.cppreference.com/w/cpp/memory/polymorphic_allocator)
was created. Thus, this library also provides `decodeless::pmr_file_writer` and
`decodeless::pmr_memory_writer`, which both provide a common
`std::pmr::memory_resource` compatible `resource()`.

## Contributing

Issues and pull requests are most welcome, thank you! Note the
[DCO](CONTRIBUTING) and MIT [LICENSE](LICENSE).
