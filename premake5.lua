dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

newoption {
	trigger = "dev-build",
	description = "Enable development builds of the client."
}

newoption {
	trigger = "ci-build",
	description = "Enable CI builds of the client."
}


dependencies.load()

workspace "shield-development"
	startproject "proxy-dll"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations {"Debug", "Release"}

	language "C++"
	cppdialect "C++20"

	architecture "x86_64"
	platforms "x64"

	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	if _OPTIONS["dev-build"] then
		defines {"DEV_BUILD"}
	end

	if _OPTIONS["ci-build"] then
		defines {"CI"}
	end

	flags {"NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks"}

	filter "platforms:x64"
		defines {"_WINDOWS", "WIN32"}
	filter {}

	filter "configurations:Release"
		optimize "Size"
		buildoptions {"/GL"}
		linkoptions { "/IGNORE:4702", "/LTCG" }
		defines {"NDEBUG"}
		flags {"FatalCompileWarnings"}
	filter {}

	filter "configurations:Debug"
		optimize "Debug"
		defines {"DEBUG", "_DEBUG"}
	filter {}

project "shared-code"
	kind "StaticLib"
	language "C++"

	files {"./source/shared-code/**.hpp", "./source/shared-code/**.cpp"}

	includedirs {"./source/shared-code", "%{prj.location}/src"}

	resincludedirs {"$(ProjectDir)src"}

	dependencies.imports()

project "proxy-dll"
	kind "SharedLib"
	language "C++"

	targetname "d3d11"

	pchheader "std_include.hpp"
	pchsource "source/proxy-dll/std_include.cpp"

	files {"./source/proxy-dll/**.rc", "./source/proxy-dll/**.hpp", "./source/proxy-dll/**.cpp", "./source/proxy-dll/resources/**.*"}

	includedirs {"./source/proxy-dll", "./source/shared-code", "%{prj.location}/src"}

	resincludedirs {"$(ProjectDir)src"}

	links {"shared-code"}

	if _OPTIONS["copy-to"] then
		postbuildcommands {"copy /y \"$(TargetPath)\" \"" .. _OPTIONS["copy-to"] .. "\""}
	end

	dependencies.imports()


group "Dependencies"
	dependencies.projects()
