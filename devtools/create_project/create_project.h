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

#ifndef TOOLS_CREATE_PROJECT_H
#define TOOLS_CREATE_PROJECT_H

#ifndef __has_feature      // Optional of course.
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif

#if __cplusplus < 201103L && (!defined(_MSC_VER) || _MSC_VER < 1700)
#define override           // Compatibility with non-C++11 compilers.
#endif

#include <list>
#include <map>
#include <map>
#include <string>
#include <vector>

#include <cassert>

typedef std::list<std::string> StringList;

typedef StringList TokenList;

/**
 * Takes a given input line and creates a list of tokens out of it.
 *
 * A token in this context is separated by whitespaces. A special case
 * are quotation marks though. A string inside quotation marks is treated
 * as single token, even when it contains whitespaces.
 *
 * Thus for example the input:
 * foo bar "1 2 3 4" ScummVM
 * will create a list with the following entries:
 * "foo", "bar", "1 2 3 4", "ScummVM"
 * As you can see the quotation marks will get *removed* too.
 *
 * You can also use this with non-whitespace by passing another separator
 * character (e.g. ',').
 *
 * @param input The text to be tokenized.
 * @param separator The token separator.
 * @return A list of tokens.
 */
TokenList tokenize(const std::string &input, char separator = ' ');

/**
 * Quits the program with the specified error message.
 *
 * @param message The error message to print to stderr.
 */
#if defined(__GNUC__)
#define NORETURN_POST __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#define NORETURN_PRE __declspec(noreturn)
#endif

#ifndef NORETURN_PRE
#define NORETURN_PRE
#endif

#ifndef NORETURN_POST
#define NORETURN_POST
#endif
void NORETURN_PRE error(const std::string &message) NORETURN_POST;

/**
 * Structure to describe a Visual Studio version specification.
 *
 * This includes various generation details for MSVC projects,
 * as well as describe the versions supported.
 */
struct MSVCVersion {
	int version;                 ///< Version number passed as parameter.
	const char *name;            ///< Full program name.
	const char *solutionFormat;  ///< Format used for solution files.
	const char *solutionVersion; ///< Version number used in solution files.
	const char *project;         ///< Version number used in project files.
	const char *toolsetMSVC;     ///< Toolset version for MSVC compiler.
	const char *toolsetLLVM;     ///< Toolset version for Clang/LLVM compiler.
};
typedef std::list<MSVCVersion> MSVCList;

enum MSVC_Architecture {
	ARCH_ARM64,
	ARCH_X86,
	ARCH_AMD64
};

std::string getMSVCArchName(MSVC_Architecture arch);
std::string getMSVCConfigName(MSVC_Architecture arch);

/**
 * Creates a list of all supported versions of Visual Studio.
 *
 * @return A list including all versions available.
 */
MSVCList getAllMSVCVersions();

/**
 * Returns the definitions for a specific Visual Studio version.
 *
 * @param version The requested version.
 * @return The version information, or NULL if the version isn't supported.
 */
const MSVCVersion *getMSVCVersion(int version);

/**
 * Auto-detects the latest version of Visual Studio installed.
 *
 * @return Version number, or 0 if no installations were found.
 */
int getInstalledMSVC();

namespace CreateProjectTool {

/**
 * Structure for describing an FSNode. This is a very minimalistic
 * description, which includes everything we need.
 * It only contains the name of the node and whether it is a directory
 * or not.
 */
struct FSNode {
	FSNode() : name(), isDirectory(false) {}
	FSNode(const std::string &n, bool iD) : name(n), isDirectory(iD) {}

	std::string name; ///< Name of the file system node
	bool isDirectory; ///< Whether it is a directory or not
};

typedef std::list<FSNode> FileList;

/**
 * Gets a proper sequence of \t characters for the given
 * indentation level.
 *
 * For example with an indentation level of 2 this will
 * produce:
 *  \t\t
 *
 * @param indentation The indentation level
 * @return Sequence of \t characters.
 */
std::string getIndent(const int indentation);

/**
 * Converts the given path to only use backslashes.
 * This means that for example the path:
 *  foo/bar\test.txt
 * will be converted to:
 *  foo\bar\test.txt
 *
 * @param path Path string.
 * @return Converted path.
 */
std::string convertPathToWin(const std::string &path);

/**
 * Splits a file name into name and extension.
 * The file name must be only the filename, no
 * additional path name.
 *
 * @param fileName Filename to split
 * @param name Reference to a string, where to store the name.
 * @param ext Reference to a string, where to store the extension.
 */
void splitFilename(const std::string &fileName, std::string &name, std::string &ext);

/**
 * Splits a full path into directory and filename.
 * This assumes the last part is the filename, even if it
 * has no extension.
 *
 * @param path Path to split
 * @param name Reference to a string, where to store the directory part.
 * @param ext Reference to a string, where to store the filename part.
 */
void splitPath(const std::string &path, std::string &dir, std::string &file);

/**
 * Returns the basename of a path.
 * examples:
 *   a/b/c/d.ext -> d.ext
 *   d.ext       -> d.ext
 *
 * @param fileName Filename
 * @return The basename
 */
std::string basename(const std::string &fileName);

/**
 * Checks whether the given file will produce an object file or not.
 *
 * @param fileName Name of the file.
 * @return "true" when it will produce a file, "false" otherwise.
 */
bool producesObjectFile(const std::string &fileName);

/**
* Convert an integer to string
*
* @param num the integer to convert
* @return string representation of the number
*/
std::string toString(int num);

/**
* Convert a string to uppercase
*
* @param str the source string
* @return The string transformed to uppercase
*/
std::string toUpper(const std::string &str);

/**
 * Returns a list of all files and directories in the specified
 * path.
 *
 * @param dir Directory which should be listed.
 * @return List of all children.
 */
FileList listDirectory(const std::string &dir);

/**
 * Create a directory at the given path.
 *
 * @param dir The path to create.
 */
void createDirectory(const std::string &dir);

/**
 * Structure representing a file tree. This contains two
 * members: name and children. "name" holds the name of
 * the node. "children" does contain all the node's children.
 * When the list "children" is empty, the node is a file entry,
 * otherwise it's a directory.
 */
struct FileNode {
	typedef std::list<FileNode *> NodeList;

	explicit FileNode(const std::string &n) : name(n), children() {}

	~FileNode() {
		for (NodeList::iterator i = children.begin(); i != children.end(); ++i)
			delete *i;
	}

	std::string name;  ///< Name of the node
	NodeList children; ///< List of children for the node
};


} // namespace CreateProjectTool

#endif // TOOLS_CREATE_PROJECT_H
