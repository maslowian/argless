module;
#define ARGLESS_STDH_ALL
#include <argless.hpp>
export module argless;

export _ARGLESS_BEGIN

using _ARGLESS app;
using _ARGLESS arg;

using _ARGLESS nodesc; 

using _ARGLESS result;
using _ARGLESS result_error;
using _ARGLESS result_error_type;

using _ARGLESS accumulate;
using _ARGLESS required;
using _ARGLESS default_value;

using _ARGLESS rename;
using _ARGLESS force;
using _ARGLESS fallback;
using _ARGLESS transform;
using _ARGLESS validate;
using _ARGLESS option;

using _ARGLESS enum_value;
using _ARGLESS enum_values;
using _ARGLESS enum_refl;

using _ARGLESS get_name;

using _ARGLESS help_error;
using _ARGLESS help_arg;
using _ARGLESS help_app;

_ARGLESS_END

// TODO: move to argless.core;
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
using _ARGLESS_CORE str_from;
using _ARGLESS_CORE str_cast;

using _ARGLESS_CORE get_charu;

using _ARGLESS_CORE seq;
using _ARGLESS_CORE seq_nocase;
using _ARGLESS_CORE stot;
using _ARGLESS_CORE slen;

_ARGLESS_CORE_END
