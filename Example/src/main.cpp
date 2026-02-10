#include <iostream>

#include <array>
#include <filesystem>
#include <argless.hpp>


enum class ExampleEnum
{
	None,
	Active,
	Inactive = 17
};

template <>
struct argless::enum_refl<ExampleEnum>
{
	using values = argless::enum_values<
		argless::enum_value<"on", ExampleEnum::Active, true>,
		argless::enum_value<"off", ExampleEnum::Inactive, true>,
		argless::enum_value<"enable", ExampleEnum::Active>,
		argless::enum_value<"disable", ExampleEnum::Inactive>
	>;
};

using app = argless::app<"ExampleArglessApp", argless::nodesc, argless::accumulate<std::filesystem::path>,
		argless::arg<std::optional<argless::force<const char*>>, "--help", "this is example description", "-h", "?", "-help">,
		argless::arg<void, "--version", argless::nodesc, "-V", "-version">,
		argless::arg<void, "--flag">,
		argless::arg<unsigned int, "--positive-int">,
		argless::arg<float, "--float">,
		argless::arg<std::array<bool, 3>, "--array">,
		argless::arg<std::vector<double>, "--dynamic-array">,
		argless::arg<std::variant<bool, float>, "--variant">,
		argless::arg<std::tuple<int, float>, "--tuple">,
		argless::arg<std::optional<const char*>, "--optional">,
		argless::arg<ExampleEnum, "--enum">,

		argless::arg<argless::option<"y", "n">, "--option">,

		argless::arg<argless::rename<int, "float100%">, "--rename">,
		argless::arg<argless::transform<[](long v) { return static_cast<float>(v) / 10; }>, "--transform">,
		argless::arg<argless::validate<[](const argless::rename<std::filesystem::path, "abs path">& p) { return p.is_absolute(); }>, "--validate">,

		argless::arg<argless::accumulate<void>, "--accumulate">,
		argless::arg<argless::required<bool>, "--required">,
		argless::arg<argless::default_value<int, 2137>, "--default-value">
	>;

int main(int argc, const char** argv)
{
	auto result = app::parse(argc, argv);

	if (auto& err = result.error(); err && !result.get<"--help">())
	{
		std::cerr << argless::help_error<app>(err, argc, argv) << std::endl;
		return EXIT_FAILURE;
	}

	if (auto& arg = result.get<"--help">())
	{
		if (*arg)
			std::cout << argless::help_arg<app>(static_cast<const char*>(**arg)) << std::endl;
		else
			std::cout << argless::help_app<app>() << std::endl;
		return EXIT_SUCCESS;
	}

	if (result.get<"-V">())
	{
		std::cout << app::name << "-" << ARGLESS_VERSION_MAJOR << "." << ARGLESS_VERSION_MINOR << "." << ARGLESS_VERSION_PATCH << std::endl;
		std::cout << "Executable: " << result.path() << std::endl;
		return EXIT_SUCCESS;
	}

	if (result.get<"--flag">())
		std::cout << "--flag is enabled" << std::endl;

	if (auto& pi = result.get<"--positive-int">())
		std::cout << "--positive-int: " << *pi << std::endl;

	if (auto& f = result.get<"--float">())
		std::cout << "--float: " << *f << std::endl;

	if (auto& a = result.get<"--array">())
		std::cout << "--array: " << (*a)[0] << " " << (*a)[1] << " " << (*a)[2] << std::endl;

	if (auto& da = result.get<"--dynamic-array">())
	{
		std::cout << "--dynamic-array: ";
		for (auto& v : *da)
			std::cout << v << " ";
		std::cout << std::endl;
	}

	if (auto& v = result.get<"--variant">())
	{
		if (auto* b = std::get_if<bool>(&*v))
			std::cout << "--variant: bool: " << b << std::endl;

		if (auto* f = std::get_if<float>(&*v))
			std::cout << "--variant: float: " << f << std::endl;
	}

	if (auto& t = result.get<"--tuple">())
		std::cout << "--tuple: " << std::get<0>(*t) << " " << std::get<1>(*t) << std::endl;

	if (auto& o = result.get<"--optional">())
	{
		if (auto& ov = *o)
			std::cout << "--optional: " << *ov << std::endl;
		else
			std::cout << "--optional: std::nullopt" << std::endl;
	}

	if (auto& e = result.get<"--enum">())
		std::cout << "--enum: " << static_cast<int>(*e) << std::endl;

	if (auto& o = result.get<"--option">())
	{
		auto& opt = *o;
		if (opt.is<"y">())
			std::cout << "--option: y" << std::endl;
		else if (opt.is<"n">())
			std::cout << "--option: y" << std::endl;
	}

	if (auto& r = result.get<"--rename">())
		std::cout << "--rename: " << *r << std::endl;

	if (auto& r = result.get<"--transform">())
		std::cout << "--transform: " << *r << std::endl;

	if (auto& v = result.get<"--validate">())
		std::cout << "--validate: " << *v << std::endl;

	if (auto& c = result.get<"--accumulate">())
		std::cout << "--accumulate: " << c << std::endl;

	std::cout << "--required: " << result.get<"--required">() << std::endl;

	std::cout << "--default-value: " << result.get<"--default-value">() << std::endl;

	if (result.get<>())
		std::cout << "no-arg paths:" << std::endl;
	for (auto& path : result.get()) // get() is same as get<>()
		std::cout << path << std::endl;

	return EXIT_SUCCESS;
}
