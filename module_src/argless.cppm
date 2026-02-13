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
