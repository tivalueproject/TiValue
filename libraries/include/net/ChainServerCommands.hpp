#pragma once

/**
 * This is an internal header for tiv. It does not contain any classes or functions intended for clients.
 * It exists purely as an implementation detail, and may change at any time without notice.
 */

#include <fc/reflect/reflect.hpp>

const static uint32_t PROTOCOL_VERSION = 0;

namespace TiValue {
    namespace net {
        namespace detail {
            enum ChainServerCommands {
                finish = 0,
                get_blocks_from_number
            };
        }
    }
} //namespace TiValue::net::detail

FC_REFLECT_ENUM(TiValue::net::detail::ChainServerCommands, (finish)(get_blocks_from_number))
FC_REFLECT_TYPENAME(TiValue::net::detail::ChainServerCommands)
