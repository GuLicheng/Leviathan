/*

    factories:
        concat
        enumerate
        repeat

    adaptor:
        concat_with
        join_with
*/

#pragma once 


#include <concepts>
#include <optional>
#include <tuple>
#include <compare>
#include <functional>
#include <variant>
#include <assert.h>

#include <iostream>
#include <lv_cpp/meta/template_info.hpp>
#include "common.hpp"

#include "enumerate.hpp"
#include "concat.hpp"
#include "common.hpp"
#include "repeat.hpp"
#include "join_with.hpp"
#include "zip.hpp"
#include "adjacent.hpp"
#include "chunk.hpp"
#include "chunk_by.hpp"
#include "stride.hpp"

