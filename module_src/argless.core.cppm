module;
#define ARGLESS_STDH_ALL
#include <argless.hpp>
export module argless.core;

import argless;

export _ARGLESS_CORE_BEGIN

using _ARGLESS_CORE parser;
using _ARGLESS_CORE args;
using _ARGLESS_CORE make_default;
using _ARGLESS_CORE make_expected;

using _ARGLESS_CORE type_name;
using _ARGLESS_CORE wrapped_name;
using _ARGLESS_CORE optional_name;
using _ARGLESS_CORE array_wrap_name;
using _ARGLESS_CORE or_name;
using _ARGLESS_CORE and_name;
using _ARGLESS_CORE number_name; 

using _ARGLESS_CORE str;
using _ARGLESS_CORE str_cast;
using _ARGLESS_CORE str_from;
using _ARGLESS_CORE str_cast;

using _ARGLESS_CORE get_charu;

using _ARGLESS_CORE seq;
using _ARGLESS_CORE seq_nocase;
using _ARGLESS_CORE stot;
using _ARGLESS_CORE slen;

_ARGLESS_CORE_END
