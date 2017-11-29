#include "fc/utf8.hpp"

#include "utf8/checked.h"
#include "utf8/core.h"
#include "utf8/unchecked.h"

#include <assert.h>

namespace fc {

   bool is_utf8( const std::string& str )
   {
      return utf8::is_valid( str.begin(), str.end() );
   }

   void decodeUtf8(const std::string& input, std::wstring* storage)
   {
     assert(storage != nullptr);

     utf8::utf8to32(input.begin(), input.end(), std::back_inserter(*storage));
   }

   void encodeUtf8(const std::wstring& input, std::string* storage)
   {
     assert(storage != nullptr);

     utf8::utf32to8(input.begin(), input.end(), std::back_inserter(*storage));
   }

} ///namespace fc


