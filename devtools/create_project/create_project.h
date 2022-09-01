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


class DefineList {
public:
	DefineList() {}
	DefineList(const DefineList &old) : defines(old.defines) {}

	void add(std::string name) {
		defines.emplace(name, std::string("1"));
	}
	void add(std::string name, std::string value) {
		defines.emplace(name, value);
	}
	void remove(std::string name) {
		defines.erase(name);
	}

	bool has(std::string name) const {
		return defines.count(name) > 0;
	}

	using const_iterator = std::map<std::string, std::string>::const_iterator;
	using iterator = std::map<std::string, std::string>::iterator;

	iterator begin() { return defines.begin(); }
	iterator end() { return defines.end(); }
	const_iterator cbegin() const { return defines.cbegin(); }
	const_iterator cend() const { return defines.cend(); }

	DefineList &operator+=(const DefineList &right) {
		defines.insert(right.defines.cbegin(), right.defines.cend());
		return *this;
	}

	friend DefineList operator+(const DefineList &left, const DefineList &right) {
		DefineList newList = left;
		newList.defines.insert(right.defines.cbegin(), right.defines.cend());
		return newList;
	}

private:
	std::map<std::string, std::string> defines;
};

/**
 * Converts the given path to only use slashes as
 * delimiters.
 * This means that for example the path:
 *  foo/bar\test.txt
 * will be converted to:
 *  foo/bar/test.txt
 *
 * @param path Path string.
 * @return Converted path.
 */
std::string unifyPath(const std::string &path);

/**
 * Removes trailing slash from path if it exists
 *
 * @param path Path string.
 */
void removeTrailingSlash(std::string &path);

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

extern std::map<std::string, bool> isEngineEnabled;

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
