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

#include "path_utils.hpp"

#include <algorithm>
#include <string>

std::string
path_utils::extractTail(std::string path)
{
    // normalize slashes
    std::replace(path.begin(), path.end(), '\\', '/');

    // leave only trailing path component
    std::string::size_type tailPos = path.rfind('/');
    tailPos = (tailPos == std::string::npos) ? 0UL : (tailPos + 1);
    return std::string(path, tailPos);
}

std::pair<std::string, std::string>
path_utils::crackName(const std::string &name)
{
    const std::string::size_type dotPos = name.rfind('.');
    const std::string::size_type extPos = (dotPos == std::string::npos)
                                        ? 0UL
                                        : (dotPos + 1);

    return std::make_pair(std::string(name, 0UL, dotPos),
                          std::string(name, extPos));
}

std::string
path_utils::extractRoot(const std::string &name)
{
    return std::string(name, 0UL, name.rfind('.'));
}
