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

#include <iostream>

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
};

inline
IncludeFinder::IncludeFinder(const clang::CompilerInstance &compiler)
    : compiler(compiler)
{
}

inline clang::PPCallbacks *
IncludeFinder::createPreprocessorCallbacks()
{
    return new CallbacksProxy(*this);
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
    if (compiler.getSourceManager().isInMainFile(hashLoc)) {
        std::cout << fileName.str() << std::endl;
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
}
