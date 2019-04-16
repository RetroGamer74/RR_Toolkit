/*
* ReiNX Toolkit
* Copyright (C) 2018  Team ReiSwitched
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
*/

#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

namespace FS {
    vector<string> EnumDir(string);
    bool DirExists(string dirstr);
    void SetFTPStatus(bool value);
    void SetLDNStatus(bool value);
    void SetAMIBOStatus(bool value);
    void DeleteFile(string path);
    unsigned DeleteDirRecursive(string path);
    unsigned MakeDir(string file, unsigned perm);
    int GetCustomFirmware_ID();
    bool CopyFile(string origen, string destino);
    string GetLineFile(string path);
    string GetProdinfoKeyValue(string path);
    string GetCustomFirmwareString();
    void WriteLineFile(string path, string data);
    string GetCustomFirmwareFolder();
    string GetDefaultTemplate();
    string GetRRRelease();
    string GetToggleMarkWithTemplateName(string atemplate);
    string GetTemplateName(string templateCode);
    string SwitchIdent_GetFirmwareVersion();
    string GetNXSerialNumber();
    bool IsTemplatedEnabled();
    bool IsProdinfoRW();
    int GetFirmwareMajorVersion();
    void FixTemplateCrash(string path);
    int CheckFileExists(string path);
    void DoEnableTemplate(string TemplateCode);
    string GetFullTemplatePath();
    string GetFullProdinfoPath();
    string GetFullFTPPath();
    string GetFullLDNPath();
    string GetFullAMIBOPath();
    bool IsFTPEnabled();
    bool IsLDNEnabled();
    bool IsAMIBOEnabled();
    int GetRRReleaseNumber();
    void SetProdinfoMode(int value);
	string TestFunction();
};
