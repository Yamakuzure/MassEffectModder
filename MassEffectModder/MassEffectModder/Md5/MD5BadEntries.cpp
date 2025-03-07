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

#include <Md5/MD5BadEntries.h>

const char Bad_ME1UITM[] = "MEUITM legacy installer version";
const char Bad_ME1Controller[] = "ME1 Controller v1.2.0 - legacy MEUITM version";
const char Bad_ME1SameSexRomances[] = "ME1 Same-Sex Romances v2.0 - legacy MEUITM version";
const char Bad_NoSharedCooldown[] = "ME2 No Shared Cooldown";

MD5ModFileEntry badMOD[] =
{

{
"/BioGame/CookedPC/Maps/END/CIN/BIOA_END20_Bridge_CIN.SFM",
{ 0x45, 0x15, 0xC3, 0x47, 0xBB, 0xAF, 0xD4, 0xC6, 0x09, 0xD3, 0xBC, 0xA9, 0xFC, 0xA1, 0x60, 0x0A, },
Bad_ME1UITM,
},
{
"/BioGame/CookedPC/Maps/JUG/DSG/BIOA_JUG20_00_DSG.SFM",
{ 0x5E, 0x2E, 0x64, 0x63, 0x98, 0x26, 0x07, 0xB5, 0xE8, 0x1A, 0xC5, 0x99, 0xBE, 0x1F, 0xED, 0xAF, },
Bad_ME1UITM,
},
{
"/BioGame/CookedPC/BIOC_Materials.u",
{ 0x0D, 0x0E, 0x4D, 0x96, 0xAC, 0x8E, 0x86, 0x0E, 0xBE, 0x19, 0x71, 0x97, 0x3E, 0xB5, 0xA8, 0x5A, },
Bad_ME1Controller,
},
{
"/BioGame/CookedPC/Maps/EntryMenu.SFM",
{ 0xDF, 0xB7, 0x59, 0x83, 0x05, 0xC6, 0x81, 0x63, 0x7A, 0xA1, 0xFD, 0x3B, 0x01, 0xA1, 0x56, 0xFC, },
Bad_ME1Controller,
},
{
"/BioGame/CookedPC/Startup_int.upk",
{ 0xB5, 0x4E, 0x98, 0x0F, 0x58, 0x4F, 0xD2, 0x0C, 0xBA, 0x63, 0xAB, 0x02, 0x90, 0x9D, 0xB7, 0x3E, },
Bad_ME1Controller,
},
{
"/BioGame/CookedPC/Maps/NOR/DS2/BIOA_NOR10_09ashley_DS2.SFM",
{ 0xA5, 0x4A, 0x66, 0xA3, 0x68, 0xE9, 0xCC, 0xEE, 0xA4, 0x1F, 0x84, 0x5F, 0x1B, 0xD7, 0xE2, 0x22, },
Bad_ME1SameSexRomances,
},
{
"/BioGame/CookedPC/Maps/NOR/DS2/BIOA_NOR10_09kaidan_DS2.SFM",
{ 0x68, 0xA7, 0x46, 0x0E, 0x0D, 0xA1, 0x67, 0xB7, 0xA5, 0x55, 0x16, 0x4B, 0x50, 0xBE, 0xA7, 0x67, },
Bad_ME1SameSexRomances,
},
{
"/BioGame/CookedPC/Maps/NOR/DSG/BIOA_NOR10_04A_DSG.SFM",
{ 0x3D, 0x72, 0x6D, 0x40, 0x16, 0xB9, 0x50, 0x83, 0x1A, 0x1D, 0x15, 0x9F, 0x55, 0x60, 0xA8, 0x8C, },
Bad_ME1SameSexRomances,
},
{
"/BioGame/CookedPC/Maps/NOR/DSG/BIOA_NOR10_06wake_DSG.SFM",
{ 0x93, 0xE4, 0xE6, 0xBE, 0x7A, 0x0B, 0x09, 0x07, 0x0B, 0x27, 0xD9, 0x5C, 0xD7, 0x1F, 0xA4, 0x71, },
Bad_ME1SameSexRomances,
},
{
"/BioGame/CookedPC/Maps/STA/DSG/BIOA_STA60_09A_DSG.SFM",
{ 0x77, 0xD8, 0x8D, 0x60, 0x18, 0x00, 0x82, 0xE0, 0xCC, 0x7B, 0x2A, 0x68, 0xC1, 0x60, 0x62, 0xCA, },
Bad_ME1SameSexRomances,
},
{
"/BioGame/CookedPC/EntryMenu.pcc",
{ 0xB0, 0x56, 0x45, 0x71, 0xA5, 0x7F, 0x6F, 0xEA, 0xFC, 0x88, 0x13, 0x29, 0xA6, 0xB6, 0xBA, 0x90, },
Bad_NoSharedCooldown,
},
{
"/BioGame/CookedPC/SFXGame.pcc",
{ 0x15, 0x4D, 0xDA, 0x5B, 0xB3, 0xB4, 0x3C, 0x3C, 0x1B, 0xAD, 0xCB, 0xB4, 0x52, 0xFD, 0x72, 0xCF, },
Bad_NoSharedCooldown,
},
{
"/BioGame/CookedPC/SFXGame.pcc",
{ 0xEB, 0x7D, 0xDC, 0xE5, 0x35, 0x65, 0xFB, 0x6B, 0x32, 0x51, 0x74, 0x2D, 0x6B, 0x2C, 0x0C, 0x3F, },
Bad_NoSharedCooldown,
},
{
"/BioGame/CookedPC/SFXGame.pcc",
{ 0x75, 0x0D, 0x73, 0xB8, 0x23, 0x58, 0xAA, 0x4C, 0x44, 0x18, 0xD5, 0xBA, 0xF2, 0x64, 0xFB, 0x4A, },
Bad_NoSharedCooldown,
},
{
"/BIOGame/DLC/DLC_MOD_ProjectVariety/CookedPCConsole/BioD_Lev004_280Leviathan.pcc",
{ 0xC9, 0x64, 0x79, 0x14, 0x86, 0xE3, 0xEE, 0x8A, 0x72, 0x15, 0x64, 0xF3, 0x90, 0x10, 0xD5, 0xDF, },
"Project Variety 1.0",
},
{
"/BIOGame/DLC/DLC_MOD_ProjectVariety/CookedPCConsole/BioD_Lev004_280Leviathan.pcc",
{ 0x07, 0xB2, 0x53, 0x84, 0xEA, 0xD5, 0x71, 0x6E, 0x91, 0xAE, 0xCA, 0x4B, 0x22, 0x99, 0x7C, 0xFA, },
"Project Variety 1.1.0",
},
{
"/BIOGame/DLC/DLC_MOD_JKalliancearmour/CookedPCConsole/BioH_Kaidan_00.pcc",
{ 0x7d, 0xa5, 0x1e, 0xc1, 0x0c, 0x05, 0x60, 0xda, 0xe9, 0x77, 0xbd, 0x44, 0x8a, 0xb0, 0x64, 0x59, },
"ME3 James and Kaidan Systems Alliance Marines armour",
},
{
"/BIOGame/DLC/DLC_MOD_Spectre/CookedPCConsole/BioA_N7Karza_000LevelTrans_LOC_INT.pcc",
{ 0xf5, 0xb1, 0xf6, 0x66, 0x63, 0xcf, 0xfc, 0x46, 0x07, 0x9e, 0xac, 0x8c, 0xe2, 0xfd, 0x15, 0xb5, },
"ME3 Spectre Expansion mod 1.1",
},
{
"/BIOGame/CookedPCConsole/BioD_Nor_405Engineering_LOC_INT.pcc",
{ 0x9D, 0x63, 0x72, 0xD1, 0x95, 0xE6, 0x98, 0x49, 0x64, 0xA9, 0xEF, 0x3A, 0x6E, 0x30, 0xD4, 0x8F, },
"Improperly installed Urz On The Normandy Leviathan Patch",
},
{
"/BIOGame/CookedPCConsole/BioD_Nor_500.pcc",
{ 0xCC, 0xD5, 0x44, 0x5C, 0x13, 0x9B, 0x63, 0x3D, 0xE9, 0x2D, 0xCE, 0x90, 0x8B, 0xD1, 0x0C, 0xE5, },
"Improperly installed Urz On The Normandy Leviathan Patch",
},
{
"/BIOGame/CookedPCConsole/BioD_Nor_500_LOC_INT.pcc",
{ 0x60, 0x57, 0xDE, 0x3A, 0x5D, 0xD7, 0x88, 0x2E, 0xB3, 0xA3, 0x96, 0x3A, 0x27, 0xB4, 0x2E, 0x1A, },
"Improperly installed Urz On The Normandy Leviathan Patch",
},

};

const int badMODSize = sizeof (badMOD) / sizeof (MD5ModFileEntry);
