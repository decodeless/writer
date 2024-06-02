// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <decodeless/offset_ptr.hpp>
#include <decodeless/offset_span.hpp>
#include <decodeless/writer.hpp>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace decodeless;

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
