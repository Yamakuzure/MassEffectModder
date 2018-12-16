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

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"

#include "CmdLineParams.h"
#include "CmdLineTools.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "MemTypes.h"
#include "TOCFile.h"
#include "DLC.h"
#include "Misc.h"

void DisplayHelp()
{
    ConsoleWrite("\nHelp:");
    ConsoleWrite("  --help");
    ConsoleWrite("     This help");
    ConsoleWrite("");
    ConsoleWrite("  --scan --gameid <game id> [--ipc]");
    ConsoleWrite("     Scan game data for textures.");
    ConsoleWrite("");
    ConsoleWrite("  --update-toc");
    ConsoleWrite("     Update TOC files in ME3.");
    ConsoleWrite("");
    ConsoleWrite("  --unpack-dlcs [--ipc]");
    ConsoleWrite("     Unpack ME3 DLCs.");
    ConsoleWrite("");
    ConsoleWrite("  --repack --gameid <game id> [--ipc]");
    ConsoleWrite("     Repack ME2 or ME3 packages.");
    ConsoleWrite("");
    ConsoleWrite("  --check-game-data-after --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Check game data for mods installed after textures installation.\n");
    ConsoleWrite("");
    ConsoleWrite("  --check-game-data-mismatch --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Check game data with md5 database.\n");
    ConsoleWrite("     Scan to detect mods");
    ConsoleWrite("");
    ConsoleWrite("  --check-game-data-only-vanilla --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Check game data with md5 database.\n");
    ConsoleWrite("");
    ConsoleWrite("  --check-for-markers --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Check game data for markers.\n");
    ConsoleWrite("");
    ConsoleWrite("  --install-mods --gameid <game id> --input <input dir> [--repack] [--ipc] [--alot-mode] [--limit-2k]\n");
    ConsoleWrite("     Install MEM mods from input directory.\n");
    ConsoleWrite("");
    ConsoleWrite("  --apply-me1-laa\n");
    ConsoleWrite("     Apply LAA patch to ME1 executable.\n");
    ConsoleWrite("");
    ConsoleWrite("  --detect-mods --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Detect compatibe mods.\n");
    ConsoleWrite("");
    ConsoleWrite("  --detect-bad-mods --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Detect not compatibe mods.\n");
    ConsoleWrite("");
    ConsoleWrite("  --apply-lods-gfx --gameid <game id> [--soft-shadows-mode] [--meuitm-mode] [--limit-2k]\n");
    ConsoleWrite("     Update LODs and GFX settings.\n");
    ConsoleWrite("");
    ConsoleWrite("  --remove-lods --gameid <game id>\n");
    ConsoleWrite("     Remove LODs settings.\n");
    ConsoleWrite("");
    ConsoleWrite("  --print-lods --gameid <game id> [--ipc]\n");
    ConsoleWrite("     Print LODs settings.\n");
    ConsoleWrite("");
    ConsoleWrite("  --convert-to-mem --gameid <game id> --input <input dir> --output <output file> [--mark-to-convert] [--ipc]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     input dir: directory to be converted, containing following file extension(s):");
    ConsoleWrite("        MEM, MOD, TPF");
    ConsoleWrite("        BIN - package export raw data");
    ConsoleWrite("           Naming pattern used for package in DLC:");
    ConsoleWrite("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    ConsoleWrite("           Naming pattern used for package in base directory:");
    ConsoleWrite("             B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("             example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    ConsoleWrite("        XDELTA - package export xdelta3 patch data");
    ConsoleWrite("           Naming pattern used for package in DLC:");
    ConsoleWrite("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta");
    ConsoleWrite("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.xdelta");
    ConsoleWrite("           Naming pattern used for package in base directory:");
    ConsoleWrite("             B<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta");
    ConsoleWrite("             example: B23-BioH_EDI_00_Explore.pcc-E5090.xdelta");
    ConsoleWrite("        DDS, BMP, TGA, PNG, JPG, JPEG");
    ConsoleWrite("           input format supported for DDS images:");
    ConsoleWrite("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    ConsoleWrite("           input format supported for TGA images:");
    ConsoleWrite("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    ConsoleWrite("           input format supported for BMP images:");
    ConsoleWrite("              uncompressed RGBA/RGB/RGBX");
    ConsoleWrite("           Image filename must include texture CRC (0xhhhhhhhh)");
    ConsoleWrite("     ipc: turn on IPC traces");
    ConsoleWrite("");
    ConsoleWrite("  --convert-game-image --gameid <game id> --input <input image> <--output output image> [--mark-to-convert]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     Input file with following extension:");
    ConsoleWrite("        DDS, BMP, TGA, PNG, JPG, JPEG");
    ConsoleWrite("           input format supported for DDS images:");
    ConsoleWrite("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    ConsoleWrite("           input format supported for TGA images:");
    ConsoleWrite("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    ConsoleWrite("           input format supported for BMP images:");
    ConsoleWrite("              uncompressed RGBA/RGB/RGBX");
    ConsoleWrite("           Image filename must include texture CRC (0xhhhhhhhh)");
    ConsoleWrite("     Output file is DDS image");
    ConsoleWrite("");
    ConsoleWrite("  --convert-game-images --gameid <game id> --input <input dir> --output <output dir> [--mark-to-convert]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     input dir: directory to be converted, containing following file extension(s):");
    ConsoleWrite("        Input files with following extension:");
    ConsoleWrite("        DDS, BMP, TGA, PNG, JPEG");
    ConsoleWrite("           input format supported for DDS images:");
    ConsoleWrite("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    ConsoleWrite("           input format supported for TGA images:");
    ConsoleWrite("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    ConsoleWrite("           input pixel format supported for BMP images:");
    ConsoleWrite("              uncompressed RGBA/RGB/RGBX");
    ConsoleWrite("           Image filename must include texture CRC (0xhhhhhhhh)");
    ConsoleWrite("     output dir: directory where textures converted to DDS are placed");
    ConsoleWrite("");
    ConsoleWrite("  --extract-mod --gameid <game id> --input <input dir> --output <output dir> [--ipc]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     input dir: directory of ME3Explorer MOD file(s)");
    ConsoleWrite("     Can extract textures and package export raw data");
    ConsoleWrite("     Naming pattern used for package in DLC:");
    ConsoleWrite("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    ConsoleWrite("     Naming pattern used for package in base directory:");
    ConsoleWrite("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    ConsoleWrite("");
    ConsoleWrite("  --extract-mem --gameid <game id> --input <input dir> --output <output dir> [--ipc]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     input dir: directory of MEM mod file(s)");
    ConsoleWrite("     Can extract textures and package export raw data");
    ConsoleWrite("     Naming pattern used for package in DLC:");
    ConsoleWrite("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    ConsoleWrite("     Naming pattern used for package in base directory:");
    ConsoleWrite("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    ConsoleWrite("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    ConsoleWrite("");
    ConsoleWrite("  --extract-tpf --gameid <input dir> --output <output dir> [--ipc]\n");
    ConsoleWrite("     input dir: directory containing the TPF file(s) to be extracted");
    ConsoleWrite("     Textures are extracted as they are in the TPF, no additional modifications are made.");
    ConsoleWrite("");
    ConsoleWrite("  --convert-image --format <output pixel format> [--threshold <dxt1 alpha threshold>] --input <input image> --output <output image>\n");
    ConsoleWrite("     input image file types: DDS, BMP, TGA, PNG, JPEG");
    ConsoleWrite("     output image file type: DDS");
    ConsoleWrite("     output pixel format: DXT1 (no alpha), DXT1a (alpha), DXT3, DXT5, ATI2, V8U8, G8, RGBA, RGB");
    ConsoleWrite("     For DXT1a you have to set the alpha threshold (0-255). 128 is suggested as a default value.");
    ConsoleWrite("");
    ConsoleWrite("  --extract-all-dds --gameid <game id> --output <output dir> [--tfc-name <filter name|--pcc-only|--tfc-only>]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     output dir: directory where textures converted to DDS are placed");
    ConsoleWrite("     TFC filter name: it will filter only textures stored in specific TFC file.");
    ConsoleWrite("     Or option: -pcc-only to extract only textures stored in packages.");
    ConsoleWrite("     Or option: -tfc-only to extract only textures stored in TFC files.");
    ConsoleWrite("     Textures are extracted as they are in game data, only DDS header is added.");
    ConsoleWrite("");
    ConsoleWrite("  --extract-all-png --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only]\n");
    ConsoleWrite("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    ConsoleWrite("     output dir: directory where textures converted to PNG are placed");
    ConsoleWrite("     TFC filter name: it will filter only textures stored in specific TFC file.");
    ConsoleWrite("     Or option: -pcc-only to extract only textures stored in packages.");
    ConsoleWrite("     Or option: -tfc-only to extract only textures stored in TFC files.");
    ConsoleWrite("     Textures are extracted with only top mipmap.");
    ConsoleWrite("");
    ConsoleWrite("  --dlc-mod-textures --gameid <game id> --input <mem file> [--tfc-name <tfc name>] [--guid <guid in 16 hex digits>]\n");
    ConsoleWrite("     Replace textures from <mem file> and store in new <tfc name> file.");
    ConsoleWrite("     New TFC name must be added earlier to PCC files.");
    ConsoleWrite("\n");
}

bool hasValue(const QStringList &args, int curPos)
{
    return args.count() >= (curPos + 2) && !args[curPos + 1].contains("--");
}

QStringList convertLegacyArguments()
{
    QStringList retArgs;
    const QStringList args = QCoreApplication::arguments();
    if (args.count() < 2)
        return QStringList();

    if (args[1] == "-check-game-data-after")
    {
        if (args.count() > 1)
        {
            retArgs.append("--check-game-data-after");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-check-game-data-mismatch")
    {
        if (args.count() > 1)
        {
            retArgs.append("--check-game-data-mismatch");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-check-game-data-only-vanilla")
    {
        if (args.count() > 1)
        {
            retArgs.append("--check-game-data-vanilla");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-check-for-markers")
    {
        if (args.count() > 1)
        {
            retArgs.append("--check-for-markers");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-install-mods")
    {
        if (args.count() > 2)
        {
            retArgs.append("--install-mods");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--input");
            retArgs.append(args[2]);
            for (int a = 0; a < args.count(); a++)
            {
                if (args[a] == "-repack")
                    retArgs.append("--repack-mode");
            }
            retArgs.append("--ipc");
        }

    }
    else if (args[1] == "-apply-me1-laa")
    {
        retArgs.append("--apply-me1-laa");
    }
    else if (args[1] == "-detect-mods")
    {
        if (args.count() > 1)
        {
            retArgs.append("--detect-mods");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-detect-bad-mods")
    {
        if (args.count() > 1)
        {
            retArgs.append("--detect-bad-mods");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-apply-lods-gfx")
    {
        if (args.count() > 1)
        {
            retArgs.append("--apply-lods-gfx");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            for (int a = 0; a < args.count(); a++)
            {
                if (args[a] == "-soft-shadows-mode")
                    retArgs.append("--soft-shadows-mode");
            }
            for (int a = 0; a < args.count(); a++)
            {
                if (args[a] == "-meuitm-mode")
                    retArgs.append("--meuitm-mode");
            }
            for (int a = 0; a < args.count(); a++)
            {
                if (args[a] == "-limit2k")
                    retArgs.append("--limit-2k");
            }
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-remove-lods")
    {
        if (args.count() > 1)
        {
            retArgs.append("--remove-lods");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-print-lods")
    {
        if (args.count() > 1)
        {
            retArgs.append("--print-lods");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-convert-to-mem")
    {
        if (args.count() > 3)
        {
            retArgs.append("--convert-to-mem");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--input");
            retArgs.append(args[2]);
            retArgs.append("--output");
            retArgs.append(args[3]);
            for (int a = 0; a < args.count(); a++)
            {
                if (args[a] == "--mark-to-convert")
                    retArgs.append("--mark-to-convert");
            }
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-extract-mod")
    {
        if (args.count() > 3)
        {
            retArgs.append("--extract-mod");
            retArgs.append("--gameid");
            retArgs.append(args[1]);
            retArgs.append("--input");
            retArgs.append(args[2]);
            retArgs.append("--output");
            retArgs.append(args[3]);
            retArgs.append("--ipc");
        }
    }
    else if (args[1] == "-extract-tpf")
    {
        if (args.count() > 2)
        {
            retArgs.append("--extract-tpf");
            retArgs.append("--input");
            retArgs.append(args[1]);
            retArgs.append("--output");
            retArgs.append(args[2]);
            retArgs.append("--ipc");
        }
    }
    else
    {
        return args;
    }

    return retArgs;
}

int ProcessArguments()
{
    int errorCode = 0;
    int cmd = CmdType::UNKNOWN;
    MeType gameId = MeType::UNKNOWN_TYPE;
    bool ipc = false;
    bool markToConvert = false;
    bool guiMode = false;
    bool repackMode = false;
    bool meuitmMode = false;
    bool softShadowsMods = false;
    bool limit2k = false;
    bool pccOnly = false;
    bool tfcOnly = false;
    QString input, output, threshold, format, tfcName, guid;
    CmdLineTools tools;

    QStringList args = convertLegacyArguments();

    for (int l = 0; l < args.count(); l++)
    {
        const QString arg = args[l].toLower();
        if (arg == "--help")
            cmd = CmdType::HELP;
        if (arg == "--scan")
            cmd = CmdType::SCAN;
        if (arg == "--update-toc")
            cmd = CmdType::UPDATE_TOC;
        if (arg == "--unpack-dlcs")
            cmd = CmdType::UNPACK_DLCS;
        if (arg == "--repack")
            cmd = CmdType::REPACK;
        if (arg == "--convert-to-mem")
            cmd = CmdType::CONVERT_TO_MEM;
        if (arg == "--convert-game-image")
            cmd = CmdType::CONVERT_GAME_IMAGE;
        if (arg == "--convert-game-images")
            cmd = CmdType::CONVERT_GAME_IMAGES;
        if (arg == "--convert-image")
            cmd = CmdType::CONVERT_IMAGE;
        if (arg == "--install-mods")
            cmd = CmdType::INSTALL_MODS;
        if (arg == "--extract-mod")
            cmd = CmdType::EXTRACT_MOD;
        if (arg == "--extract-mem")
            cmd = CmdType::EXTRACT_MEM;
        if (arg == "--extract-tpf")
            cmd = CmdType::EXTRACT_TPF;
        if (arg == "--detect-mods")
            cmd = CmdType::DETECT_MODS;
        if (arg == "--detect-bad-mods")
            cmd = CmdType::DETECT_BAD_MODS;
        if (arg == "--apply-lods-gfx")
            cmd = CmdType::APPLY_LODS_GFX;
        if (arg == "--print-lods")
            cmd = CmdType::PRINT_LODS;
        if (arg == "--remove-lods")
            cmd = CmdType::REMOVE_LODS;
        if (arg == "--apply-me1-laa")
            cmd = CmdType::APPLY_ME1_LAA;
        if (arg == "--check-game-data-textures")
            cmd = CmdType::CHECK_GAME_DATA_TEXTURES;
        if (arg == "--check-game-data-mismatch")
            cmd = CmdType::CHECK_GAME_DATA_MISMATCH;
        if (arg == "--check-game-data-after")
            cmd = CmdType::CHECK_GAME_DATA_AFTER;
        if (arg == "--check-game-data-vanilla")
            cmd = CmdType::CHECK_GAME_DATA_VANILLA;
        if (arg == "--check-for-markers")
            cmd = CmdType::CHECK_FOR_MARKERS;
        if (arg == "--extract-all-dds")
            cmd = CmdType::EXTRACT_ALL_DDS;
        if (arg == "--extract-all-png")
            cmd = CmdType::EXTRACT_ALL_PNG;
        if (arg == "--dlc-mod-textures")
            cmd = CmdType::DLC_MOD_TEXTURES;
        else if (arg == "--gameid" && hasValue(args, l))
        {
            bool ok;
            int id = args[l + 1].toInt(&ok);
            if (ok && id >= 1 && id <= 3)
            {
                gameId = (MeType)id;
            }
        }
        else if (arg == "--ipc")
            ipc = true;
        else if (arg == "--input" && hasValue(args, l))
        {
            input = args[l + 1];
            if (!QDir(input).exists() && !QFile(input).exists())
            {
                ConsoleWrite("Input path doesn't exists!");
                input = "";
            }
        }
        else if (arg == "--output" && hasValue(args, l))
        {
            output = args[l + 1];
        }
        else if (arg == "--mark-to-convert")
            markToConvert = true;
        else if (arg == "--alot-mode")
            guiMode = true;
        else if (arg == "--repack-mode")
            repackMode = true;
        else if (arg == "--soft-shadows-mode")
            softShadowsMods = true;
        else if (arg == "--meuitm-mode")
            meuitmMode = true;
        else if (arg == "--limit-2k")
            limit2k = true;
        else if (arg == "--pcc-only" && !tfcOnly)
            pccOnly = true;
        else if (arg == "--tfc-only" && !pccOnly)
            tfcOnly = true;
        else if (arg == "--threshold" && hasValue(args, l))
        {
            threshold = args[l + 1];
        }
        else if (arg == "--format" && hasValue(args, l))
        {
            format = args[l + 1];
        }
        else if (arg == "--tfc-name" && hasValue(args, l) &&
                !pccOnly && !tfcOnly)
        {
            tfcName = args[l + 1];
        }
        else if (arg == "--guid" && hasValue(args, l))
        {
            guid = args[l + 1];
        }
    }

    switch (cmd)
    {
    case CmdType::HELP:
        DisplayHelp();
        return 0;
    case CmdType::UNKNOWN:
        ConsoleWrite("Wrong command!");
        DisplayHelp();
        return -1;
    }

    switch (cmd)
    {
    case CmdType::SCAN:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        errorCode = tools.scanTextures(gameId, ipc);
        break;
    case CmdType::UPDATE_TOC:
    {
        ConfigIni configIni = ConfigIni();
        g_GameData->Init(MeType::ME3_TYPE, configIni);
        if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
        {
            ConsoleWrite("Error: Could not found the game!");
            errorCode = -1;
            break;
        }
        TOCBinFile::UpdateAllTOCBinFiles();
        break;
    }
    case CmdType::UNPACK_DLCS:
    {
        ConfigIni configIni = ConfigIni();
        g_GameData->Init(MeType::ME3_TYPE, configIni);
        if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
        {
            ConsoleWrite("Error: Could not found the game!");
            errorCode = -1;
            break;
        }
        Misc::startTimer();
        ME3DLC::unpackAllDLC(ipc);
        long elapsed = Misc::elapsedTime();
        ConsoleWrite(Misc::getTimerFormat(elapsed));
        break;
    }
    case CmdType::REPACK:
    {
        if (gameId != MeType::ME2_TYPE && gameId != MeType::ME3_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        ConfigIni configIni = ConfigIni();
        g_GameData->Init(gameId, configIni);
        if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
        {
            ConsoleWrite("Error: Could not found the game!");
            errorCode = -1;
            break;
        }
        Misc::startTimer();
        tools.Repack(gameId, ipc);
        long elapsed = Misc::elapsedTime();
        ConsoleWrite(Misc::getTimerFormat(elapsed));
        break;
    }
    case CmdType::CONVERT_TO_MEM:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.ConvertToMEM(gameId, input, output, markToConvert, ipc))
            errorCode = -1;
        break;
    case CmdType::CONVERT_GAME_IMAGE:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!output.endsWith(".dds", Qt::CaseInsensitive))
        {
            ConsoleWrite(QString("Error: output file is not dds: ") + output);
            errorCode = -1;
            break;
        }
        if (!tools.convertGameImage(gameId, input, output, markToConvert))
            errorCode = -1;
        break;
    case CmdType::CONVERT_IMAGE:
        if (!tools.convertImage(input, output, format, threshold))
            errorCode = -1;
        break;
    case CmdType::CONVERT_GAME_IMAGES:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.convertGameImages(gameId, input, output, markToConvert))
            errorCode = -1;
        break;
    case CmdType::INSTALL_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.InstallMods(gameId, input, ipc, repackMode, guiMode, limit2k))
            errorCode = -1;
        break;
    case CmdType::EXTRACT_MOD:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.extractMOD(gameId, input, output, ipc))
            errorCode = -1;
        break;
    case CmdType::EXTRACT_MEM:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.extractMEM(gameId, input, output, markToConvert))
            errorCode = -1;
        break;
    case CmdType::EXTRACT_TPF:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.extractTPF(input, output, ipc))
            errorCode = -1;
        break;
    case CmdType::DETECT_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.DetectMods(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::DETECT_BAD_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.DetectBadMods(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::APPLY_LODS_GFX:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.ApplyLODAndGfxSettings(gameId, softShadowsMods, meuitmMode, limit2k))
            errorCode = -1;
        break;
    case CmdType::PRINT_LODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.PrintLODSettings(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::REMOVE_LODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.RemoveLODSettings(gameId))
            errorCode = -1;
        break;
    case CmdType::APPLY_ME1_LAA:
        if (!tools.ApplyME1LAAPatch())
            errorCode = -1;
        break;
    case CmdType::CHECK_GAME_DATA_TEXTURES:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.CheckTextures(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::CHECK_GAME_DATA_AFTER:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.checkGameFilesAfter(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::CHECK_GAME_DATA_VANILLA:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.CheckGameData(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::CHECK_FOR_MARKERS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.CheckForMarkers(gameId, ipc))
            errorCode = -1;
        break;
    case CmdType::EXTRACT_ALL_DDS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.extractAllTextures(gameId, output, false, pccOnly, tfcOnly, tfcName))
            errorCode = -1;
        break;
    case CmdType::EXTRACT_ALL_PNG:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (!tools.extractAllTextures(gameId, output, true, pccOnly, tfcOnly, tfcName))
            errorCode = -1;
        break;
    case CmdType::DLC_MOD_TEXTURES:
        if (gameId == MeType::UNKNOWN_TYPE || gameId == MeType::ME1_TYPE)
        {
            ConsoleWrite("Wrong game id!");
            errorCode = -1;
            break;
        }
        if (tfcName.length() == 0)
        {
            ConsoleWrite("TFC name param missing!");
            errorCode = -1;
            break;
        }
        quint8 guidArray[16];
        memset(guidArray, 0, 16);
        QByteArray array;
        if (guid.length() != 0)
        {
            for (int i = 0; i < 32; i += 2)
            {
                bool ok;
                guidArray[i / 2] = guid.midRef(i, 2).toInt(&ok, 16);
                if (!ok)
                {
                    ConsoleWrite("Guid param is wrong!");
                    errorCode = -1;
                    break;
                }
            }
            array = QByteArray(reinterpret_cast<char *>(guidArray), 16);
        }
        if (!tools.applyMEMSpecialModME3(gameId, input, tfcName, array))
            errorCode = -1;
        break;
    }

    return errorCode;
}
