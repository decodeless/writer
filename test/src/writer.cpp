// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <decodeless/offset_ptr.hpp>
#include <decodeless/offset_span.hpp>
#include <decodeless/pmr_writer.hpp>
#include <decodeless/writer.hpp>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ostream>

using namespace decodeless;

TEST(Writer, CreateEmpty) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    EXPECT_FALSE(std::filesystem::exists(tmpFile));
    { file_writer writer(tmpFile, 4096); }
    EXPECT_EQ(std::filesystem::file_size(tmpFile), 0);
    fs::remove(tmpFile);
}

// Perhaps controversial, but a "writer" is generally going to create files from
// scratch and should clear existing files by default to rewrite them. In
// contrast, the decodeless::resizable_file will keep the initial contents.
// Without this, re-running a write operation could silently append to the file
// instead, which IMO would be more surprising and bug prone.
TEST(Writer, ClearExisting) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    { std::ofstream(tmpFile, std::ios::binary).write("abcd", 4); }
    EXPECT_TRUE(std::filesystem::exists(tmpFile));
    EXPECT_EQ(std::filesystem::file_size(tmpFile), 4);
    { file_writer writer(tmpFile, 4096); }
    EXPECT_EQ(std::filesystem::file_size(tmpFile), 0);
    fs::remove(tmpFile);
}

TEST(Writer, CreateFromEmpty) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    EXPECT_FALSE(std::filesystem::exists(tmpFile));
    {
        file_writer writer(tmpFile, 4096);
        writer.create<int>(42);
    }
    EXPECT_EQ(std::filesystem::file_size(tmpFile), sizeof(int));
    fs::remove(tmpFile);
}

TEST(Writer, Simple) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    {
        file_writer writer(tmpFile, 4096, 4);
        writer.create<int>(42);
    }
    {
        file mapped(tmpFile);
        EXPECT_EQ(*reinterpret_cast<const int*>(mapped.data()), 42);
    }
    fs::remove(tmpFile);
}

TEST(Writer, Realloc) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    {
        file_writer writer(tmpFile, 4096, 4);
        writer.createArray<int>(1000);
        writer.create<int>(42);
    }
    {
        ASSERT_EQ(std::filesystem::file_size(tmpFile), 1001 * sizeof(int));
        std::ifstream    f(tmpFile, std::ios::binary);
        ASSERT_TRUE(f.good());
        std::vector<int> ints(1001);
        f.read(reinterpret_cast<char*>(ints.data()), 1001 * sizeof(int));
        EXPECT_TRUE(f.good());
        EXPECT_EQ(ints[ints.size() - 1], 42);
    }
    {
        file      mapped(tmpFile);
        std::span ints(reinterpret_cast<const int*>(mapped.data()), mapped.size() / sizeof(int));
        EXPECT_EQ(ints.size(), 1001);
        EXPECT_EQ(ints[ints.size() - 1], 42);
    }
    fs::remove(tmpFile);
}

struct TestHeader {
    offset_span<char> hello;
    offset_ptr<int>   data;
};

TEST(Writer, Header) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";
    {
        file_writer writer(tmpFile, 4096, 4);
        TestHeader* header = writer.create<TestHeader>();
        header->hello = writer.createArray("Hello World!");
        header->data = writer.create<int>(42);
        EXPECT_EQ(writer.size(), 44);
    }
    {
        file              mapped(tmpFile);
        const TestHeader* header = reinterpret_cast<const TestHeader*>(mapped.data());
        EXPECT_EQ(mapped.size(), 44);
        EXPECT_THAT(header->hello, ::testing::ElementsAre('H', 'e', 'l', 'l', 'o', ' ', 'W', 'o',
                                                          'r', 'l', 'd', '!', '\0'));
        EXPECT_EQ(header->hello[header->hello.size() - 2], '!');
        EXPECT_EQ(header->hello[header->hello.size() - 1], '\0');
        EXPECT_EQ(reinterpret_cast<uintptr_t>(&header->hello[header->hello.size()]) -
                      reinterpret_cast<uintptr_t>(header),
                  37); // next byte after string is not 4-byte aligned
        EXPECT_EQ(reinterpret_cast<uintptr_t>(&*header->data) - reinterpret_cast<uintptr_t>(header),
                  40); // int is 4-byte aligned
        EXPECT_EQ(*header->data, 42);
    }
    fs::remove(tmpFile);
}

void writeMyCustomObjectToFile(const decodeless::mapped_file_allocator<std::byte>& allocator) {
    create::array<int>(allocator, 1000);
    create::object<int>(allocator, 42);
}

void writeMyCustomObjectToMemory(const decodeless::mapped_memory_allocator<std::byte>& allocator) {
    create::array<int>(allocator, 1000);
    create::object(allocator, 42);
}

void writeMyCustomObject(const std::pmr::polymorphic_allocator<std::byte>& allocator) {
    create::array<int>(allocator, 1000);
    create::object(allocator, 42);
}

TEST(Writer, Allocators) {
    fs::path tmpFile = fs::path{testing::TempDir()} / "test.dat";

    {
        // templated file-only allocator
        decodeless::file_writer fileWriter(tmpFile, 4096, 4);
        writeMyCustomObjectToFile(fileWriter.resource());
        EXPECT_EQ(reinterpret_cast<int*>(fileWriter.data())[1000], 42);
    }

    {
        // templated memory-only allocator
        decodeless::memory_writer memoryWriter(4096, 4);
        writeMyCustomObjectToMemory(memoryWriter.resource());
        EXPECT_EQ(reinterpret_cast<int*>(memoryWriter.data())[1000], 42);
    }

    {
        // polymorphic allocator
        decodeless::pmr_file_writer   pmrFileWriter(tmpFile, 4096, 4);
        decodeless::pmr_memory_writer pmrMemoryWriter(4096, 4);
        writeMyCustomObject(&pmrFileWriter.resource());
        writeMyCustomObject(&pmrMemoryWriter.resource());
        EXPECT_EQ(reinterpret_cast<int*>(pmrFileWriter.data())[1000], 42);
        EXPECT_EQ(reinterpret_cast<int*>(pmrMemoryWriter.data())[1000], 42);
    }

    fs::remove(tmpFile);
}
