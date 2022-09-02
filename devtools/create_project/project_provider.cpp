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
#include "build_setup.h"
#include "project_provider.h"
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

namespace CreateProjectTool {

bool producesObjectExtension(const std::string &ext) {
	return (ext == "cpp" || ext == "c" || ext == "asm" || ext == "m" || ext == "mm");
}

bool producesObjectFile(const std::string &fileName) {
	std::string n, ext;
	splitFilename(fileName, n, ext);
	return producesObjectExtension(ext);
}

/**
 * Checks whether the give file in the specified directory is present in the given
 * file list.
 *
 * This function does as special match against the file list.
 * By default object files (.o) are excluded, header files (.h) are included,
 * and it will not take file extensions into consideration, when the extension
 * of a file in the specified directory is one of "m", "cpp", "c" or "asm".
 *
 * @param dir Parent directory of the file.
 * @param fileName File name to match.
 * @param fileList List of files to match against.
 * @return "true" when the file is in the list, "false" otherwise.
 */
bool isInList(const std::string &dir, const std::string &fileName, const StringList &fileList) {
	if (fileList.empty())
		return false;

	std::string compareName, extensionName;
	splitFilename(fileName, compareName, extensionName);

	if (!extensionName.empty())
		compareName += '.';

	for (StringList::const_iterator i = fileList.begin(); i != fileList.end(); ++i) {

		// When no comparison name is given, we try to match whether a subset of
		// the given directory should be included. To do that we must assure that
		// the first character after the substring, having the same size as dir, must
		// be a path delimiter.
		if (compareName.empty()) {
			if (i->compare(0, dir.size(), dir))
				continue;
			if (i->size() >= dir.size() + 1 && i->at(dir.size()) == '/')
				return true;
			else
				continue;
		}

		std::string listDir, listFile;
		splitPath(*i, listDir, listFile);
		if (dir.compare(0, listDir.size(), listDir))
			continue;

		if (extensionName == "o") {
			return false;
		} else if (extensionName == "h") {
			return true;
		} else if (!producesObjectExtension(extensionName)) {
			if (fileName == listFile)
				return true;
		} else {
			if (!listFile.compare(0, compareName.size(), compareName))
				return true;
		}
	}

	return false;
}

/**
 * A strict weak compare predicate for sorting a list of
 * "FileNode *" entries.
 *
 * It will sort directory nodes before file nodes.
 *
 * @param l Left-hand operand.
 * @param r Right-hand operand.
 * @return "true" if and only if l should be sorted before r.
 */
bool compareNodes(const FileNode *l, const FileNode *r) {
	if (!l) {
		return false;
	} else if (!r) {
		return true;
	} else {
		if (l->children.empty() && !r->children.empty()) {
			return false;
		} else if (!l->children.empty() && r->children.empty()) {
			return true;
		} else {
			return l->name < r->name;
		}
	}
}

/**
 * Scans the specified directory against files, which should be included
 * in the project files. It will not include files present in the exclude list.
 *
 * @param dir Directory in which to search for files.
 * @param includeList Files to include in the project.
 * @param excludeList Files to exclude from the project.
 * @return Returns a file node for the specific directory.
 */
FileNode *scanFiles(const std::string &dir, const StringList &includeList, const StringList &excludeList) {
	FileList files = listDirectory(dir);

	if (files.empty())
		return nullptr;

	FileNode *result = new FileNode(dir);
	assert(result);

	for (FileList::const_iterator i = files.begin(); i != files.end(); ++i) {
		if (i->isDirectory) {
			const std::string subDirName = dir + '/' + i->name;
			if (!isInList(subDirName, std::string(), includeList))
				continue;

			FileNode *subDir = scanFiles(subDirName, includeList, excludeList);

			if (subDir) {
				subDir->name = i->name;
				result->children.push_back(subDir);
			}
			continue;
		}

		std::string name, ext;
		splitFilename(i->name, name, ext);

		if (ext != "h" && isInList(dir, i->name, excludeList))
			continue;

		if (!isInList(dir, i->name, includeList))
			continue;

		FileNode *child = new FileNode(i->name);
		assert(child);
		result->children.push_back(child);
	}

	if (result->children.empty()) {
		delete result;
		return nullptr;
	} else {
		result->children.sort(compareNodes);
		return result;
	}
}

//////////////////////////////////////////////////////////////////////////
// Project Provider methods
//////////////////////////////////////////////////////////////////////////
ProjectProvider::ProjectProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: _version(version), _globalWarnings(global_warnings), _projectWarnings(project_warnings) {
}

void ProjectProvider::createProject(BuildSetup &setup) {
	std::string targetFolder;

	if (setup.devTools) {
		_engineUuidMap = createToolsUUIDMap();
		targetFolder = "/devtools/";
	} else if (!setup.tests) {
		_engineUuidMap = createUUIDMap(setup);
		targetFolder = "/engines/";
	}

	_allProjUuidMap = _engineUuidMap;

	// We also need to add the UUID of the main project file.
	const std::string svmUUID = _allProjUuidMap[setup.projectName] = createUUID(setup.projectName);
	// Add the uuid of the detection project
	const std::string detProject = setup.projectName + "-detection";
	const std::string detUUID = createUUID(detProject);
	if (setup.useStaticDetection) {
		_allProjUuidMap[detProject] = _engineUuidMap[detProject] = detUUID;
	}

	createWorkspace(setup);

	StringList in, ex;

	// Create project files
	for (UUIDMap::const_iterator i = _engineUuidMap.begin(); i != _engineUuidMap.end(); ++i) {
		if (i->first == detProject)
			continue;
		// Retain the files between engines if we're creating a single project
		in.clear();
		ex.clear();

		const std::string moduleDir = setup.srcDir + targetFolder + i->first;

		createModuleList(moduleDir, setup.defines, setup.testDirs, in, ex);
		createProjectFile(i->first, i->second, setup, moduleDir, in, ex);
	}

	// Create engine-detection submodules.
	if (setup.useStaticDetection) {
		in.clear();
		ex.clear();
		std::vector<std::string> detectionModuleDirs;
		detectionModuleDirs.reserve(setup.engines.size());
		bool detectAllEngines = setup.featureEnabled("detection-full");

		for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
			// We ignore all sub engines here because they require no special handling.
			if (isSubEngine(i->name, setup.engines)) {
				continue;
			}
			// If we're not detecting all engines then ignore the disabled ones
			if (!(detectAllEngines || i->enable)) {
				continue;
			}
			detectionModuleDirs.push_back(setup.srcDir + "/engines/" + i->name);
		}

		for (std::vector<std::string>::const_iterator i = detectionModuleDirs.begin(), end = detectionModuleDirs.end(); i != end; ++i) {
			createModuleList(*i, setup.defines, setup.testDirs, in, ex, true);
		}

		createProjectFile(detProject, detUUID, setup, setup.srcDir + "/engines", in, ex);
	}

	if (!setup.devTools) {
		// Last but not least create the main project file.
		in.clear();
		ex.clear();
		// File list for the Project file
		createModuleList(setup.srcDir + "/backends", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/backends/platform/sdl", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/base", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/common", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/engines", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/graphics", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/gui", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/audio", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/audio/softsynth/mt32", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/video", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/image", setup.defines, setup.testDirs, in, ex);
		createModuleList(setup.srcDir + "/math", setup.defines, setup.testDirs, in, ex);
		if (setup.tests) {
			createModuleList(setup.srcDir + "/test", setup.defines, setup.testDirs, in, ex);
		} else {
			// Resource files
			addResourceFiles(setup, in, ex);

			// Various text files
			in.push_back(setup.srcDir + "/AUTHORS");
			in.push_back(setup.srcDir + "/COPYING");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.BSD");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.LGPL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.FREEFONT");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.OFL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.ISC");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.LUA");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.MIT");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.TINYGL");
			in.push_back(setup.srcDir + "/LICENSES/COPYING.GLAD");
			in.push_back(setup.srcDir + "/COPYRIGHT");
			in.push_back(setup.srcDir + "/NEWS.md");
			in.push_back(setup.srcDir + "/README.md");
		}

		// Create the main project file.
		createProjectFile(setup.projectName, svmUUID, setup, setup.srcDir, in, ex);
	}

	// Create other misc. build files
	createOtherBuildFiles(setup);

	// In case we create the main ScummVM project files we will need to
	// generate engines/plugins_table.h & engines/detection_table.h
	if (!setup.tests && !setup.devTools) {
		createEnginePluginsTable(setup);
	}
}

ProjectProvider::UUIDMap ProjectProvider::createUUIDMap(const BuildSetup &setup) const {
	UUIDMap result;

	for (EngineDescList::const_iterator i = setup.engines.begin(); i != setup.engines.end(); ++i) {
		if (!i->enable || isSubEngine(i->name, setup.engines))
			continue;

		result[i->name] = createUUID(i->name);
	}

	return result;
}

ProjectProvider::UUIDMap ProjectProvider::createToolsUUIDMap() const {
	UUIDMap result;

	ToolList tools = getAllTools();
	for (ToolList::const_iterator i = tools.begin(); i != tools.end(); ++i) {
		if (!i->enable)
			continue;

		result[i->name] = createUUID(i->name);
	}

	return result;
}

const int kUUIDLen = 16;

std::string ProjectProvider::createUUID() const {
#ifdef USE_WIN32_API
	UUID uuid;
	RPC_STATUS status = UuidCreateSequential(&uuid);
	if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
		error("UuidCreateSequential failed");

	unsigned char *string = 0;
	if (UuidToStringA(&uuid, &string) != RPC_S_OK)
		error("UuidToStringA failed");

	std::string result = std::string((char *)string);
	std::transform(result.begin(), result.end(), result.begin(), toupper);
	RpcStringFreeA(&string);
	return result;
#else
	unsigned char uuid[kUUIDLen];

	for (int i = 0; i < kUUIDLen; ++i)
		uuid[i] = (unsigned char)((std::rand() / (double)(RAND_MAX)) * 0xFF);

	uuid[8] &= 0xBF;
	uuid[8] |= 0x80;
	uuid[6] &= 0x4F;
	uuid[6] |= 0x40;

	return UUIDToString(uuid);
#endif
}

std::string ProjectProvider::createUUID(const std::string &name) const {
#ifdef USE_WIN32_API
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		error("CryptAcquireContext failed");
	}

	// Use MD5 hashing algorithm
	HCRYPTHASH hHash = NULL;
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		error("CryptCreateHash failed");
	}

	// Hash unique ScummVM namespace {5f5b43e8-35ff-4f1e-ad7e-a2a87e9b5254}
	const BYTE uuidNs[kUUIDLen] =
	    {0x5f, 0x5b, 0x43, 0xe8, 0x35, 0xff, 0x4f, 0x1e, 0xad, 0x7e, 0xa2, 0xa8, 0x7e, 0x9b, 0x52, 0x54};
	if (!CryptHashData(hHash, uuidNs, kUUIDLen, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptHashData failed");
	}

	// Hash project name
	if (!CryptHashData(hHash, (const BYTE *)name.c_str(), name.length(), 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptHashData failed");
	}

	// Get resulting UUID
	BYTE uuid[kUUIDLen];
	DWORD len = kUUIDLen;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, uuid, &len, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		error("CryptGetHashParam failed");
	}

	// Add version and variant
	uuid[6] &= 0x0F;
	uuid[6] |= 0x30;
	uuid[8] &= 0x3F;
	uuid[8] |= 0x80;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return UUIDToString(uuid);
#else
	// Fallback to random UUID
	return createUUID();
#endif
}

std::string ProjectProvider::UUIDToString(unsigned char *uuid) const {
	std::stringstream uuidString;
	uuidString << std::hex << std::uppercase << std::setfill('0');
	for (int i = 0; i < kUUIDLen; ++i) {
		uuidString << std::setw(2) << (int)uuid[i];
		if (i == 3 || i == 5 || i == 7 || i == 9) {
			uuidString << std::setw(0) << '-';
		}
	}
	return uuidString.str();
}

std::string ProjectProvider::getLastPathComponent(const std::string &path) {
	std::string::size_type pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	else
		return path.substr(pos + 1);
}

void ProjectProvider::addFilesToProject(const std::string &dir, std::ostream &projectFile,
										const StringList &includeList, const StringList &excludeList,
										const std::string &filePrefix) {
	FileNode *files = scanFiles(dir, includeList, excludeList);

	writeFileListToProject(*files, projectFile, 0, std::string(), filePrefix + '/');

	delete files;
}

void ProjectProvider::createModuleList(const std::string &moduleDir, const DefineList &defines, StringList &testDirs, StringList &includeList, StringList &excludeList, bool forDetection) const {
	const std::string moduleMkFile = moduleDir + "/module.mk";
	std::ifstream moduleMk(moduleMkFile.c_str());
	if (!moduleMk)
		error(moduleMkFile + " is not present");

	includeList.push_back(moduleMkFile);

	std::stack<bool> shouldInclude;
	shouldInclude.push(true);

	StringList filesInVariableList;
	std::string moduleRootDir;

	bool hadModule = false;
	std::string line;
	for (;;) {
		std::getline(moduleMk, line);

		if (moduleMk.eof())
			break;

		if (moduleMk.fail())
			error("Failed while reading from " + moduleMkFile);

		TokenList tokens = tokenize(line);
		if (tokens.empty())
			continue;

		TokenList::const_iterator i = tokens.begin();
		if (*i == "MODULE") {
			if (hadModule)
				error("More than one MODULE definition in " + moduleMkFile);
			// Format: "MODULE := path/to/module"
			if (tokens.size() < 3)
				error("Malformed MODULE definition in " + moduleMkFile);
			++i;
			if (*i != ":=")
				error("Malformed MODULE definition in " + moduleMkFile);
			++i;

			std::string moduleRoot = unifyPath(*i);
			if (moduleDir.compare(moduleDir.size() - moduleRoot.size(), moduleRoot.size(), moduleRoot))
				error("MODULE root " + moduleRoot + " does not match base dir " + moduleDir);

			hadModule = true;
			if (forDetection) {
				moduleRootDir = moduleRoot;
				break;
			}
		} else if (*i == "MODULE_OBJS") {
			if (tokens.size() < 3)
				error("Malformed MODULE_OBJS definition in " + moduleMkFile);
			++i;

			// This is not exactly correct, for example an ":=" would usually overwrite
			// all already added files, but since we do only save the files inside
			// includeList or excludeList currently, we couldn't handle such a case easily.
			// (includeList and excludeList should always preserve their entries, not added
			// by this function, thus we can't just clear them on ":=" or "=").
			// But hopefully our module.mk files will never do such things anyway.
			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed MODULE_OBJS definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else if (*i == "$(KYRARPG_COMMON_OBJ)") {
					// HACK to fix EOB/LOL compilation in the kyra engine:
					// replace the variable name with the stored files.
					// This assumes that the file list has already been defined.
					if (filesInVariableList.size() == 0)
						error("$(KYRARPG_COMMON_OBJ) found, but the variable hasn't been set before it");
					// Construct file list and replace the variable
					for (StringList::iterator j = filesInVariableList.begin(); j != filesInVariableList.end(); ++j) {
						const std::string filename = *j;

						if (shouldInclude.top()) {
							// In case we should include a file, we need to make
							// sure it is not in the exclude list already. If it
							// is we just drop it from the exclude list.
							excludeList.remove(filename);

							includeList.push_back(filename);
						} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
							// We only add the file to the exclude list in case it
							// has not yet been added to the include list.
							excludeList.push_back(filename);
						}
					}
					++i;
				} else {
					const std::string filename = moduleDir + "/" + unifyPath(*i);

					if (shouldInclude.top()) {
						// In case we should include a file, we need to make
						// sure it is not in the exclude list already. If it
						// is we just drop it from the exclude list.
						excludeList.remove(filename);

						includeList.push_back(filename);
					} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
						// We only add the file to the exclude list in case it
						// has not yet been added to the include list.
						excludeList.push_back(filename);
					}
					++i;
				}
			}
		} else if (*i == "KYRARPG_COMMON_OBJ") {
			// HACK to fix EOB/LOL compilation in the kyra engine: add the
			// files defined in the KYRARPG_COMMON_OBJ variable in a list
			if (tokens.size() < 3)
				error("Malformed KYRARPG_COMMON_OBJ definition in " + moduleMkFile);
			++i;

			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed KYRARPG_COMMON_OBJ definition in " + moduleMkFile);

			++i;

			while (i != tokens.end()) {
				if (*i == "\\") {
					std::getline(moduleMk, line);
					tokens = tokenize(line);
					i = tokens.begin();
				} else {
					const std::string filename = moduleDir + "/" + unifyPath(*i);
					filesInVariableList.push_back(filename);
					++i;
				}
			}
		} else if (*i == "TESTS") {
			if (tokens.size() < 3)
				error("Malformed TESTS definition in " + moduleMkFile);
			++i;

			if (*i != ":=" && *i != "+=" && *i != "=")
				error("Malformed TESTS definition in " + moduleMkFile);
			++i;

			while (i != tokens.end()) {
				// Read input
				std::string folder = unifyPath(*i);

				// Get include folder
				const std::string source_dir = "$(srcdir)/";
				const std::string selector = getLastPathComponent(folder);
				const std::string module = getLastPathComponent(moduleDir);

				folder.replace(folder.find(source_dir), source_dir.length(), "");
				folder.replace(folder.find(selector), selector.length(), "");
				folder.replace(folder.find(module), module.length(), moduleDir);

				// Scan all files in the include folder
				FileList files = listDirectory(folder);

				// Add to list of test folders
				testDirs.push_back(folder);

				for (FileList::const_iterator f = files.begin(); f != files.end(); ++f) {
					if (f->isDirectory)
						continue;

					std::string filename = folder + f->name;

					if (shouldInclude.top()) {
						// In case we should include a file, we need to make
						// sure it is not in the exclude list already. If it
						// is we just drop it from the exclude list.
						excludeList.remove(filename);

						includeList.push_back(filename);
					} else if (std::find(includeList.begin(), includeList.end(), filename) == includeList.end()) {
						// We only add the file to the exclude list in case it
						// has not yet been added to the include list.
						excludeList.push_back(filename);
					}
				}

				++i;
			}
		} else if (*i == "ifdef") {
			if (tokens.size() < 2)
				error("Malformed ifdef in " + moduleMkFile);
			++i;

			if (!defines.has(*i))
				shouldInclude.push(false);
			else
				shouldInclude.push(true && shouldInclude.top());
		} else if (*i == "ifndef") {
			if (tokens.size() < 2)
				error("Malformed ifndef in " + moduleMkFile);
			++i;

			if (!defines.has(*i))
				shouldInclude.push(true && shouldInclude.top());
			else
				shouldInclude.push(false);
		} else if (*i == "else") {
			bool last = shouldInclude.top();
			shouldInclude.pop();
			shouldInclude.push(!last && shouldInclude.top());
		} else if (*i == "endif") {
			if (shouldInclude.size() <= 1)
				error("endif without ifdef found in " + moduleMkFile);
			shouldInclude.pop();
		} else if (*i == "elif") {
			error("Unsupported operation 'elif' in " + moduleMkFile);
		} else if (*i == "ifeq" || *i == "ifneq") {
			//XXX
			shouldInclude.push(false);
		}
	}

	if (forDetection) {
		std::string::size_type p = moduleRootDir.find('/');
		std::string engineName = moduleRootDir.substr(p + 1);
		std::string engineNameUpper = toUpper(engineName);

		for (;;) {
			std::getline(moduleMk, line);

			if (moduleMk.eof())
				break;

			if (moduleMk.fail())
				error("Failed while reading from " + moduleMkFile);

			TokenList tokens = tokenize(line);
			if (tokens.empty())
				continue;

			TokenList::const_iterator i = tokens.begin();

			if (*i != "DETECT_OBJS" && *i != "ifneq") {
				continue;
			}

			if (*i == "ifneq") {
				++i;
				if (*i != ("($(ENABLE_" + engineNameUpper + "),")) {
					continue;
				}

				// If the engine is already enabled, skip the additional
				// dependencies for detection objects.
				if (isEngineEnabled[engineName]) {
					bool breakEarly = false;
					while (true) {
						std::getline(moduleMk, line);
						if (moduleMk.eof()) {
							error("Unexpected EOF found, while parsing for " + engineName + " engine's module file.");
						} else if (line != "endif") {
							continue;
						} else {
							breakEarly = true;
							break;
						}
					}
					if (breakEarly) {
						break;
					}
				}

				while (*i != "DETECT_OBJS") {
					std::getline(moduleMk, line);
					if (moduleMk.eof()) {
						break;
					}

					tokens = tokenize(line);

					if (tokens.empty())
						continue;
					i = tokens.begin();
				}
			}


			if (tokens.size() < 3)
				error("Malformed DETECT_OBJS definition in " + moduleMkFile);
			++i;

			if (*i != "+=")
				error("Malformed DETECT_OBJS definition in " + moduleMkFile);

			++i;

			p = (*i).find('/');
			const std::string filename = moduleDir + "/" + (*i).substr(p + 1);

			includeList.push_back(filename);
		}
	}
	if (shouldInclude.size() != 1)
		error("Malformed file " + moduleMkFile);
}

void ProjectProvider::createEnginePluginsTable(const BuildSetup &setup) {
	// First we need to create the "engines" directory.
	createDirectory(setup.outputDir + "/engines");

	// Then, we can generate the actual "plugins_table.h" & "detection_table.h" file.
	const std::string enginePluginsTableFile = setup.outputDir + "/engines/plugins_table.h";
	const std::string detectionTableFile = setup.outputDir + "/engines/detection_table.h";

	std::ofstream enginePluginsTable(enginePluginsTableFile.c_str());
	std::ofstream detectionTable(detectionTableFile.c_str());

	if (!enginePluginsTable) {
		error("Could not open \"" + enginePluginsTableFile + "\" for writing");
	}

	if (!detectionTable) {
		error("Could not open \"" + detectionTableFile + "\" for writing");
	}

	enginePluginsTable << "/* This file is automatically generated by create_project */\n"
	                   << "/* DO NOT EDIT MANUALLY */\n"
	                   << "// This file is being included by \"base/plugins.cpp\"\n";

	detectionTable	   << "/* This file is automatically generated by create_project */\n"
	                   << "/* DO NOT EDIT MANUALLY */\n"
	                   << "// This file is being included by \"base/plugins.cpp\"\n";

	for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
		// We ignore all sub engines here because they require no special
		// handling.
		if (isSubEngine(i->name, setup.engines)) {
			continue;
		}

		// Make the engine name all uppercase.
		const std::string engineName = toUpper(i->name);

		enginePluginsTable << "#if PLUGIN_ENABLED_STATIC(" << engineName << ")\n"
		                   << "LINK_PLUGIN(" << engineName << ")\n"
		                   << "#endif\n";

		detectionTable << "#if defined(ENABLE_" << engineName << ") || defined(DETECTION_FULL)\n"
					   << "LINK_PLUGIN(" << engineName << "_DETECTION)\n"
					   << "#endif\n";
	}
}
} // namespace CreateProjectTool
