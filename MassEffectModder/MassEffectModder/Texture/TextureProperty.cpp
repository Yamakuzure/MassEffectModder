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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>
#include <Texture/TextureProperty.h>
#include <Program/ConfigIni.h>
#include <GameData/GameData.h>
#include <Types/MemTypes.h>

TextureProperty::TextureProperty(Package &pkg, const ByteBuffer &data)
{
    package = &pkg;
    headerData = *reinterpret_cast<quint32 *>(data.ptr());
    getProperty(data.ptr(), 4);
}

TextureProperty::~TextureProperty()
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        texPropertyList[i].valueRaw.Free();
        texPropertyList[i].valueStruct.Free();
    }
}

void TextureProperty::getProperty(quint8 *data, int offset)
{
    TexturePropertyEntry texProperty{};
    int size, valueRawPos, nextOffset;

    texProperty.name = package->getName(*reinterpret_cast<quint32 *>(data + offset));
    if (texProperty.name == "None")
    {
        nextOffset = offset;
        propertyEndOffset = valueRawPos = offset + 8;
        size = 0;
    }
    else
    {
        texProperty.type = package->getName(*reinterpret_cast<qint32 *>(data + offset + 8));
        size = *reinterpret_cast<qint32 *>(data + offset + 16);
        texProperty.index = *reinterpret_cast<qint32 *>(data + offset + 20);

        valueRawPos = offset + 24;

        if (texProperty.type == "IntProperty" ||
            texProperty.type == "StrProperty" ||
            texProperty.type == "FloatProperty" ||
            texProperty.type == "NameProperty" ||
            texProperty.type == "ObjectProperty")
        {
        }
        else if (texProperty.type == "StructProperty")
        {
            size += 8;
        }
        else if (texProperty.type == "ByteProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size += 8;
        }
        else if (texProperty.type == "BoolProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size = 1;
            else
                size = 4;
        }
        else {
            std::string err_str( "Unknown texture property type: \"" + texProperty.type.toStdString() + "\"" );
            qulonglong data_p = (qulonglong)(data);

            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_INVALID_PROPERTY %1").arg(texProperty.name));
                ConsoleWrite(QString("[IPC]ERROR_UNKNOWN_TYPE %1").arg(texProperty.type));
                ConsoleWrite(QString("[IPC]DEBUG address 0x%1 (%2)").arg(data_p, 8, 16, QChar('0')).arg(data_p));
                ConsoleWrite(QString("[IPC]DEBUG offset  0x%1 (%2)").arg(offset, 8, 16, QChar('0')).arg(offset));
                ConsoleWrite(QString("[IPC]DEBUG size    0x%1 (%2)").arg(size, 8, 16, QChar('0')).arg(size));
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Invalid texture property: %1\n").arg(texProperty.name));
                PINFO(QString("Unknown texture property type: %1\n").arg(texProperty.type));
                PDEBUG(QString(" => Address 0x%1 (%2)\n").arg(data_p, 8, 16, QChar('0')).arg(data_p));
                PDEBUG(QString(" => Offset  0x%1 (%2)\n").arg(offset, 8, 16, QChar('0')).arg(offset));
                PDEBUG(QString(" => Size    0x%1 (%2)\n").arg(size, 8, 16, QChar('0')).arg(size));
            }

            CRASH_MSG(err_str.c_str());
        }

        nextOffset = valueRawPos + size;
    }
    texProperty.valueRaw = ByteBuffer(size);
    texProperty.valueStruct = ByteBuffer();
    texProperty.fetched = false;
    memcpy(texProperty.valueRaw.ptr(), data + valueRawPos, size);
    texPropertyList.push_back(texProperty);

    if (nextOffset != offset)
        getProperty(data, nextOffset);
}

TextureProperty::TexturePropertyEntry TextureProperty::getProperty(const QString &name)
{
    fetchValue(name);
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            return texPropertyList[i];
        }
    }
    CRASH("");
}

void TextureProperty::fetchValue(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            fetchValue(i);
            return;
        }
    }
}

void TextureProperty::fetchValue(int index)
{
    if (index < 0 || index >= texPropertyList.count())
        CRASH("");
    TexturePropertyEntry texProperty = texPropertyList[index];
    if (texProperty.fetched || texProperty.name == "None")
        return;
    if (texProperty.type == "IntProperty" ||
        texProperty.type == "ObjectProperty")
    {
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0);
    }
    else if (texProperty.type == "ByteProperty")
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueNameType = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
            texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 8));
            texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 12);
        }
        else
        {
            texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
            texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr()  + 4);
        }
    }
    else if (texProperty.type == "BoolProperty")
    {
        texProperty.valueBool = texProperty.valueRaw.ptr()[0] != 0;
    }
    else if (texProperty.type == "StrProperty")
    {
    }
    else if (texProperty.type == "FloatProperty")
    {
        texProperty.valueFloat = *reinterpret_cast<float *>(texProperty.valueRaw.ptr() + 0);
    }
    else if (texProperty.type == "NameProperty")
    {
        texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 4);
    }
    else if (texProperty.type == "StructProperty")
    {
        texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 4);
        texProperty.valueStruct = ByteBuffer(texProperty.valueRaw.size() - 8);
        memcpy(texProperty.valueStruct.ptr(), texProperty.valueRaw.ptr() + 8, texProperty.valueStruct.size());
    }
    else
        CRASH("");

    texProperty.fetched = true;
    texPropertyList[index] = texProperty;
}

QString TextureProperty::getDisplayString(int index)
{
    QString result = "";
    if (index < 0 || index >= texPropertyList.count())
        CRASH();

    fetchValue(index);
    TexturePropertyEntry texProperty = texPropertyList[index];
    if (texProperty.name == "None")
        return result;

    result = "  " + texProperty.name + ": ";
    if (texProperty.type == "IntProperty")
    {
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "ObjectProperty")
    {
        result += package->getName(package->getClassNameId(texProperty.valueInt)) + "\n";
    }
    else if (texProperty.type == "ByteProperty")
    {
        if (GameData::gameType == MeType::ME3_TYPE)
            result += texProperty.valueNameType + ": ";
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "BoolProperty")
    {
        result += QString(texProperty.valueBool ? "true" : "false") + "\n";
    }
    else if (texProperty.type == "StrProperty")
    {
        result += "\n";
    }
    else if (texProperty.type == "FloatProperty")
    {
        result += QString::number(texProperty.valueFloat) + "\n";
    }
    else if (texProperty.type == "NameProperty")
    {
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "StructProperty")
    {
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else
        CRASH();

    return result;
}

bool TextureProperty::exists(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
            return true;
    }
    return false;
}

void TextureProperty::removeProperty(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            texPropertyList.removeAt(i);
            return;
        }
    }
}

void TextureProperty::setIntValue(const QString &name, qint32 value)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "IntProperty")
            CRASH();
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(sizeof(qint32));
        texProperty.type = "IntProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    memcpy(texProperty.valueRaw.ptr(), &value, sizeof(qint32));
    texProperty.valueInt = value;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TextureProperty::setFloatValue(const QString &name, float value)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "FloatProperty")
            CRASH();
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(sizeof(float));
        texProperty.type = "FloatProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    memcpy(texProperty.valueRaw.ptr(), &value, sizeof(float));
    texProperty.valueFloat = value;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TextureProperty::setByteValue(const QString &name, const QString &valueName,
                               const QString &valueNameType, qint32 valueInt)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "ByteProperty")
            CRASH();
    }
    else
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueRaw = ByteBuffer(16);
            memset(texProperty.valueRaw.ptr() + 4, 0, sizeof(qint32));
        }
        else
        {
            texProperty.valueRaw = ByteBuffer(8);
        }
        texProperty.type = "ByteProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);
    if (GameData::gameType == MeType::ME3_TYPE)
    {
        if (!package->existsNameId(valueNameType))
            package->addName(valueNameType);
        qint32 nameTypeId = package->getNameId(valueNameType);
        qint32 nameId = package->getNameId(valueName);
        memcpy(texProperty.valueRaw.ptr(), &nameTypeId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 8, &nameId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 12, &valueInt, sizeof(qint32));
    }
    else
    {
        qint32 nameId = package->getNameId(valueName);
        memcpy(texProperty.valueRaw.ptr() + 0, &nameId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 4, &valueInt, sizeof(qint32));
    }
    texProperty.valueName = valueName;
    texProperty.valueInt = valueInt;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TextureProperty::setBoolValue(const QString &name, bool value)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "BoolProperty")
            CRASH();
    }
    else
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueRaw = ByteBuffer(1);
        }
        else
        {
            texProperty.valueRaw = ByteBuffer(4);
            memset(texProperty.valueRaw.ptr() + 1, 0, 3);
        }
        texProperty.type = "BoolProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (value)
        texProperty.valueRaw.ptr()[0] = 1;
    else
        texProperty.valueRaw.ptr()[0] = 0;
    texProperty.valueBool = value;

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TextureProperty::setNameValue(const QString &name, const QString &valueName, qint32 valueInt)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "NameProperty")
            CRASH();
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(8);
        texProperty.type = "NameProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(texProperty.valueRaw.ptr(), &nameId, sizeof(qint32));
    memcpy(texProperty.valueRaw.ptr() + 4, &valueInt, sizeof(qint32));
    texProperty.valueName = valueName;
    texProperty.valueInt = valueInt;

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TextureProperty::setStructValue(const QString &name, const QString &valueName, ByteBuffer valueStruct)
{
    TexturePropertyEntry texProperty{};
    if (exists(name))
    {
        int index = -1;
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                index = i;
                break;
            }
        }
        fetchValue(index);
        texProperty = texPropertyList[index];
        if (texProperty.type != "StructProperty" || texProperty.valueStruct.size() != valueStruct.size())
            CRASH();
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(valueStruct.size() + 8);
        texProperty.valueStruct = ByteBuffer(valueStruct.size());
        texProperty.type = "StructProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;
    texProperty.valueName = valueName;
    texProperty.valueInt = 0;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(texProperty.valueRaw.ptr(), &nameId, sizeof(qint32));
    memcpy(texProperty.valueRaw.ptr() + 4, &texProperty.valueInt, sizeof(qint32));
    memcpy(texProperty.valueRaw.ptr() + 8, valueStruct.ptr(), valueStruct.size());
    memcpy(texProperty.valueStruct.ptr(), valueStruct.ptr(), valueStruct.size());

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

ByteBuffer TextureProperty::toArray()
{
    MemoryStream mem;
    mem.WriteUInt32(headerData);
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        mem.WriteInt32(package->getNameId(texPropertyList[i].name));
        mem.WriteInt32(0); // skip
        if (texPropertyList[i].name == "None")
            break;
        mem.WriteInt32(package->getNameId(texPropertyList[i].type));
        mem.WriteInt32(0); // skip
        int size = texPropertyList[i].valueRaw.size();
        if (texPropertyList[i].type == "StructProperty")
        {
            size -= 8;
        }
        else if (texPropertyList[i].type == "ByteProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size -= 8;
        }
        else if (texPropertyList[i].type == "BoolProperty")
        {
            size = 0;
        }
        mem.WriteInt32(size);
        mem.WriteInt32(texPropertyList[i].index);
        mem.WriteFromBuffer(texPropertyList[i].valueRaw.ptr(), texPropertyList[i].valueRaw.size());
    }

    return mem.ToArray();
}
