#include <iostream>
#include <utility>
#include <lv_cpp/meta/template_info.hpp>

#include <format>
#include <optional>
#include <lv_cpp/collections/internal/buffer.hpp>
#include <vector>
#include <memory>
#include <scoped_allocator>
#include <unordered_set>

int main(int argc, char const *argv[])
{
    using StringAllocator = std::allocator<std::string>;
    leviathan::collections::buffer<std::string, StringAllocator> buffer;

    StringAllocator salloc;

    buffer.emplace(salloc, buffer.begin(), "This sentence must be longer enough and will be moved to last position");
    buffer.emplace(salloc, buffer.begin(), "This sentence must be longer enough and will be moved to third position");
    buffer.emplace(salloc, buffer.begin(), "This sentence must be longer enough and will be moved to second position");

    buffer.emplace(salloc, buffer.begin(), buffer[0]);


    assert(buffer[0] == "This sentence must be longer enough and will be moved to second position");
    assert(buffer[1] == "This sentence must be longer enough and will be moved to second position");

    buffer.dispose(salloc);

    return 0;
}
