// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __WELDR_TARGET_H
#define __WELDR_TARGET_H

#ifndef WELDING
#define input_target(buf, src, len) test_input_target(buf, src, len)
#define output_target(buf, tgt, len) test_output_target(buf, tgt, len)
#endif

void test_input_target(char *buf, const char *source, size_t size);

void test_output_target(const char *buf, char *source, size_t size);

#endif
