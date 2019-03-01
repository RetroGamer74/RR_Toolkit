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

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "FS.hpp"
#include <fstream>
#include <switch.h>

vector<string> FS::EnumDir(string path) {
    DIR *dp;
    vector<string> ret;
    struct dirent *ep;     
    dp = opendir (path.c_str());
    if (dp != NULL){
        while (ep = readdir(dp))
            ret.push_back(ep->d_name);
        closedir(dp);
    }
    return ret;
}

void FS::DeleteFile(string path) {
	 remove(path.c_str());
}


string FS::GetProdinfoKeyValue(string path) {
	std::string dataLine="0";
	std::ifstream datafile(path.c_str(),ios_base::binary);
    if (datafile.is_open()) {
        getline(datafile, dataLine);
        getline(datafile, dataLine);
        datafile.close();
    }
    return dataLine;
}


string FS::GetLineFile(string path) {
	std::string dataLine="0";
	std::ifstream datafile(path.c_str(),ios_base::binary);
    if (datafile.is_open()) {
        getline(datafile, dataLine);
        datafile.close();
    }
    return dataLine;
}

void FS::WriteLineFile(string path, string data) {
	std::ofstream dataFile(path.c_str());
	std::string my_string = data+"\n";
	dataFile << my_string;
}

int FS::GetCustomFirmware_ID() {
	return int(FS::GetLineFile("sdmc:/RR/boot.txt").at(0));
}

string FS::GetDefaultTemplate(){

	string cfw_folder = FS::GetCustomFirmwareFolder();
	return FS::GetLineFile("sdmc:/"+cfw_folder+"/template.txt");
}

string FS::GetRRRelease(){
	return FS::GetLineFile("sdmc:/RR_version.txt");
}

int FS::GetRRReleaseNumber(){
	string rr_release = FS::GetRRRelease();
	string charToFind = ".";
	string replaceChar = "";
	rr_release.replace(rr_release.find(charToFind),charToFind.length(),replaceChar);
	return std::atoi(rr_release.c_str());
}

string FS::GetTemplateName(string templateCode){
	string defaultTemplate = templateCode;
	string defaultTemplateName = "RetroReloaded";

	if(templateCode == "RR") defaultTemplateName= "RetroReloaded";
	if(templateCode == "SB") defaultTemplateName= "SuperBan";

	return defaultTemplateName;
}

int FS::CheckFileExists(string path) {
  FILE *f = fopen(path.c_str(), "rb");
  int exists = !!f;
  if (f) fclose(f);
  return exists;
}

string FS::GetToggleMarkWithTemplateName(string atemplate){
	string defaultTemplate;
	defaultTemplate = FS::GetDefaultTemplate();
	string ToggleMarkWithTemplateName;

	if(defaultTemplate == atemplate)
		ToggleMarkWithTemplateName = "\uE14B "+FS::GetTemplateName(atemplate);
	else
		ToggleMarkWithTemplateName = "\uE14C "+FS::GetTemplateName(atemplate);



	return ToggleMarkWithTemplateName;
}

string FS::SwitchIdent_GetFirmwareVersion() {
    Result rc = 0;
    SetSysFirmwareVersion fwversion;
    std::string firmwareVersion;
    rc = setsysInitialize();
	memset(&fwversion, 0, sizeof(fwversion));
	rc = setsysGetFirmwareVersion(&fwversion);
    if (R_FAILED(rc)) {
        return "ERROR_GETTING_FW_VER";
    }
    
    if (R_SUCCEEDED(rc))
    {
    	firmwareVersion = std::to_string(fwversion.major)+"."+std::to_string(fwversion.minor)+"."+std::to_string(fwversion.micro)+"-";
    	firmwareVersion = firmwareVersion+std::to_string(fwversion.revision_major)+std::to_string(fwversion.revision_minor);
	
    }
    setsysExit();
    return firmwareVersion;
}

int FS::GetFirmwareMajorVersion() {
    Result rc = 0;
    SetSysFirmwareVersion fwversion;
    int fwmajorVersion;

    rc = setsysInitialize();
	memset(&fwversion, 0, sizeof(fwversion));
	rc = setsysGetFirmwareVersion(&fwversion);
    if (R_FAILED(rc)) {
        return -1;
    }
    
    if (R_SUCCEEDED(rc))
    {
    	fwmajorVersion = fwversion.major;
	
    }
    setsysExit();
    return fwmajorVersion;

}

string FS::GetFullProdinfoPath()
{
    string config_dir = "sdmc:/atmosphere/prodinfo.ini";
    return config_dir;	
}

string FS::GetFullFTPPath()
{
    string config_dir = "sdmc:/atmosphere/titles/420000000000000E/flags/boot2.flag";
    return config_dir;	
}

string FS::GetFullTemplatePath()
{
    string config_dir = "sdmc:/";
    string basetitle ="/titles/0100000000001000";
    string source_path;
    string CurrentCFG_Folder;
    CurrentCFG_Folder = FS::GetCustomFirmwareFolder();
    string base_firmware;
    base_firmware = config_dir+CurrentCFG_Folder;
    source_path = base_firmware+basetitle;

    return source_path;	
}

bool FS::IsFTPEnabled()
{
    string source_path;
    source_path = FS::GetFullFTPPath();

    if(FS::CheckFileExists(source_path))
    {
	return true;
    }
    else
        return false;
}

bool FS::IsProdinfoRW()
{
    string source_path;
    source_path = FS::GetFullProdinfoPath();
    std::string allow_write = FS::GetProdinfoKeyValue(source_path);
    if(strcmp(allow_write.c_str(),"allow_write=0")==0)
	return false;
    else
	return true;
}

bool FS::IsTemplatedEnabled()
{
    string source_path;
    source_path = FS::GetFullTemplatePath();

	if(FS::DirExists(source_path))
	{
		FS::FixTemplateCrash(source_path);
        return true;
	}
    else
        return false;
}

string FS::TestFunction()
{

    string source_path;
    source_path = FS::GetFullTemplatePath();
    return source_path;
}

void FS::FixTemplateCrash(string path)
{
	string fixFile = "/romfs/lyt/Set.szs";
	string newFixedFile = "/romfs/lyt/Set.szs.old";
	int result=0;

	if(FS::GetFirmwareMajorVersion() < 6)
	{
		if(FS::CheckFileExists(path+fixFile))
		{
			result= rename( (path+fixFile).c_str() , (path+newFixedFile).c_str() );	
		}
		
	}
	else
	{
		if(FS::CheckFileExists(path+newFixedFile))
		{
			result= rename( (path+newFixedFile).c_str() , (path+fixFile).c_str() );	
		}
	}
}

void FS::DoEnableTemplate(string TemplateCode){
	int res = 0;
	string source_path;
    source_path = FS::GetFullTemplatePath();
    string TemplateA = "SB";
    string TemplateB = "RR";
    string TemplateChange = "";

    if(TemplateCode == "RR")
    	TemplateChange = TemplateA;
    if(TemplateCode == "SB")
    	TemplateChange = TemplateB;

    res = rename(source_path.c_str(),(source_path+"."+TemplateCode).c_str());
    res = rename((source_path+"."+TemplateChange).c_str(),source_path.c_str());
}

string FS::GetNXSerialNumber(){
	Result rc=0;
	std::string serialNumber;
    rc = setsysInitialize();
    char* sn = new char[0x30];
    rc = setsysGetSerialNumber(sn);
    if(R_FAILED(rc))
    	return "NOT FOUND";

    if(R_SUCCEEDED(rc))
    {
    	std::string sn_nx(sn);
    	serialNumber = sn_nx;
    }

    return serialNumber;
}

string FS::GetCustomFirmwareString()
{
	int id = FS::GetCustomFirmware_ID();
	std::string CFW_String = "";
	switch (id) {
		case 1: CFW_String = "Atmosphere";break;
		case 2: CFW_String = "ReiNX";break;
		case 3: CFW_String = "SX OS";break;
	}
	return CFW_String;
}

string FS::GetCustomFirmwareFolder()
{
	int id = FS::GetCustomFirmware_ID();
	std::string CFW_String = "";
	switch (id) {
		case 1: CFW_String = "atmosphere";break;
		case 2: CFW_String = "ReiNX";break;
		case 3: CFW_String = "sxos";break;
	}
	return CFW_String;
}
/*
* copy function
*/
bool FS::CopyFile(string origen, string destino) {
    clock_t start, end;
    start = clock();
    ifstream source(origen, ios::binary);
    ofstream dest(destino, ios::binary);

    dest << source.rdbuf();

    source.close();
    dest.close();

    end = clock();

    cout << "CLOCKS_PER_SEC " << CLOCKS_PER_SEC << "\n";
    cout << "CPU-TIME START " << start << "\n";
    cout << "CPU-TIME END " << end << "\n";
    cout << "CPU-TIME END - START " <<  end - start << "\n";
    cout << "TIME(SEC) " << static_cast<double>(end - start) / CLOCKS_PER_SEC << "\n";
    return 0;
}

bool FS::DirExists(string dirstr) {
    bool exist = false;
    DIR * dir = opendir(dirstr.c_str());
    if(dir) {
        closedir(dir);
        exist = true;
    }
    return exist;
}

unsigned FS::DeleteDirRecursive(string path) {
	int ret = -1;
	DIR * d = opendir(path.c_str());
	size_t path_len = strlen(path.c_str());
	if(d!=NULL) {
		struct dirent * de;
		ret = 0;
		
		while(!ret && (de=readdir(d))) {
			int status = -1;
			char * buf;
			size_t len;
			
			//skip over . and ..
			if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
				continue;
			}
			
			len = path_len + strlen(de->d_name);
			buf = (char *)malloc(len +2);
			
			if(buf!=NULL) {
				struct stat stator;
				snprintf(buf, len, "%s/%s", path.c_str(), de->d_name);
				
				if(!stat(buf, &stator)) {
					if(S_ISDIR(stator.st_mode)) {
						status = DeleteDirRecursive(buf);
					}
					else {
					status = unlink(buf);
					}
				}
				free(buf);
			}
			ret = status;
		}
		closedir(d);
	}
	
	if(!ret) {
		ret = rmdir(path.c_str());
	}
	
	return 0;
}

void FS::SetProdinfoMode(int value)
{
   FS::WriteLineFile(FS::GetFullProdinfoPath(), "[config]\nallow_write="+std::to_string(value));
}

void FS::SetFTPStatus(bool value)
{
   if(value)
	FS::WriteLineFile(FS::GetFullFTPPath(), "");
   else
	FS::DeleteFile(FS::GetFullFTPPath());	
}

unsigned FS::MakeDir(string file, unsigned perm) {
    return mkdir(file.c_str(), perm);
}
