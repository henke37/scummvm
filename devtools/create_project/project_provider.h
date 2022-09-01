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

#ifndef TOOLS_PROJECT_PROVIDER_H
#define TOOLS_PROJECT_PROVIDER_H

#include <list>
#include <map>
#include <map>
#include <string>
#include <vector>

#include <cassert>

#include "create_project.h"

struct BuildSetup;

namespace CreateProjectTool {

class ProjectProvider {
public:
	typedef std::map<std::string, std::string> UUIDMap;

	/**
	 * Instantiate new ProjectProvider class
	 *
	 * @param global_warnings List of warnings that apply to all projects
	 * @param project_warnings List of project-specific warnings
	 * @param version Target project version.
	 */
	ProjectProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version = 0);
	virtual ~ProjectProvider() {}

	/**
	 * Creates all build files
	 *
	 * @param setup Description of the desired build setup.
	 */
	void createProject(BuildSetup &setup);

	/**
	 * Returns the last path component.
	 *
	 * @param path Path string.
	 * @return Last path component.
	 */
	static std::string getLastPathComponent(const std::string &path);

protected:
	const int _version;                                  ///< Target project version
	StringList &_globalWarnings;                         ///< Global warnings
	std::map<std::string, StringList> &_projectWarnings; ///< Per-project warnings

	UUIDMap _engineUuidMap; ///< List of (project name, UUID) pairs
	UUIDMap _allProjUuidMap;

	/**
	 *  Create workspace/solution file
	 *
	 * @param setup Description of the desired build setup.
	 */
	virtual void createWorkspace(const BuildSetup &setup) = 0;

	/**
	 *  Create other files (such as build properties)
	 *
	 * @param setup Description of the desired build setup.
	 */
	virtual void createOtherBuildFiles(const BuildSetup &setup) = 0;

	/**
	 *  Add resources to the project
	 *
	 * @param setup Description of the desired build setup.
	 */
	virtual void addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) = 0;

	/**
	 * Create a project file for the specified list of files.
	 *
	 * @param name Name of the project file.
	 * @param uuid UUID of the project file.
	 * @param setup Description of the desired build.
	 * @param moduleDir Path to the module.
	 * @param includeList Files to include (must have "moduleDir" as prefix).
	 * @param excludeList Files to exclude (must have "moduleDir" as prefix).
	 */
	virtual void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                               const StringList &includeList, const StringList &excludeList) = 0;

	/**
	 * Writes file entries for the specified directory node into
	 * the given project file.
	 *
	 * @param dir Directory node.
	 * @param projectFile File stream to write to.
	 * @param indentation Indentation level to use.
	 * @param objPrefix Prefix to use for object files, which would name clash.
	 * @param filePrefix Generic prefix to all files of the node.
	 */
	virtual void writeFileListToProject(const FileNode &dir, std::ostream &projectFile, const int indentation,
	                                    const std::string &objPrefix, const std::string &filePrefix) = 0;

	/**
	 * Get the file extension for project files
	 */
	virtual const char *getProjectExtension() { return ""; }

	/**
	 * Adds files of the specified directory recursively to given project file.
	 *
	 * @param dir Path to the directory.
	 * @param projectFile Output stream object, where all data should be written to.
	 * @param includeList Files to include (must have a relative directory as prefix).
	 * @param excludeList Files to exclude (must have a relative directory as prefix).
	 * @param filePrefix Prefix to use for relative path arguments.
	 */
	void addFilesToProject(const std::string &dir, std::ostream &projectFile,
	                       const StringList &includeList, const StringList &excludeList,
	                       const std::string &filePrefix);

	/**
	 * Creates a list of files of the specified module. This also
	 * creates a list of files, which should not be included.
	 * All filenames will have "moduleDir" as prefix.
	 *
	 * @param moduleDir Path to the module.
	 * @param defines List of set defines.
	 * @param testDirs List of folders containing tests.
	 * @param includeList Reference to a list, where included files should be added.
	 * @param excludeList Reference to a list, where excluded files should be added.
	 */
	void createModuleList(const std::string &moduleDir, const DefineList &defines, StringList &testDirs, StringList &includeList, StringList &excludeList, bool forDetection = false) const;

	/**
	 * Creates an UUID for every enabled engine of the
	 * passed build description.
	 *
	 * @param setup Description of the desired build.
	 * @return A map, which includes UUIDs for all enabled engines.
	 */
	UUIDMap createUUIDMap(const BuildSetup &setup) const;

	/**
	 * Creates an UUID for every enabled tool of the
	 * passed build description.
	 *
	 * @return A map, which includes UUIDs for all enabled tools.
	 */
	UUIDMap createToolsUUIDMap() const;

	/**
	 * Creates an UUID and returns it in string representation.
	 *
	 * @return A new UUID as string.
	 */
	std::string createUUID() const;

	/**
	 * Creates a name-based UUID and returns it in string representation.
	 *
	 * @param name Unique name to hash.
	 * @return A new UUID as string.
	 */
	std::string createUUID(const std::string &name) const;

private:
	/**
	 * Returns the string representation of an existing UUID.
	 *
	 * @param uuid 128-bit array.
	 * @return Existing UUID as string.
	 */
	std::string UUIDToString(unsigned char *uuid) const;

	/**
	 * This creates the engines/plugins_table.h file required for building
	 * ScummVM.
	 *
	 * @param setup Description of the desired build.
	 */
	void createEnginePluginsTable(const BuildSetup &setup);
};

} // namespace CreateProjectTool

#endif // TOOLS_PROJECT_PROVIDER_H
