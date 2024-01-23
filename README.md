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
    offset_span<char> hello;
    offset_ptr<int>   data;
};

size_t maxSize = 4096;  // or a terabyte :) - just reserves address space
decodeless::Writer writer("myfile.dat", maxSize);

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

## Contributing

Issues and pull requests are most welcome, thank you! Note the
[DCO](CONTRIBUTING) and MIT [LICENSE](LICENSE).
