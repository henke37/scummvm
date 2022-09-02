/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define ENABLE_XCODE

#if (defined(_WIN32) || defined(WIN32)) && !defined(__GNUC__)
#define USE_WIN32_API
#endif

#if (defined(_WIN32) || defined(WIN32))
#define _WIN32_WINNT 0x0502
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sstream>
#include <sys/param.h>
#include <sys/stat.h>
#endif

#include "create_project.h"
#include "config.h"

#include "cmake.h"
#include "codeblocks.h"
#include "msbuild.h"
#include "msvc.h"
#include "xcode.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <utility>

#include <cstdlib>
#include <cstring>
#include <ctime>

namespace {

/**
 * Display the help text for the program.
 *
 * @param exe Name of the executable.
 */
void displayHelp(const char *exe);

/**
 * Build a list of options to enable or disable GCC warnings
 *
 * @param globalWarnings Resulting list of warnings
 */
void addGCCWarnings(StringList &globalWarnings);
} // End of anonymous namespace

enum ProjectType {
	kProjectNone,
	kProjectCMake,
	kProjectCodeBlocks,
	kProjectMSVC,
	kProjectXcode
};

std::map<std::string, bool> isEngineEnabled;

int main(int argc, char *argv[]) {
#ifndef USE_WIN32_API
	// Initialize random number generator for UUID creation
	std::srand((unsigned int)std::time(nullptr));
#endif

	if (argc < 2) {
		displayHelp(argv[0]);
		return -1;
	}

	const std::string srcDir = argv[1];

	BuildSetup setup;
	setup.srcDir = unifyPath(srcDir);
	removeTrailingSlash(setup.srcDir);

	setup.filePrefix = setup.srcDir;
	setup.outputDir = '.';

	setup.engines = parseEngines(setup.srcDir);

	if (setup.engines.empty()) {
		std::cout << "WARNING: No engines found in configure file or configure file missing in \"" << setup.srcDir << "\"\n";
		return 0;
	}

	setup.features = getAllFeatures();

	ProjectType projectType = kProjectNone;
	const MSVCVersion *msvc = nullptr;
	int msvcVersion = 0;

	// Parse command line arguments
	using std::cout;
	for (int i = 2; i < argc; ++i) {
		if (!std::strcmp(argv[i], "--list-engines")) {
			cout << " The following enables are available in the " PROJECT_DESCRIPTION " source distribution\n"
			     << " located at \"" << srcDir << "\":\n";

			cout << "   state  |       name      |     description\n\n";
			cout.setf(std::ios_base::left, std::ios_base::adjustfield);
			for (EngineDescList::const_iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
				cout << ' ' << (j->enable ? " enabled" : "disabled") << " | " << std::setw((std::streamsize)15) << j->name << std::setw((std::streamsize)0) << " | " << j->desc << "\n";
			cout.setf(std::ios_base::right, std::ios_base::adjustfield);

			return 0;

		} else if (!std::strcmp(argv[i], "--cmake")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectCMake;

		} else if (!std::strcmp(argv[i], "--codeblocks")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectCodeBlocks;

		} else if (!std::strcmp(argv[i], "--msvc")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectMSVC;

#ifdef ENABLE_XCODE
		} else if (!std::strcmp(argv[i], "--xcode")) {
			if (projectType != kProjectNone) {
				std::cerr << "ERROR: You cannot pass more than one project type!\n";
				return -1;
			}

			projectType = kProjectXcode;
#endif

		} else if (!std::strcmp(argv[i], "--msvc-version")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"version\" parameter for \"--msvc-version\"!\n";
				return -1;
			}

			msvcVersion = atoi(argv[++i]);

		} else if (!strncmp(argv[i], "--enable-engine=", 16)) {
			const char *names = &argv[i][16];
			if (!*names) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			TokenList tokens = tokenize(names, ',');
			TokenList::const_iterator token = tokens.begin();
			while (token != tokens.end()) {
				std::string name = *token++;
				if (!setEngineBuildState(name, setup.engines, true)) {
					std::cerr << "ERROR: \"" << name << "\" is not a known engine!\n";
					return -1;
				}
			}
		} else if (!strncmp(argv[i], "--disable-engine=", 17)) {
			const char *names = &argv[i][17];
			if (!*names) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			TokenList tokens = tokenize(names, ',');
			TokenList::const_iterator token = tokens.begin();
			while (token != tokens.end()) {
				std::string name = *token++;
				if (!setEngineBuildState(name, setup.engines, false)) {
					std::cerr << "ERROR: \"" << name << "\" is not a known engine!\n";
					return -1;
				}
			}
		} else if (!strncmp(argv[i], "--enable-", 9)) {
			const char *name = &argv[i][9];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!std::strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = true;
			} else {
				setup.setFeatureEnabled(name, true);
			}
		} else if (!strncmp(argv[i], "--disable-", 10)) {
			const char *name = &argv[i][10];
			if (!*name) {
				std::cerr << "ERROR: Invalid command \"" << argv[i] << "\"\n";
				return -1;
			}

			if (!std::strcmp(name, "all-engines")) {
				for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
					j->enable = false;
			} else {
				setup.setFeatureEnabled(name, false);
			}
		} else if (!std::strcmp(argv[i], "--file-prefix")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"prefix\" parameter for \"--file-prefix\"!\n";
				return -1;
			}

			setup.filePrefix = unifyPath(argv[++i]);
			removeTrailingSlash(setup.filePrefix);
		} else if (!std::strcmp(argv[i], "--output-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--output-dir\"!\n";
				return -1;
			}

			setup.outputDir = unifyPath(argv[++i]);
			removeTrailingSlash(setup.outputDir);
		} else if (!std::strcmp(argv[i], "--include-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--include-dir\"!\n";
				return -1;
			}
			std::string includeDir = unifyPath(argv[++i]);
			removeTrailingSlash(includeDir);
			setup.includeDirs.push_back(includeDir);
		} else if (!std::strcmp(argv[i], "--library-dir")) {
			if (i + 1 >= argc) {
				std::cerr << "ERROR: Missing \"path\" parameter for \"--library-dir\"!\n";
				return -1;
			}
			std::string libraryDir = unifyPath(argv[++i]);
			removeTrailingSlash(libraryDir);
			setup.libraryDirs.push_back(libraryDir);
		} else if (!std::strcmp(argv[i], "--build-events")) {
			setup.runBuildEvents = true;
		} else if (!std::strcmp(argv[i], "--installer")) {
			setup.runBuildEvents = true;
			setup.createInstaller = true;
		} else if (!std::strcmp(argv[i], "--tools")) {
			setup.devTools = true;
		} else if (!std::strcmp(argv[i], "--tests")) {
			setup.tests = true;
		} else if (!std::strcmp(argv[i], "--sdl1")) {
			setup.useSDL2 = false;
		} else if (!std::strcmp(argv[i], "--use-canonical-lib-names")) {
			setup.useCanonicalLibNames = true;
		} else if (!std::strcmp(argv[i], "--use-windows-unicode")) {
			setup.useWindowsUnicode = true;
		} else if (!std::strcmp(argv[i], "--use-windows-ansi")) {
			setup.useWindowsUnicode = false;
		} else {
			std::cerr << "ERROR: Unknown parameter \"" << argv[i] << "\"\n";
			return -1;
		}
	}

	// When building tests, disable some features
	if (setup.tests) {
		setup.useStaticDetection = false;
		setup.setFeatureEnabled("mt32emu", false);
		setup.setFeatureEnabled("eventrecorder", false);

		for (EngineDescList::iterator j = setup.engines.begin(); j != setup.engines.end(); ++j)
			j->enable = false;
	} else if (setup.devTools) {
		setup.useStaticDetection = false;
	}

	// HACK: Vorbis and Tremor can not be enabled simultaneously
	if (setup.featureEnabled("tremor")) {
		setup.setFeatureEnabled("vorbis", false);
	}

	// HACK: Fluidsynth and Fluidlite can not be enabled simultaneously
	if (setup.featureEnabled("fluidsynth")) {
		setup.setFeatureEnabled("fluidlite", false);
	}

	// HACK: These features depend on OpenGL
	if (!setup.featureEnabled("opengl")) {
		setup.setFeatureEnabled("opengl_game", false);
		setup.setFeatureEnabled("opengl_shaders", false);
	}

	// Disable engines for which we are missing dependencies
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (i->enable) {
			for (StringList::const_iterator ef = i->requiredFeatures.begin(); ef != i->requiredFeatures.end(); ++ef) {
				FeatureList::iterator feature = std::find(setup.features.begin(), setup.features.end(), *ef);
				if (feature == setup.features.end()) {
					std::cerr << "WARNING: Missing feature " << *ef << " from engine " << i->name << '\n';
				} else if (!feature->enable) {
					setEngineBuildState(i->name, setup.engines, false);
					break;
				}
			}
			isEngineEnabled[i->name] = true;
		}
	}

	// Print status
	cout << "Enabled engines:\n\n";
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (i->enable)
			cout << "    " << i->desc << '\n';
	}

	cout << "\nDisabled engines:\n\n";
	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable)
			cout << "    " << i->desc << '\n';
	}

	cout << "\nEnabled features:\n\n";
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable)
			cout << "    " << i->description << '\n';
	}

	cout << "\nDisabled features:\n\n";
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (!i->enable)
			cout << "    " << i->description << '\n';
	}

	// Check if tools and tests are enabled simultaneously
	if (setup.devTools && setup.tests) {
		std::cerr << "ERROR: The tools and tests projects cannot be created simultaneously\n";
		return -1;
	}

	// Setup defines and libraries
	setup.defines = getEngineDefines(setup.engines);

	// Add features
	DefineList featureDefines = getFeatureDefines(setup.features);
	setup.defines += featureDefines;

	bool backendWin32 = false;
	if (projectType == kProjectXcode) {
		setup.defines.add("POSIX");
		// Define both MACOSX, and IPHONE, but only one of them will be associated to the
		// correct target by the Xcode project provider.
		// This define will help catching up target dependend files, like "browser_osx.mm"
		// The suffix ("_osx", or "_ios") will be used by the project provider to filter out
		// the files, according to the target.
		setup.defines.add("MACOSX");
		setup.defines.add("IPHONE");
	} else if (projectType == kProjectMSVC || projectType == kProjectCodeBlocks) {
		setup.defines.add("WIN32");
		backendWin32 = true;
	} else {
		// As a last resort, select the backend files to build based on the platform used to build create_project.
		// This is broken when cross compiling.
#if defined(_WIN32) || defined(WIN32)
		setup.defines.add("WIN32");
		backendWin32 = true;
#else
		setup.defines.add("POSIX");
#endif
	}

	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable) {
			if (!strcmp(i->name, "updates"))
				setup.defines.add("USE_SPARKLE");
			else if (backendWin32 && !strcmp(i->name, "libcurl"))
				setup.defines.add("CURL_STATICLIB");
			else if (!strcmp(i->name, "fluidlite"))
				setup.defines.add("USE_FLUIDSYNTH");
		}
	}

	setup.defines.add("SDL_BACKEND");
	if (!setup.useSDL2) {
		cout << "\nBuilding against SDL 1.2\n\n";
	} else {
		cout << "\nBuilding against SDL 2.0\n\n";
		// TODO: This also defines USE_SDL2 in the preprocessor, we don't do
		// this in our configure/make based build system. Adapt create_project
		// to replicate this behavior.
		setup.defines.add("USE_SDL2");
	}

	if (setup.useStaticDetection) {
		setup.defines.add("DETECTION_STATIC");
	}

	if (setup.featureEnabled("opengl")) {
		setup.defines.add("USE_GLAD");
	}

	// List of global warnings and map of project-specific warnings
	// FIXME: As shown below these two structures have different behavior for
	// Code::Blocks and MSVC. In Code::Blocks this is used to enable *and*
	// disable certain warnings (and some other not warning related flags
	// actually...). While in MSVC this is solely for disabling warnings.
	// That is really not nice. We should consider a nicer way of doing this.
	StringList globalWarnings;
	std::map<std::string, StringList> projectWarnings;

	CreateProjectTool::ProjectProvider *provider = nullptr;

	switch (projectType) {
	default:
	case kProjectNone:
		std::cerr << "ERROR: No project type has been specified!\n";
		return -1;

	case kProjectCMake:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the CMake project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::CMakeProvider(globalWarnings, projectWarnings);

		break;

	case kProjectCodeBlocks:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the CodeBlocks project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::CodeBlocksProvider(globalWarnings, projectWarnings);

		break;

	case kProjectMSVC:
		// Auto-detect if no version is specified
		if (msvcVersion == 0) {
			msvcVersion = getInstalledMSVC();
			if (msvcVersion == 0) {
				std::cerr << "ERROR: No Visual Studio versions found, please specify one with \"--msvc-version\"\n";
				return -1;
			} else {
				cout << "Visual Studio " << msvcVersion << " detected\n\n";
			}
		}

		msvc = getMSVCVersion(msvcVersion);
		if (!msvc) {
			std::cerr << "ERROR: Unsupported version: \"" << msvcVersion << "\" passed to \"--msvc-version\"!\n";
			return -1;
		}

		////////////////////////////////////////////////////////////////////////////
		// For Visual Studio, all warnings are on by default in the project files,
		// so we pass a list of warnings to disable globally or per-project
		//
		////////////////////////////////////////////////////////////////////////////
		//
		// 4068 (unknown pragma)
		//   only used in scumm engine to mark code sections
		//
		// 4100 (unreferenced formal parameter)
		//
		// 4103 (alignment changed after including header, may be due to missing #pragma pack(pop))
		//   used by pack-start / pack-end
		//
		// 4127 (conditional expression is constant)
		//   used in a lot of engines
		//
		// 4244 ('conversion' conversion from 'type1' to 'type2', possible loss of data)
		//   throws tons and tons of warnings, most of them false positives
		//
		// 4250 ('class1' : inherits 'class2::member' via dominance)
		//   two or more members have the same name. Should be harmless
		//
		// 4267 ('var' : conversion from 'size_t' to 'type', possible loss of data)
		//   throws tons and tons of warnings (no immediate plan to fix all usages)
		//
		// 4310 (cast truncates constant value)
		//   used in some engines
		//
		// 4345 (behavior change: an object of POD type constructed with an
		// initializer of the form () will be default-initialized)
		//   used in Common::Array(), and it basically means that newer VS
		//   versions adhere to the standard in this case. Can be safely
		//   disabled.
		//
		// 4351 (new behavior: elements of array 'array' will be default initialized)
		//   a change in behavior in Visual Studio 2005. We want the new behavior, so it can be disabled
		//
		// 4512 ('class' : assignment operator could not be generated)
		//   some classes use const items and the default assignment operator cannot be generated
		//
		// 4577 ('noexcept' used with no exception handling mode specified)
		//
		// 4589 (Constructor of abstract class 'type' ignores initializer for virtual base class 'type')
		//   caused by Common::Stream virtual inheritance, should be harmless
		//
		// 4702 (unreachable code)
		//   mostly thrown after error() calls (marked as NORETURN)
		//
		// 4706 (assignment within conditional expression)
		//   used in a lot of engines
		//
		// 4800 ('type' : forcing value to bool 'true' or 'false' (performance warning))
		//
		// 4996 ('function': was declared deprecated)
		//   disabling it removes all the non-standard unsafe functions warnings (strcpy_s, etc.)
		//
		// 6211 (Leaking memory <pointer> due to an exception. Consider using a local catch block to clean up memory)
		//   we disable exceptions
		//
		// 6204 (possible buffer overrun in call to <function>: use of unchecked parameter <variable>)
		// 6385 (invalid data: accessing <buffer name>, the readable size is <size1> bytes, but <size2> bytes may be read)
		// 6386 (buffer overrun: accessing <buffer name>, the writable size is <size1> bytes, but <size2> bytes may be written)
		//   give way too many false positives
		//
		////////////////////////////////////////////////////////////////////////////
		//
		// 4189 (local variable is initialized but not referenced)
		//   false positive in lure engine
		//
		// 4355 ('this' : used in base member initializer list)
		//   only disabled for specific engines where it is used in a safe way
		//
		// 4373 (previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers)
		//
		// 4510 ('class' : default constructor could not be generated)
		//
		// 4511 ('class' : copy constructor could not be generated)
		//
		// 4610 (object 'class' can never be instantiated - user-defined constructor required)
		//   "correct" but harmless (as is 4510)
		//
		////////////////////////////////////////////////////////////////////////////

		globalWarnings.push_back("4068");
		globalWarnings.push_back("4100");
		globalWarnings.push_back("4103");
		globalWarnings.push_back("4127");
		globalWarnings.push_back("4244");
		globalWarnings.push_back("4250");
		globalWarnings.push_back("4310");
		globalWarnings.push_back("4345");
		globalWarnings.push_back("4351");
		globalWarnings.push_back("4512");
		globalWarnings.push_back("4589");
		globalWarnings.push_back("4702");
		globalWarnings.push_back("4706");
		globalWarnings.push_back("4800");
		globalWarnings.push_back("4996");
		globalWarnings.push_back("6204");
		globalWarnings.push_back("6211");
		globalWarnings.push_back("6385");
		globalWarnings.push_back("6386");

		if (msvcVersion >= 14) {
			globalWarnings.push_back("4267");
			globalWarnings.push_back("4577");
		}

		projectWarnings["agi"].push_back("4510");
		projectWarnings["agi"].push_back("4610");

		projectWarnings["agos"].push_back("4511");

		projectWarnings["dreamweb"].push_back("4355");

		projectWarnings["lure"].push_back("4189");
		projectWarnings["lure"].push_back("4355");

		projectWarnings["kyra"].push_back("4355");
		projectWarnings["kyra"].push_back("4510");
		projectWarnings["kyra"].push_back("4610");

		projectWarnings["m4"].push_back("4355");

		projectWarnings["sci"].push_back("4373");

		provider = new CreateProjectTool::MSBuildProvider(globalWarnings, projectWarnings, msvcVersion, *msvc);

		break;

	case kProjectXcode:
		if (setup.devTools || setup.tests) {
			std::cerr << "ERROR: Building tools or tests is not supported for the XCode project type!\n";
			return -1;
		}

		addGCCWarnings(globalWarnings);

		provider = new CreateProjectTool::XcodeProvider(globalWarnings, projectWarnings);
		break;
	}

	// Setup project name and description
	setup.projectName = PROJECT_NAME;
	setup.projectDescription = PROJECT_DESCRIPTION;

	if (setup.devTools) {
		setup.projectName += "-tools";
		setup.projectDescription += "Tools";
	}

	if (setup.tests) {
		setup.projectName += "-tests";
		setup.projectDescription += "Tests";
	}

	provider->createProject(setup);

	delete provider;
}

std::string unifyPath(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/');
	return result;
}

void removeTrailingSlash(std::string &path) {
	if (path.size() > 0 && path.at(path.size() - 1) == '/')
		path.erase(path.size() - 1);
}

namespace {

void displayHelp(const char *exe) {
	using std::cout;

	cout << "Usage:\n"
	     << exe << " path\\to\\source [optional options]\n"
	     << "\n"
	     << " Creates project files for the " PROJECT_DESCRIPTION " source located at \"path\\to\\source\".\n"
	        " The project files will be created in the directory where tool is run from and\n"
	        " will include \"path\\to\\source\" for relative file paths, thus be sure that you\n"
	        " pass a relative file path like \"..\\..\\trunk\".\n"
	        "\n"
	        " Additionally there are the following switches for changing various settings:\n"
	        "\n"
	        "Project specific settings:\n"
	        " --cmake                    build CMake project files\n"
	        " --codeblocks               build Code::Blocks project files\n"
	        " --msvc                     build Visual Studio project files\n"
	        " --xcode                    build XCode project files\n"
	        " --file-prefix prefix       allow overwriting of relative file prefix in the\n"
	        "                            MSVC project files. By default the prefix is the\n"
	        "                            \"path\\to\\source\" argument\n"
	        " --output-dir path          overwrite path, where the project files are placed\n"
	        "                            By default this is \".\", i.e. the current working\n"
	        "                            directory\n"
			" --include-dir path         add a path to the include search path"
			" --library-dir path         add a path to the library search path"
	        "\n"
	        "MSVC specific settings:\n"
	        " --msvc-version version     set the targeted MSVC version. Possible values:\n";

	const MSVCList msvc = getAllMSVCVersions();
	for (MSVCList::const_iterator i = msvc.begin(); i != msvc.end(); ++i)
		cout << "                           " << i->version << " stands for \"" << i->name << "\"\n";

	cout << "                            If no version is set, the latest installed version is used\n"
	        " --build-events             Run custom build events as part of the build\n"
	        "                            (default: false)\n"
	        " --installer                Create installer after the build (implies --build-events)\n"
	        "                            (default: false)\n"
	        " --tools                    Create project files for the devtools\n"
	        "                            (ignores --build-events and --installer, as well as engine settings)\n"
	        "                            (default: false)\n"
	        " --tests                    Create project files for the tests\n"
	        "                            (ignores --build-events and --installer, as well as engine settings)\n"
	        "                            (default: false)\n"
	        " --use-canonical-lib-names  Use canonical library names for linking. This makes it easy to use\n"
	        "                            e.g. vcpkg-provided libraries\n"
	        "                            (default: false)\n"
	        " --use-windows-unicode      Use Windows Unicode APIs\n"
	        "                            (default: true)\n"
	        " --use-windows-ansi         Use Windows ANSI APIs\n"
	        "                            (default: false)\n"
	        "\n"
	        "Engines settings:\n"
	        " --list-engines             list all available engines and their default state\n"
	        " --enable-engine=<name>     enable building of the engine with the name \"name\"\n"
	        " --disable-engine=<name>    disable building of the engine with the name \"name\"\n"
	        " --enable-all-engines       enable building of all engines\n"
	        " --disable-all-engines      disable building of all engines\n"
	        "\n"
	        "Optional features settings:\n"
	        " --enable-<name>            enable inclusion of the feature \"name\"\n"
	        " --disable-<name>           disable inclusion of the feature \"name\"\n"
	        "\n"
	        "SDL settings:\n"
	        " --sdl1                     link to SDL 1.2, instead of SDL 2.0\n"
	        "\n"
	        " There are the following features available:\n"
	        "\n";

	cout << "   state  |       name      |     description\n\n";
	const FeatureList features = getAllFeatures();
	cout.setf(std::ios_base::left, std::ios_base::adjustfield);
	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i)
		cout << ' ' << (i->enable ? " enabled" : "disabled") << " | " << std::setw((std::streamsize)15) << i->name << std::setw((std::streamsize)0) << " | " << i->description << '\n';
	cout.setf(std::ios_base::right, std::ios_base::adjustfield);
}

void addGCCWarnings(StringList &globalWarnings) {
	////////////////////////////////////////////////////////////////////////////
	//
	// -Wall
	//   enable all warnings
	//
	// -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
	//   disable annoying and not-so-useful warnings
	//
	// -Wpointer-arith -Wcast-qual -Wcast-align
	// -Wshadow -Wimplicit -Wnon-virtual-dtor -Wwrite-strings
	//   enable even more warnings...
	//
	// -fno-exceptions -fcheck-new
	//   disable exceptions, and enable checking of pointers returned by "new"
	//
	////////////////////////////////////////////////////////////////////////////

	globalWarnings.push_back("-Wall");
	globalWarnings.push_back("-Wno-long-long");
	globalWarnings.push_back("-Wno-multichar");
	globalWarnings.push_back("-Wno-unknown-pragmas");
	globalWarnings.push_back("-Wno-reorder");
	globalWarnings.push_back("-Wpointer-arith");
	globalWarnings.push_back("-Wcast-qual");
	globalWarnings.push_back("-Wcast-align");
	globalWarnings.push_back("-Wshadow");
	globalWarnings.push_back("-Wnon-virtual-dtor");
	globalWarnings.push_back("-Wwrite-strings");
	// The following are not warnings at all... We should consider adding them to
	// a different list of parameters.
#if !NEEDS_RTTI
	globalWarnings.push_back("-fno-rtti");
#endif
	globalWarnings.push_back("-fno-exceptions");
	globalWarnings.push_back("-fcheck-new");
}

/**
 * Compares two FSNode entries in a strict-weak fashion based on the name.
 *
 * @param left  The first operand.
 * @param right The second operand.
 * @return "true" when the name of the left operand is strictly smaller than
 *         the name of the second operand. "false" otherwise.
 */
bool compareFSNode(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right);

#ifdef FIRST_ENGINE
/**
 * Compares two FSNode entries in a strict-weak fashion based on engine name
 * order.
 *
 * @param left  The first operand.
 * @param right The second operand.
 * @return "true" when the name of the left operand is strictly smaller than
 *         the name of the second operand. "false" otherwise.
 */
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right);
#endif
} // End of anonymous namespace

namespace {

bool compareFSNode(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	return left.name < right.name;
}

#ifdef FIRST_ENGINE
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	if (left.name == FIRST_ENGINE) {
		return right.name != FIRST_ENGINE;
	} else if (right.name == FIRST_ENGINE) {
		return false;
	} else {
		return compareFSNode(left, right);
	}
}
#endif
} // End of anonymous namespace

TokenList tokenize(const std::string &input, char separator) {
	TokenList result;

	std::string::size_type sIdx = input.find_first_not_of(" \t");
	std::string::size_type nIdx = std::string::npos;

	if (sIdx == std::string::npos)
		return result;

	do {
		if (input.at(sIdx) == '\"') {
			++sIdx;
			nIdx = input.find_first_of('\"', sIdx);
		} else {
			nIdx = input.find_first_of(separator, sIdx);
		}

		if (nIdx != std::string::npos) {
			result.push_back(input.substr(sIdx, nIdx - sIdx));
			if (separator == ' ')
				sIdx = input.find_first_not_of(" \t", nIdx + 1);
			else
				sIdx = input.find_first_not_of(separator, nIdx + 1);
		} else {
			result.push_back(input.substr(sIdx));
			break;
		}
	} while (sIdx != std::string::npos);

	return result;
}


namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////

std::string convertPathToWin(const std::string &path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '/', '\\');
	return result;
}

std::string getIndent(const int indentation) {
	std::string result;
	for (int i = 0; i < indentation; ++i)
		result += '\t';
	return result;
}

void splitFilename(const std::string &fileName, std::string &name, std::string &ext) {
	const std::string::size_type dot = fileName.find_last_of('.');
	name = (dot == std::string::npos) ? fileName : fileName.substr(0, dot);
	ext = (dot == std::string::npos) ? std::string() : fileName.substr(dot + 1);
}

void splitPath(const std::string &path, std::string &dir, std::string &file) {
	const std::string::size_type sep = path.find_last_of('/');
	dir = (sep == std::string::npos) ? path : path.substr(0, sep);
	file = (sep == std::string::npos) ? std::string() : path.substr(sep + 1);
}

std::string basename(const std::string &fileName) {
	const std::string::size_type slash = fileName.find_last_of('/');
	if (slash == std::string::npos)
		return fileName;
	return fileName.substr(slash + 1);
}

std::string toString(int num) {
	std::ostringstream os;
	os << num;
	return os.str();
}

std::string toUpper(const std::string &str) {
	std::string res;
	std::transform(str.begin(), str.end(), std::back_inserter(res), toupper);
	return res;
}

FileList listDirectory(const std::string &dir) {
	FileList result;
#if defined(_WIN32) || defined(WIN32)
	WIN32_FIND_DATAA fileInformation;
	HANDLE fileHandle = FindFirstFileA((dir + "/*").c_str(), &fileInformation);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return result;

	do {
		if (fileInformation.cFileName[0] == '.')
			continue;

		result.push_back(FSNode(fileInformation.cFileName, (fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
	} while (FindNextFileA(fileHandle, &fileInformation) == TRUE);

	FindClose(fileHandle);
#else
	DIR *dirp = opendir(dir.c_str());
	struct dirent *dp = NULL;

	if (dirp == NULL)
		return result;

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.')
			continue;

		struct stat st;
		if (stat((dir + '/' + dp->d_name).c_str(), &st))
			continue;

		result.push_back(FSNode(dp->d_name, S_ISDIR(st.st_mode)));
	}

	closedir(dirp);
#endif
	return result;
}

void createDirectory(const std::string &dir) {
#if defined(_WIN32) || defined(WIN32)
	if (!CreateDirectoryA(dir.c_str(), nullptr)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			error("Could not create folder \"" + dir + "\"");
		}
	}
#else
	if (mkdir(dir.c_str(), 0777) == -1) {
		if (errno == EEXIST) {
			// Try to open as a folder (might be a file / symbolic link)
			DIR *dirp = opendir(dir.c_str());
			if (dirp == NULL) {
				error("Could not create folder \"" + dir + "\"");
			} else {
				// The folder exists, just close the stream and return
				closedir(dirp);
			}
		} else {
			error("Could not create folder \"" + dir + "\"");
		}
	}
#endif
}

} // namespace CreateProjectTool
void error(const std::string &message) {
	std::cerr << "ERROR: " << message << "!" << std::endl;
	std::exit(-1);
}
