module;
#define ARGLESS_STDH_ALL
#include <argless.hpp>
export module argless;

export namespace argless
{
	using ::argless::app;
	using ::argless::arg;

	using ::argless::nodesc; 

	using ::argless::result;
	using ::argless::result_error;
	using ::argless::result_error_type;

	using ::argless::accumulate;
	using ::argless::required;
	using ::argless::default_value;

	using ::argless::rename;
	using ::argless::force;
	using ::argless::fallback;
	using ::argless::transform;
	using ::argless::validate;
	using ::argless::option;

	using ::argless::enum_value;
	using ::argless::enum_values;
	using ::argless::enum_refl;

	using ::argless::get_name;

	using ::argless::help_error;
	using ::argless::help_arg;
	using ::argless::help_app;
}
