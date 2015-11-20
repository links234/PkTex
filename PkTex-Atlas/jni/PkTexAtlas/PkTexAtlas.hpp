#ifndef PKTEXATLAS_HPP
#define PKTEXATLAS_HPP

#include "MPACK.hpp"

namespace PkTex
{
    class Atlas : public MPACK::Core::ConsoleApplication
    {
    public:
    	Atlas();
    	~Atlas();

        int Main();
    };
}

#endif
