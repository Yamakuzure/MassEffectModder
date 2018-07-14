/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <QString>
#include <QList>

#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>

#include <Package.h>
#include <TextureProps.h>

class Texture
{
private:

    enum TextureEnums
    {
        textureTag = 0x9E2A83C1,
        maxBlockSize = 0x20000, // 128KB
        SizeOfChunkBlock = 8,
        SizeOfChunk = 16,
    };

    MemoryStream *textureData = nullptr;
    ByteBuffer mipMapData;
    ByteBuffer restOfData;
    QString packagePath;

public:

    enum StorageFlags
    {
        noFlags        = 0,
        externalFile   = 1 << 0,
        compressedZlib = 1 << 1,
        compressedLZO  = 1 << 4,
        unused         = 1 << 5,
    };

    enum StorageTypes
    {
        pccUnc = StorageFlags::noFlags,                                     // ME1 (Compressed PCC), ME2 (Compressed PCC)
        pccLZO = StorageFlags::compressedLZO,                               // ME1 (Uncompressed PCC)
        pccZlib = StorageFlags::compressedZlib,                             // ME1 (Uncompressed PCC)
        extUnc = StorageFlags::externalFile,                                // ME3 (DLC TFC archive)
        extLZO = StorageFlags::externalFile | StorageFlags::compressedLZO,   // ME1 (Reference to PCC), ME2 (TFC archive)
        extZlib = StorageFlags::externalFile | StorageFlags::compressedZlib, // ME3 (non-DLC TFC archive)
        empty = StorageFlags::externalFile | StorageFlags::unused,           // ME1, ME2, ME3
    };

    struct MipMap
    {
        StorageTypes storageType = StorageTypes::empty;
        int uncompressedSize{};
        int compressedSize{};
        uint dataOffset{};
        uint internalOffset{};
        int width{};
        int height{};
        ByteBuffer newData;
    };

    QList<MipMap> mipMapsList;
    TexProperty *properties;
    QString packageName;
    QString basePackageName;
    bool slave{};
    bool weakSlave{};

    Texture(Package &package, int exportId, ByteBuffer data, bool fixDim = true);
    ~Texture();
    void replaceMipMaps(QList<MipMap> &newMipMaps);
    const ByteBuffer compressTexture(ByteBuffer inputData, StorageTypes type, qint64 &compressedSize);
    const ByteBuffer decompressTexture(MemoryStream &stream, StorageTypes type, int uncompressedSize, int compressedSize);
    uint getCrcData(ByteBuffer data);
    uint getCrcMipmap(MipMap &mipmap);
    uint getCrcTopMipmap();
    const MipMap& getTopMipmap();
    bool existMipmap(int width, int height);
    const MipMap& getMipmap(int width, int height);
    bool hasImageData();
    const ByteBuffer getTopImageData();
    const ByteBuffer getMipMapDataByIndex(int index);
    const ByteBuffer getMipMapData(MipMap &mipmap);
    bool hasEmptyMips();
    int numNotEmptyMips();
    const ByteBuffer toArray(uint pccTextureDataOffset, bool updateOffset = true);
    void Dispose();
};

#endif
