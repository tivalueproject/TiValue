#pragma once

#include <string>

namespace TiValue {
    namespace utilities {

        std::string escape_string_for_c_source_code(const std::string& input);

    }
} // end namespace TiValue::utilities
