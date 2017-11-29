#include <fc/compress/lzma.hpp>
#include <fc/filesystem.hpp>

#include <iostream>
#include <string>

using namespace fc;

int main( int argc, char** argv )
{
    if( argc != 2 )
    {
        std::cout << "usage: " << argv[0] << " <filename>\n";
        exit( -1 );
    }

    auto src = std::string( argv[1] );
    auto dst = src + ".compressed";
    lzma_compress_file( src, dst );

    lzma_decompress_file( dst, src + ".decompressed" );

    return 0;
}
