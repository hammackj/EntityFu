solution "EntityFu"
	configurations { "Debug", "Release" }
	platforms { "Native", "Universal" }
	objdir "../tmp/"
	targetdir "../bin/"
	location "../build/"
	linkoptions  { "-std=c++14" }
	buildoptions { "-std=c++14" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	project "EntityFu"
		language "C++"
		kind "StaticLib"
		files { "../src/EntityFu.hpp", "../src/EntityFu.cpp" }

	project "EntityFuTester"
		kind "ConsoleApp"
		language "C++"
		files { "../src/main.cpp"}
		links { "EntityFu" }
