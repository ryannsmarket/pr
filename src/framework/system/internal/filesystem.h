//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_SYSTEM_FILESYSTEM_H
#define MU_SYSTEM_FILESYSTEM_H

#include "../ifilesystem.h"

namespace mu::system {
class FileSystem : public IFileSystem
{
public:
    Ret exists(const io::path& path) const override;
    Ret remove(const io::path& path) const override;

    Ret makePath(const io::path& path) const override;

    RetVal<io::paths> scanFiles(const io::path& rootDir, const QStringList& filters,
                                ScanMode mode = ScanMode::IncludeSubdirs) const override;

    RetVal<QByteArray> readFile(const io::path& filePath) const override;

private:
    Ret removeFile(const QString& path) const;
    Ret removeDir(const QString& path) const;
};
}

#endif // MU_SYSTEM_FILESYSTEM_H
