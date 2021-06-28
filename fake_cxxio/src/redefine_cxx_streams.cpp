// Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "redefine_cxx_streams.h"

namespace std {
    extern istream cin;
    extern ostream cout;
    extern ostream cerr;
    extern ostream clog;
}

namespace __stub_cxxio {
    using namespace __gnu_cxx;
    extern stdio_sync_filebuf<char> buf_cin_sync;
    extern stdio_sync_filebuf<char> buf_cout_sync;
    extern stdio_sync_filebuf<char> buf_cerr_sync;

}

using namespace std;
using namespace __gnu_cxx;
using namespace __stub_cxxio;

INIT_STUB_CPP(fake_cxxio) {
    new (&buf_cin_sync) stdio_sync_filebuf<char>(stdin);
    new (&buf_cout_sync) stdio_sync_filebuf<char>(stdout);
    new (&buf_cerr_sync) stdio_sync_filebuf<char>(stderr);

    new (&cin) istream(&buf_cin_sync);
    new (&cout) ostream(&buf_cout_sync);
	new (&cerr) ostream(&buf_cerr_sync);
	new (&clog) ostream(&buf_cerr_sync);
	cin.tie(&cout);
    cerr.setf(ios_base::unitbuf);
    cerr.tie(&cout);
    return 0;
}

bool ios_base::sync_with_stdio(bool __sync) {
    return false;
}

FINI_STUB_CPP(fake_cxxio) {
    __try {
        cout.flush();
        cerr.flush();
        clog.flush();
        return 0;
    } __catch(...) {
        return 1;
    }
}

