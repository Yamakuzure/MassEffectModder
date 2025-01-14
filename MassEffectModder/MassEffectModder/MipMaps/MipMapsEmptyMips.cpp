/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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

#include <MipMaps/MipMaps.h>
#include <GameData/GameData.h>
#include <Texture/Texture.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>

void MipMaps::prepareListToRemove(QList<TextureMapEntry> &textures, QList<RemoveMipsEntry> &list, bool force)
{
#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    for (int k = 0; k < textures.count(); k++)
    {
#ifdef GUI
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
#endif
        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if ((textures[k].list[t].path.length() == 0) ||
                 textures[k].list[t].movieTexture)
            {
                continue;
            }
            if (textures[k].list[t].removeEmptyMips || force)
            {
                bool found = false;
                for (int e = 0; e < list.count(); e++)
                {
                    if (AsciiStringMatch(list[e].pkgPath, textures[k].list[t].path))
                    {
                        list[e].exportIDs.push_back(textures[k].list[t].exportID);
                        found = true;
                        break;
                    }
                }
                if (found)
                    continue;
                RemoveMipsEntry entry{};
                entry.pkgPath = textures[k].list[t].path;
                entry.exportIDs.push_back(textures[k].list[t].exportID);
                list.push_back(entry);
            }
        }
    }
}

void MipMaps::removeMipMaps(int phase, QList<TextureMapEntry> &textures, QStringList &pkgsToMarker,
                            QStringList &pkgsToRepack, bool repack, bool appendMarker, bool force,
                            ProgressCallback callback, void *callbackHandle)
{
    int lastProgress = -1;

    QList<RemoveMipsEntry> list;
    prepareListToRemove(textures, list, force);

    QString path;
    if (GameData::gameType == ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (GameData::gameType == ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    for (int i = 0; i < list.count(); i++)
    {
#ifdef GUI
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
#endif
        if (path.length() != 0 && AsciiStringMatchCaseIgnore(path, list[i].pkgPath))
            continue;

        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + list[i].pkgPath);
            ConsoleSync();
        }
        else
        {
            if (GameData::gameType == ME1_TYPE)
            {
                PINFO("Removing empty mipmaps (" + QString::number(phase) + ") " +
                             QString::number(i + 1) + "/" +
                             QString::number(list.count()) + " " + list[i].pkgPath + "\n");
            }
            else
            {
                PINFO("Removing empty mipmaps " +
                             QString::number(i + 1) + "/" +
                             QString::number(list.count()) + " " + list[i].pkgPath + "\n");
            }
        }

        int newProgress;
        if (GameData::gameType == ME1_TYPE)
            newProgress = (list.count() * (phase - 1) + i + 1) * 100 / (list.count() * 2);
        else
            newProgress = (i + 1) * 100 / list.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            else if (callback)
            {
                callback(callbackHandle, newProgress, "Removing empty mips");
            }
        }

        Package package{};
        if (package.Open(g_GameData->GamePath() + list[i].pkgPath) != 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + list[i].pkgPath + "\n");
                ConsoleSync();
            }
            else
            {
                QString err;
                err += "---- Start --------------------------------------------\n";
                err += "Issue opening package file: " + list[i].pkgPath + "\n";
                err += "---- End ----------------------------------------------\n\n";
                PERROR(err);
            }
            return;
        }

        removeMipMapsPerPackage(phase, textures, package, list[i],
                                pkgsToMarker, pkgsToRepack, repack, appendMarker);
    }
}

void MipMaps::removeMipMapsPerPackage(int phase, QList<TextureMapEntry> &textures, Package &package,
                                      RemoveMipsEntry &removeEntry, QStringList &pkgsToMarker,
                                      QStringList &pkgsToRepack, bool repack, bool appendMarker)
{
    for (int l = 0; l < removeEntry.exportIDs.count(); l++)
    {
        int exportID = removeEntry.exportIDs[l];
        Package::ExportEntry &exp = package.exportsTable[exportID];
        int id = package.getClassNameId(exp.getClassId());
        if (id == package.nameIdTextureMovie)
            continue;
        ByteBuffer exportData = package.getExportData(exportID);
        if (exportData.ptr() == nullptr)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Texture has broken export data in package: ") +
                             package.packagePath + "\nExport Id: " +
                             QString::number(exportID + 1) + "\nSkipping...");
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Error: Texture has broken export data in package: ") +
                       package.packagePath +"\nExport Id: " +
                       QString::number(exportID + 1) + "\nSkipping...\n");
            }
            continue;
        }
        Texture texture = Texture(package, exportID, exportData, false);
        exportData.Free();
        if (!texture.hasEmptyMips())
        {
            continue;
        }
        texture.removeEmptyMips();
        texture.getProperties().setIntValue("SizeX", texture.mipMapsList.first().width);
        texture.getProperties().setIntValue("SizeY", texture.mipMapsList.first().height);
        texture.getProperties().setIntValue("MipTailBaseIdx", texture.mipMapsList.count() - 1);

        TextureMapPackageEntry m;
        int foundListEntry = -1;
        int foundTextureEntry = -1;
        if (GameData::gameType == ME1_TYPE)
        {
            QString pkgName = package.packagePath.toLower();
            for (int k = 0; k < textures.count(); k++)
            {
                for (int t = 0; t < textures[k].list.count(); t++)
                {
                    if (textures[k].list[t].exportID == exportID &&
                        AsciiStringMatchCaseIgnore(textures[k].list[t].path, pkgName))
                    {
                        foundTextureEntry = k;
                        foundListEntry = t;
                        break;
                    }
                }
            }
            if (foundListEntry == -1)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + package.exportsTable[exportID].objectName +
                                 " not found in tree: " + removeEntry.pkgPath + ", skipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + package.exportsTable[exportID].objectName +
                                 " not found in package: " + removeEntry.pkgPath + ", skipping...\n");
                }
                continue;
            }

            m = textures[foundTextureEntry].list[foundListEntry];
            if (m.linkToMaster != -1)
            {
                if (phase == 1)
                {
                    continue;
                }

                const TextureMapPackageEntry& foundMasterTex = textures[foundTextureEntry].list[m.linkToMaster];
                if (texture.mipMapsList.count() != foundMasterTex.masterDataOffset.count())
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR Texture ") + package.exportsTable[exportID].objectName + " in package: " + foundMasterTex.path + " has wrong reference, skipping...");
                        ConsoleSync();
                    }
                    else
                    {
                        PERROR(QString("Error: Texture ") + package.exportsTable[exportID].objectName +
                               " in package: " + foundMasterTex.path + " has wrong reference, skipping...\n");
                    }
                    continue;
                }
                for (int t = 0; t < texture.mipMapsList.count(); t++)
                {
                    Texture::TextureMipMap mipmap = texture.mipMapsList[t];
                    if (mipmap.storageType == StorageTypes::extLZO ||
                        mipmap.storageType == StorageTypes::extZlib ||
                        mipmap.storageType == StorageTypes::extUnc)
                    {
                        mipmap.dataOffset = foundMasterTex.masterDataOffset[t];
                        texture.mipMapsList[t] = mipmap;
                    }
                }
            }
        }

        uint packageDataOffset;
        {
            MemoryStream newData;
            ByteBuffer buffer = texture.getProperties().toArray();
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            packageDataOffset = package.exportsTable[exportID].getDataOffset() + (uint)newData.Position();
            buffer = texture.toArray(packageDataOffset);
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = newData.ToArray();
            package.setExportData(exportID, buffer);
            buffer.Free();
        }

        if (GameData::gameType == ME1_TYPE)
        {
            if (m.linkToMaster == -1)
            {
                if (phase == 2)
                    CRASH();
                m.masterDataOffset.clear();
                for (int t = 0; t < texture.mipMapsList.count(); t++)
                {
                    m.masterDataOffset.push_back(packageDataOffset + texture.mipMapsList[t].internalOffset);
                }
            }

            m.removeEmptyMips = false;
            textures[foundTextureEntry].list[foundListEntry] = m;
        }
    }
    if (package.SaveToFile(repack, false, appendMarker))
    {
        if (repack)
            pkgsToRepack.removeOne(package.packagePath);
        pkgsToMarker.removeOne(package.packagePath);
    }
}
