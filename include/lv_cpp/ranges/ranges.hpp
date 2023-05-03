/*
    We try implement these range adaptors ahead of cpp standard or compiler.
*/

#pragma once 

#include "common.hpp"


#include <optional>
#include <compare>
#include <assert.h>

#include <iostream>
// #include <lv_cpp/meta/template_info.hpp>

#include "concat.hpp"

#include "enumerate.hpp"     // C++23
#include "repeat.hpp"        // C++23
#include "join_with.hpp"     // C++20/23
#include "zip.hpp"           // C++23
#include "adjacent.hpp"      // C++23
#include "chunk.hpp"         // C++23
#include "chunk_by.hpp"      // C++23
#include "stride.hpp"        // C++23

