/*
 * self-inc-first
 *
 * Copyright (C) 2014 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "IncludeFinderAction.hpp"

#include <clang/Frontend/CompilerInstance.h>

#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PPCallbacks.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "path_utils.hpp"

namespace
{

class CallbacksProxy : public clang::PPCallbacks
{
public:
    inline CallbacksProxy(clang::PPCallbacks &master);

public:
    virtual inline void InclusionDirective(clang::SourceLocation hashLoc,
                                           const clang::Token &includeTok,
                                           clang::StringRef fileName,
                                           bool isAngled,
                                           clang::CharSourceRange filenameRange,
                                           const clang::FileEntry *file,
                                           clang::StringRef searchPath,
                                           clang::StringRef relativePath,
                                           const clang::Module *imported);

private:
    clang::PPCallbacks &master;
};

inline
CallbacksProxy::CallbacksProxy(clang::PPCallbacks &master)
    : master(master)
{
}

inline void
CallbacksProxy::InclusionDirective(clang::SourceLocation hashLoc,
                                   const clang::Token &includeTok,
                                   clang::StringRef fileName,
                                   bool isAngled,
                                   clang::CharSourceRange filenameRange,
                                   const clang::FileEntry *file,
                                   clang::StringRef searchPath,
                                   clang::StringRef relativePath,
                                   const clang::Module *imported)
{
    master.InclusionDirective(hashLoc,
                              includeTok,
                              fileName,
                              isAngled,
                              filenameRange,
                              file,
                              searchPath,
                              relativePath,
                              imported);
}

class IncludeFinder : private clang::PPCallbacks
{
public:
    explicit inline IncludeFinder(const clang::CompilerInstance &compiler);

public:
    inline clang::PPCallbacks * createPreprocessorCallbacks();

    inline void diagnoseAndReport();

    virtual inline void InclusionDirective(clang::SourceLocation hashLoc,
                                           const clang::Token &includeTok,
                                           clang::StringRef fileName,
                                           bool isAngled,
                                           clang::CharSourceRange filenameRange,
                                           const clang::FileEntry *file,
                                           clang::StringRef searchPath,
                                           clang::StringRef relativePath,
                                           const clang::Module *imported);

private:
    const clang::CompilerInstance &compiler;
    std::string name;

    typedef std::pair<int, std::string> IncludeInfo;
    typedef std::vector<IncludeInfo> Includes;
    Includes includes;
};

inline
IncludeFinder::IncludeFinder(const clang::CompilerInstance &compiler)
    : compiler(compiler)
{
    const clang::FileID mainFile = compiler.getSourceManager().getMainFileID();
    name = compiler.getSourceManager().getFileEntryForID(mainFile)->getName();
}

inline clang::PPCallbacks *
IncludeFinder::createPreprocessorCallbacks()
{
    return new CallbacksProxy(*this);
}

typedef std::vector<std::string> KnownHdrExts;
static KnownHdrExts
getKnownHdrExts()
{
    KnownHdrExts knownHdrExts;
    knownHdrExts.push_back("h");
    knownHdrExts.push_back("H");
    knownHdrExts.push_back("hpp");
    knownHdrExts.push_back("HPP");
    knownHdrExts.push_back("hxx");
    knownHdrExts.push_back("HXX");
    return knownHdrExts;
}
static const KnownHdrExts KNOWN_HDR_EXTS = getKnownHdrExts();

template <typename C, typename T>
inline bool
contains(const C &c, const T &val)
{
    return std::find(c.begin(), c.end(), val) != c.end();
}

inline void
IncludeFinder::diagnoseAndReport()
{
    IncludeInfo selfInclude;

    const std::string &tail = path_utils::extractTail(name);
    const std::string &root = path_utils::extractRoot(tail);

    typedef Includes::iterator It;
    for (It it = includes.begin(); it != includes.end(); ++it) {
        const std::string &hdrPath = it->second;
        const std::string &hdrName = path_utils::extractTail(hdrPath);

        const std::pair<std::string, std::string> nameParts =
            path_utils::crackName(hdrName);

        // if root parts are the same and extension is one of known
        // TODO: maybe compare roots case insensitive
        if (nameParts.first == root) {
            if (contains(KNOWN_HDR_EXTS, nameParts.second)) {
                if (selfInclude.second.empty()) {
                    selfInclude = *it;
                } else {
                    // TODO: issue a warning about failed self include detection
                    std::cout << name
                              << ':'
                              << "ambiguous header name detection: "
                              << hdrPath
                              << std::endl;
                }
            } else {
                std::cout << name
                          << ':'
                          << "unexpected extension: "
                          << nameParts.second
                          << std::endl;
            }
        }
    }

    if (!selfInclude.second.empty() && !includes.empty()) {
        if (includes[0] != selfInclude) {
            std::cout << name
                      << ':'
                      << selfInclude.first
                      << ':'
                      << "should be the first include in the file"
                      << std::endl;
        }
    }
}

inline void
IncludeFinder::InclusionDirective(clang::SourceLocation hashLoc,
                                  const clang::Token &includeTok,
                                  clang::StringRef fileName,
                                  bool isAngled,
                                  clang::CharSourceRange filenameRange,
                                  const clang::FileEntry *file,
                                  clang::StringRef searchPath,
                                  clang::StringRef relativePath,
                                  const clang::Module *imported)
{
    clang::SourceManager &sm = compiler.getSourceManager();

    if (sm.isInMainFile(hashLoc)) {
        const unsigned int lineNum = sm.getSpellingLineNumber(hashLoc);
        includes.push_back(std::make_pair(lineNum, fileName));
    }
}

}

void
IncludeFinderAction::ExecuteAction()
{
    IncludeFinder includeFinder(getCompilerInstance());
    getCompilerInstance().getPreprocessor().addPPCallbacks(
        includeFinder.createPreprocessorCallbacks()
    );

    clang::PreprocessOnlyAction::ExecuteAction();

    includeFinder.diagnoseAndReport();
}
