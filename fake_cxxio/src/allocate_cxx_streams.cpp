// Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "allocate_cxx_streams.h"

namespace std {
    typedef char fake_istream[sizeof(istream)]
    __attribute__((aligned(__alignof__(istream))));
    typedef char fake_ostream[sizeof(ostream)]
    __attribute__((aligned(__alignof__(ostream))));

    fake_istream cin;
    fake_ostream cout;
    fake_ostream cerr;
    fake_ostream clog;
}

namespace __stub_cxxio {
    using namespace std;
    using namespace __gnu_cxx;

    typedef char fake_stdiobuf[sizeof(stdio_sync_filebuf<char>)]
    __attribute__ ((aligned(__alignof__(stdio_sync_filebuf<char>))));
    
    fake_stdiobuf buf_cin_sync;
    fake_stdiobuf buf_cout_sync;
    fake_stdiobuf buf_cerr_sync;
}
