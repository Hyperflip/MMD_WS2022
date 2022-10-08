require("./tools/premake5/premake-ps4")

-- premake5.lua
workspace "HelloWorld"
	configurations { "Debug", "Release" }
	platforms { "x64", "Orbis" }
	
project "HelloWorld"
	kind "ConsoleApp" 	-- can also be "WindowedApp"
	language "C++"		-- also supports C and C#
	cppdialect "C++14"
	fatalwarnings { "warnings" }
	
	targetdir "build/%{cfg.platform}/bin/%{cfg.buildcfg}"
	objdir "build/%{cfg.platform}/bin/%{cfg.buildcfg}/obj"

	files { "**.h", "**.cpp" }
	-- Clean Function --
	newaction {
		trigger     = "clean",
		description = "clean the software",
		execute     = function ()
			print("clean the build...")
			os.rmdir("./build")
			os.remove("*.sln")
   			os.remove("*.vcxproj")
			os.remove("*.user")
			print("done.")
		end
	}

	filter { "platforms:x64" }
		system "Windows"
		architecture "x86_64"

	filter { "platforms:Orbis" }
		system "Orbis"
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		
	filter "configurations:Release"
		defines { "NDEBUG", "RELEASE" }
		optimize "On"