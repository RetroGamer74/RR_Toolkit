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

#include <switch.h>
#include "Net/Request.hpp"
#include "Utils/unzip_utils.hpp"
#include "FS.hpp"
#include "UI.hpp"
#include <stdio.h>
#include <fstream>
#include <stdbool.h>
#include "Utils/reboot_class.hpp"

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720
#define RR_MINVERSION 124
#define RR_MINVERSIONPRODINFO 133
#define RR_MINVERSIONFTP 135

static u32 currSel = 0;
static u32 currSubSel = 0;
static u32 vol = 64;
static u32 titleX = 60;
static u32 titleY = 30;
static u32 menuX = 55, menuY = 115;
static u32 subX = 411, subY = 88;

static u64 HeldInput = 0;
static u64 PressedInput = 0;
static u64 ReleasedInput = 0;

static string title;
static string version;
static string current_RR_version= "1.12"; //Retrieved from release txt file
static string CurrentCFG_Name; //Custom Firmware Name
static string CurrentCFG_Folder; //Custom Firmware Folder
static string CurrentTemplate; //Current Active Template
static string CurrentNXFirmwareVersion; //Switch Firmware Version
static string SerialNumber; //Switch Serial Number
static string SXOS_Fail = "Not available in SX OS";
static u32 CurrentFirmware_ID; //Current CFW ID
static bool TemplateEnabled;
static bool ProdinfoRW;
static bool isFTPEnabled = false;
static bool isLDNEnabled = false;
static bool isAMIBOEnabled = false;
static int RRReleaseNumber;
static string MessageIssue = "";
static string FTPStatus = "Off";
static string LDNStatus = "Off";
static string AMIBOStatus = "Off";
static string TemplateCapableStatus = "Off";

//static u32 menuOptionCFWUpdate = 0;
static u32 menuOptionReboot = 0;
static u32 menuOptionThemeEnable = 1;
static u32 menuOptionSelectTheme = 2;
static u32 menuOptionProdinfo = 3;
static u32 menuOptionService = 4;
static u32 menuOptionAbout = 5;

static bool can_reboot = true;

u32 clippy = 0;

vector<MenuOption> mainMenu;
vector<SDL_Surface*> images;

Mix_Music *menuSel;
Mix_Music *menuConfirm;
Mix_Music *menuBack;

UI * UI::mInstance = 0;

/* ---------------------------------------------------------------------------------------
* Menu functions
*/
void UI::optRRUpdate() {
    ProgBar prog;
    prog.max = 5;
    prog.step = 1;
    string latest_release_url = "http://switch.retrogamer.tech/glr.php";
    CreateProgressBar(&prog, "Updating RetroReloaded...");

    string config_dir = "sdmc:";
    string config_path = config_dir + "/RR_version.txt";
   

    current_RR_version = version;
    
    Net net = Net();
    hidScanInput();
/*
    if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_L) {
        if(MessageBox("Warning!", "You are about to do a clean install.\nThis deletes the ReiNX folder!", TYPE_YES_NO)) {
            FS::DeleteDirRecursive("./ReiNX");
        }
    }
*/
    string new_release = "";
    new_release = net.Request("GET",latest_release_url);
    new_release = net.readBuffer;
    string filename = "/RR_"+new_release+".zip";
    string url = "http://switch.retrogamer.tech/RR_"+new_release+".zip";

if(current_RR_version != new_release)
{
if(MessageBox("Warning!", "The latest release is "+ new_release + ". \nAdvise:Update thru Github is best option.\nThis procedure will take some time, and \nyou can feel it is frozen. Just wait :-)\nAre you ready to go?", TYPE_YES_NO)) {
    IncrementProgressBar(&prog);
    bool res = net.Download(url,filename );
    IncrementProgressBar(&prog);
    if(!res){
        appletBeginBlockingHomeButton(0);
        unzFile zip = Utils::zip_open(filename.c_str()); IncrementProgressBar(&prog);
        Utils::zip_extract_all(zip, "/"); IncrementProgressBar(&prog);
        Utils::zip_close(zip); IncrementProgressBar(&prog);
        remove(filename.c_str());
        appletEndBlockingHomeButton();
        MessageBox("Update", "Update "+new_release+" applied successfully!", TYPE_OK);
    }else{
        prog.curr = 4;
        MessageBox("Update", "Update not applied!", TYPE_OK);
    }
 }
}
else
{
    MessageBox(
        "You're up to date",
        "You're currently updated to latest release.",
    TYPE_OK);
}
}

/*
* SubMenus
*/

//Help

void UI::optInfoSXOS() {
	MessageBox("Not available","Feature not available in SX OS",TYPE_OK);
}

void UI::optAbout() {
    string mode = "";
    string ftpStatus = "";
    string ldnStatus = "";
    string amiboStatus = "";
    if(FS::IsProdinfoRW())
	mode = "RW";
    else
	mode = "RO";

    if(FS::IsFTPEnabled())
	ftpStatus="Enabled";
    else
        ftpStatus="Disabled";
    if(FS::IsLDNEnabled())
	ldnStatus="Enabled";
    else
        ldnStatus="Disabled";
    if(FS::IsAMIBOEnabled())
	amiboStatus="Enabled";
    else
        amiboStatus="Disabled";

    MessageBox(
        "About", 
        "NX FW Version: "+CurrentNXFirmwareVersion +"\n"+
        "SN: "+SerialNumber+"\n"+
        "RR Version: " + current_RR_version + 
        "\n" +
        "CFW: "+ CurrentCFG_Name+"\n"+
	"PRODINFO MODE: "+ mode + "\n"+
	"SYS-FTP STATUS: "+ ftpStatus + "\n"+
	"LANPLAY STATUS: "+ ldnStatus + "\n"+
	"EMUIIBO STATUS: "+ amiboStatus + "\n"+
	
        "Template: "+CurrentTemplate+"\n"+
        //" Located: sdmc:/"+CurrentCFG_Folder+" FWD_ID: "+std::to_string(CurrentFirmware_ID)+"\n"+
        "Main developers:\n" +
        "RetroGamer_74\n" ,
    TYPE_OK);
}


void setServicesStatusStrings()
{
	if(isFTPEnabled)
		FTPStatus = "On";
	else
		FTPStatus = "Off";

	if(isLDNEnabled)
		LDNStatus = "On";
	else
		LDNStatus = "Off";

	if(TemplateEnabled)
		TemplateCapableStatus = "On";
	else
		TemplateCapableStatus = "Off";

	if(isAMIBOEnabled)
		AMIBOStatus = "On";
	else
		AMIBOStatus = "Off";
}

void setMenuOptionValues()
{
	
	setServicesStatusStrings();

	if(RRReleaseNumber < RR_MINVERSION)
	{
		menuOptionAbout = 1;
		return;
	}

	if(RRReleaseNumber >= RR_MINVERSION && RRReleaseNumber < RR_MINVERSIONPRODINFO)
	{
		menuOptionAbout = 4;
		return;
	}

	if(RRReleaseNumber >= RR_MINVERSIONPRODINFO && RRReleaseNumber < RR_MINVERSIONFTP)
	{
		menuOptionAbout = 5;
		return;
	}
}


void UI::optReboot() {

    if(can_reboot)
	Reboot::DoReboot();
    else
	MessageBox("Info","Reboot didn't work",TYPE_OK);	
}


void UI::optDisableProdinfo() {
 
  bool isProdinfoRW = FS::IsProdinfoRW();
  string RO = "Read Only";
  string RW = "Read / Write";

  if(CurrentCFG_Name != "Atmosphere")
  {
     MessageBox("Info","This feature is only available\nbooting Atmosphere.",TYPE_OK);
     return; 
  }

  string question = "";

  if(isProdinfoRW)
	question = RO;
  else
	question = RW;

    if(MessageBox("Prodinfo Mode","Do you want to set Prodinfo in \n"+ question +" mode?", TYPE_YES_NO)) {

        if(isProdinfoRW)
        {
            FS::SetProdinfoMode(0);
            MessageBox("Info","Prodinfo has been set as "+RO+"\nReboot is required to apply changes!",TYPE_OK);
        }
        else
        {
            FS::SetProdinfoMode(1);
            MessageBox("Info","Prodinfo has been set as "+RW+"\nReboot is required to apply changes!",TYPE_OK);
        }

    }

}

void UI::optDisableFTP() {
 
  isFTPEnabled = FS::IsFTPEnabled();
  string enable = "enable";
  string disable = "disable";

  string question = "";

  if(isFTPEnabled)
	question = disable;
  else
	question = enable;

    if(MessageBox("FTP On/Off","Do you want to "+question+"\nFTP as system service?", TYPE_YES_NO)) {

        if(isFTPEnabled)
        {
            FS::SetFTPStatus(false);
            MessageBox("Info","FTP has been set to disabled.\nReboot is required to apply changes!",TYPE_OK);
        }
        else
        {
            FS::SetFTPStatus(true);
            MessageBox("Info","FTP has been set to enabled.\nReboot is required to apply changes!",TYPE_OK);
        }

        isFTPEnabled = FS::IsFTPEnabled();
	setServicesStatusStrings();
	UI::drawServicesOption();
    }

}
void UI::optDisableLDN() {
 
  isLDNEnabled = FS::IsLDNEnabled();
  string enable = "enable";
  string disable = "disable";

  string question = "";

  if(isLDNEnabled)
	question = disable;
  else
	question = enable;

    if(MessageBox("LanPlay On/Off","Do you want to "+question+"\nLanPlay as system service?", TYPE_YES_NO)) {

        if(isLDNEnabled)
        {
            FS::SetLDNStatus(false);
            MessageBox("Info","LanPlay has been set to disabled.\nReboot is required to apply changes!",TYPE_OK);
        }
        else
        {
            FS::SetLDNStatus(true);
            MessageBox("Info","LanPlay has been set to enabled.\nReboot is required to apply changes!",TYPE_OK);
        }

	isLDNEnabled = FS::IsLDNEnabled();
	setServicesStatusStrings();
	UI::drawServicesOption();
    }

}
void UI::optDisableAMIBO() {
 
  isAMIBOEnabled = FS::IsAMIBOEnabled();
  string enable = "enable";
  string disable = "disable";

  string question = "";

  if(isAMIBOEnabled)
	question = disable;
  else
	question = enable;

    if(MessageBox("EMUIIBO On/Off","Do you want to "+question+"\nEmuiibo as system service?", TYPE_YES_NO)) {

        if(isAMIBOEnabled)
        {
            FS::SetAMIBOStatus(false);
            MessageBox("Info","Emuiibo has been set to disabled.\nReboot is required to apply changes!",TYPE_OK);
        }
        else
        {
            FS::SetAMIBOStatus(true);
            MessageBox("Info","Emuiibo has been set to enabled.\nReboot is required to apply changes!",TYPE_OK);
        }

	isAMIBOEnabled = FS::IsAMIBOEnabled();
	setServicesStatusStrings();
	UI::drawServicesOption();
    }

}


//remove template
void UI::optDisableTemplate() {
    int result;
    string config_dir = "sdmc:/";
    string basetitle ="/titles/0100000000001000";
    string newtitle ="/titles/0100000000001000."+CurrentTemplate;

    string source_path;
    string target_path;

    string base_firmware;

    base_firmware = config_dir+CurrentCFG_Folder;

    source_path = base_firmware+basetitle;
    target_path = base_firmware+newtitle;

    bool enabledTemplate=false;

    //DIR *dir = opendir(atmosphere.c_str());
    //if(dir != NULL)
    if(FS::DirExists(source_path))
        enabledTemplate=true;
    else
        enabledTemplate=false;

    string enableText="enable";
    string disableText="disable";

    string questionText="";
    if(enabledTemplate)
        questionText = disableText;
    else
        questionText = enableText;

    char c=questionText[0];
    string titleQuestionText = questionText;
    titleQuestionText[0] = toupper(c);

    if(MessageBox(titleQuestionText+" template", "Would you like to "+questionText+" themes?", TYPE_YES_NO)) {

        if(enabledTemplate)
        {
            result= rename( source_path.c_str() , target_path.c_str() );
            MessageBox("Info","Themes feature has been disabled",TYPE_OK);
        }
        else
        {
            result= rename( target_path.c_str() , source_path.c_str() );
            MessageBox("Info","Themes feature has been enabled",TYPE_OK);
        }
 
        TemplateEnabled = FS::IsTemplatedEnabled();
        setServicesStatusStrings();
        UI::drawTemplateCapableOption();
    }
}

void UI::RePaintMenu()
{
    string menuOptionProdInfoTitle = "RO/RW Prodinfo";

    if(CurrentCFG_Name == "SX OS")
	menuOptionProdInfoTitle = SXOS_Fail;


    setServicesStatusStrings();
    mainMenu.clear();

        //Main pages
    //mainMenu.push_back(MenuOption("RR Updates", "Update RR Now!.", nullptr));
    mainMenu.push_back(MenuOption("Warm Reboot", "RR. Reboot!.", nullptr));

    if(RRReleaseNumber >= RR_MINVERSION)
    {
        mainMenu.push_back(MenuOption("Themes Capable","Showing current status.",nullptr));
        mainMenu.push_back(MenuOption("Select Themes","Select Template",nullptr));

    }
    if(RRReleaseNumber >= RR_MINVERSIONPRODINFO)
    {
        mainMenu.push_back(MenuOption("Prodinfo RW",menuOptionProdInfoTitle,nullptr));
    }
    if(RRReleaseNumber >= RR_MINVERSIONFTP)
    {
	if(CurrentCFG_Name == "SX OS")
	        mainMenu.push_back(MenuOption("Services",SXOS_Fail,bind(&UI::optInfoSXOS, this)));
	else
	        mainMenu.push_back(MenuOption("Services","Showing current status.",nullptr));

    }


    mainMenu.push_back(MenuOption("About", "About RetroReloaded Updater.",  bind(&UI::optAbout, this)));

    
    
    //Subpages
//    mainMenu[menuOptionCFWUpdate].subMenu.push_back(MenuOption("Update RR Now", "", bind(&UI::optRRUpdate, this)));
    mainMenu[menuOptionReboot].subMenu.push_back(MenuOption("RR. Reboot!", "", bind(&UI::optReboot, this)));

//    mainMenu[menuOptionCFWUpdate].subMenu.push_back(MenuOption("Update RR NRO", "", bind(&UI::optUpdateHB, this)));
    if(RRReleaseNumber >= RR_MINVERSION)
    {
        UI::drawTemplateCapableOption();
        UI::drawTemplatesOption();
    }

    if(RRReleaseNumber >= RR_MINVERSIONPRODINFO)
    {
        mainMenu[menuOptionProdinfo].subMenu.push_back(MenuOption("Prodinfo RO/RW", "",bind(&UI::optDisableProdinfo, this)));
    }

    if(RRReleaseNumber >= RR_MINVERSIONFTP)
    {
	if(CurrentCFG_Name != "SX OS")
		UI::drawServicesOption();
    }
}


void UI::drawTemplateCapableOption()
{
	mainMenu[menuOptionThemeEnable].subMenu.clear();
        mainMenu[menuOptionThemeEnable].subMenu.push_back(MenuOption("Themes => "+TemplateCapableStatus, "", bind(&UI::optDisableTemplate, this)));    


}

void UI::drawServicesOption()
{
	mainMenu[menuOptionService].subMenu.clear();
        mainMenu[menuOptionService].subMenu.push_back(MenuOption("FTP => "+FTPStatus, "",bind(&UI::optDisableFTP, this)));
        mainMenu[menuOptionService].subMenu.push_back(MenuOption("LanPlay => "+LDNStatus, "",bind(&UI::optDisableLDN, this)));
        mainMenu[menuOptionService].subMenu.push_back(MenuOption("Emuiibo => "+AMIBOStatus, "",bind(&UI::optDisableAMIBO, this)));
}

void UI::drawTemplatesOption(){
    TemplateEnabled = FS::IsTemplatedEnabled();
    string TemplateName;
    mainMenu[menuOptionSelectTheme].subMenu.clear();
    if(TemplateEnabled)
    {
        TemplateName = FS::GetToggleMarkWithTemplateName("RR");
        mainMenu[menuOptionSelectTheme].subMenu.push_back(MenuOption(TemplateName, "", bind(&UI::EnableRRTheme, this)));
        TemplateName = FS::GetToggleMarkWithTemplateName("SB");
        mainMenu[menuOptionSelectTheme].subMenu.push_back(MenuOption(TemplateName, "", bind(&UI::EnableSBTheme, this)));    
    }
    else
    {
        mainMenu[menuOptionSelectTheme].subMenu.push_back(MenuOption("Themes Capable", "", bind(&UI::EnableTemplateCapable, this)));        
    }



}

void UI::EnableTemplateCapable(){
    MessageBox("Info","In order to choose a theme,\nset Themes Capable to ON",TYPE_OK);
    UI::drawTemplatesOption();
}

void UI::EnableRRTheme() {
    if(CurrentTemplate != "RR")
    {
        FS::WriteLineFile("sdmc:/"+CurrentCFG_Folder+"/template.txt", "RR");
        MessageBox("Info","RetroReloaded Template has been set.\nYou must reboot to activate it.",TYPE_OK);
        FS::DoEnableTemplate("SB");
        UI::drawTemplatesOption();
    }
}

void UI::EnableSBTheme() {
    if(CurrentTemplate != "SB")
    {
        FS::WriteLineFile("sdmc:/"+CurrentCFG_Folder+"/template.txt", "SB");
        MessageBox("Info","SuperBan Template has been set.\nYou must reboot to activate it.",TYPE_OK);
        FS::DoEnableTemplate("RR");
        UI::drawTemplatesOption();
    }
}

/*
void UI::optUpdateHB() {
    ProgBar prog;
    prog.max = 1;
    prog.step = 1;
    string url = "http://builds.reinx.guide/toolkit/ReiNXToolkit-latest.nro";

    if (!MessageBox("Update", 
      "This will attempt to update the Toolbox.\nAfter updating, the app will exit.\n\nContinue?", 
      TYPE_YES_NO))
        return;

    appletBeginBlockingHomeButton(0);
    CreateProgressBar(&prog, "Updating Toolkit...");
    
    Net net = Net();
    if (net.Download(url, "/switch/ReiNXToolkit_new.nro")){
        prog.curr = 1;
        appletEndBlockingHomeButton();
        MessageBox("Update", "Update unsuccessful!", TYPE_OK);
        return;
    }

    IncrementProgressBar(&prog);
    romfsExit();
    remove("/switch/ReiNXToolkit.nro");
    rename("/switch/ReiNXToolkit_new.nro", "/switch/ReiNXToolkit.nro");
    fsdevCommitDevice("sdmc");
    exitApp();
}
*/

/* ---------------------------------------------------------------------------------------
* UI class functions
*/
UI::UI(string Title, string Version) {
    romfsInit();
    fsdevMountSdmc();
    socketInitializeDefault();
#ifdef DEBUG
    nxlinkStdio();
    printf("printf output now goes to nxlink server\n");
#endif
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIN_WIDTH, WIN_HEIGHT, 0, &mRender._window, &mRender._renderer);
    mRender._surface = SDL_GetWindowSurface(mRender._window);
    SDL_SetRenderDrawBlendMode(mRender._renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    SDL_SetRenderDrawColor(mRender._renderer, 255, 255, 255, 255);
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
    Mix_VolumeMusic(vol);
    mThemes = Themes::instance();
    mThemes->Init(mRender);
    inSubMenu = false;

    title = Title;
    version = Version;
    
    menuSel = Mix_LoadMUS("romfs:/Sounds/menu_select.mp3");
    menuConfirm = Mix_LoadMUS("romfs:/Sounds/menu_confirm.mp3");
    menuBack = Mix_LoadMUS("romfs:/Sounds/menu_back.mp3");
    
    UI::RePaintMenu();

}

void UI::setInstance(UI ui) {
    mInstance = &ui;
    CurrentCFG_Name = FS::GetCustomFirmwareString();
    CurrentCFG_Folder = FS::GetCustomFirmwareFolder();
    CurrentFirmware_ID = FS::GetCustomFirmware_ID() ;
    CurrentTemplate = FS::GetDefaultTemplate();
    version = FS::GetRRRelease();
    CurrentNXFirmwareVersion = FS::SwitchIdent_GetFirmwareVersion();
    SerialNumber = FS::GetNXSerialNumber();
    current_RR_version = version;
    RRReleaseNumber = FS::GetRRReleaseNumber();

    TemplateEnabled = FS::IsTemplatedEnabled();
    ProdinfoRW = FS::IsProdinfoRW();
    isFTPEnabled = FS::IsFTPEnabled();
    isLDNEnabled = FS::IsLDNEnabled();
    isAMIBOEnabled = FS::IsAMIBOEnabled();
    setMenuOptionValues();

    if(RRReleaseNumber < RR_MINVERSION)
        ui.MessageBox("Warning","Some features are disabled because\nyour RR version is lower than 1.24.\nPlease update!",TYPE_OK);

    can_reboot=Reboot::InitalizeRebootFeature();
}

UI *UI::getInstance() {
    if(!mInstance)
        return NULL;
    return mInstance;
}

void UI::deinit() {
    TTF_Quit();
    IMG_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyRenderer(mRender._renderer);
    SDL_FreeSurface(mRender._surface);
    SDL_DestroyWindow(mRender._window);
    SDL_Quit();
    romfsExit();
    socketExit();
    fsdevUnmountAll();
}

void UI::exitApp() {
    deinit();
    appletEndBlockingHomeButton(); // make sure we don't screw up hbmenu
    Reboot::ExitRebootFeature(can_reboot);
    ::exit(0);
}

/*
* UI draw functions
*/
void UI::drawText(int x, int y, SDL_Color scolor, string text, TTF_Font *font) {
    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), scolor, 1280);
    SDL_SetSurfaceAlphaMod(surface, 255);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRender._renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect position;
    position.x = x;
    position.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
    SDL_RenderCopy(mRender._renderer, texture, NULL, &position);
    SDL_DestroyTexture(texture);
}

void UI::drawRect(int x, int y, int w, int h, SDL_Color scolor, unsigned border, SDL_Color bcolor) {
    drawRect(x-border, y-border, w+(2*border), h+(2*border), bcolor);
    drawRect(x, y, w, h, scolor);
}

void UI::drawRect(int x, int y, int w, int h, SDL_Color scolor) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawColor(mRender._renderer, scolor.r, scolor.g, scolor.b, scolor.a);
    SDL_RenderFillRect(mRender._renderer, &rect);
}

void UI::drawBackXY(SDL_Surface *surf, SDL_Texture *tex, int x, int y) {
    SDL_Rect position;
    position.x = x;
    position.y = y;
    position.w = surf->w;
    position.h = surf->h;
    SDL_RenderCopy(mRender._renderer, tex, NULL, &position);
}

void UI::drawScaled(SDL_Surface *surf, SDL_Texture *tex, int x, int y, u32 w, u32 h) {
    SDL_Rect position;
    position.x = x;
    position.y = y;
    position.w = w;
    position.h = h;
    SDL_RenderCopy(mRender._renderer, tex, NULL, &position);
}

/*
* UI Pop-up stuff
*/
void UI::CreatePopupBox(u32 x, u32 y, u32 w, u32 h, string header) {
    drawRect(x-5, y-5, w+10, h+70, {0, 0, 0, 0xFF}); //BG box
    drawRect(x, y, w, 57, mThemes->popCol1); //Header
    drawRect(x, y+60, w, h, mThemes->popCol2); //Message
    drawText(x+15, y+15, mThemes->txtcolor, header, mThemes->fntMedium);
}

void UI::CreateProgressBar(ProgBar *prog, string header) {
    u32 poph = 100, popw = 300;
    u32 barh = 40, barw = 200;
    u32 startx = (WIN_WIDTH/2)-(popw/2), starty = (WIN_HEIGHT/2)-(poph/2);
    
    if(prog->max <= 0) prog->max = 100;
    if(prog->step <= 0) prog->step = 1;
    prog->curr = 0;
    prog->posx = startx;
    prog->posy = starty;
    CreatePopupBox(prog->posx, prog->posy, popw, poph, header);
    drawRect(prog->posx+(popw/2)-(barw/2), prog->posy+60+((poph-60)/2), barw, barh, {0, 0, 0, 0xFF}); //empty bar
    drawText(prog->posx+(popw/2), prog->posy+(poph/2)-(barh/2)+60, {0xFF,0xFF,0xFF,0xFF}, to_string((unsigned)((prog->curr/prog->max)*100)) + "%", mThemes->fntSmall);
    SDL_RenderPresent(mRender._renderer);
}

void UI::IncrementProgressBar(ProgBar *prog) {
    u32 poph = 100+2, popw = 300+2;
    u32 barh = 40-4, barw = 200-4;
    drawRect(prog->posx+(popw/2)-(barw/2), prog->posy+60+((poph-60)/2), barw, barh, {0, 0, 0, 0xFF}); //empty bar
    drawRect(prog->posx+(popw/2)-(barw/2), prog->posy+60+((poph-60)/2), (unsigned)(barw*(prog->curr/prog->max)), barh, {0, 0xFF, 0, 0xFF}); //green bar
    drawText(prog->posx+(popw/2), prog->posy+(poph/2)-(barh/2)+60, {0xFF,0xFF,0xFF,0xFF}, to_string((unsigned)((prog->curr/prog->max)*100)) + "%", mThemes->fntSmall);
    prog->curr += prog->step;
    SDL_RenderPresent(mRender._renderer);
}

bool UI::MessageBox(string header, string message, MessageType type) {
    bool ret = false;
    SDL_Rect rect;
    u32 poph = 300, popw = 450;
    u32 buth = 50, butw = 100;
    u32 startx = (WIN_WIDTH/2)-(popw/2), starty = (WIN_HEIGHT/2)-(poph/2);
    
    CreatePopupBox(startx, starty, popw, poph, header);
    drawText(startx+15, starty+75, mThemes->txtcolor, message, mThemes->fntMedium);
    
    switch(type){
        case TYPE_YES_NO:
            drawRect(startx+popw-butw-10, starty+poph, butw, buth, mThemes->popCol1, 3, {0, 0, 0, 0xFF}); //YES
            drawText(startx+popw-butw+12, starty+poph+12, mThemes->txtcolor, "\ue0e0 Yes", mThemes->fntMedium);
            drawRect(startx+popw-(2*(butw+5))-10, starty+poph, butw, buth, mThemes->popCol1, 3, {0, 0, 0, 0xFF}); //NO
            drawText(startx+popw-(2*(butw+5))+12, starty+poph+12, mThemes->txtcolor, "\ue0e1 No", mThemes->fntMedium);
            break;
        default:
        case TYPE_OK:
            drawRect(startx+popw-butw-10, starty+poph, butw, buth, mThemes->popCol1, 3, {0, 0, 0, 0xFF}); //OK
            drawText(startx+popw-butw+12, starty+poph+12, mThemes->txtcolor, "\ue0e0 OK", mThemes->fntMedium);
            break;
    }
    SDL_RenderPresent(mRender._renderer);
    while(1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if(kDown & KEY_A) {
            ret = true;
            Mix_PlayMusic(menuConfirm, 1);
            break;
        }
        else if(kDown & KEY_B) {
            ret = (type == TYPE_OK);
            Mix_PlayMusic(menuBack, 1);
            break;
        }
    }
    return ret;
}

/*
* Render function
*/
void UI::renderMenu() {
    UI::RePaintMenu();
    //if(TemplateEnabled)
    //        MessageBox("Title","Template TRUE "+FS::TestFunction(),TYPE_OK);

    SDL_RenderClear(mRender._renderer);
    drawBackXY(mThemes->bgs, mThemes->bgt, 0, 0);
    //Mainmenu  text
//    drawText(titleX, titleY, mThemes->txtcolor, title, mThemes->fntLarge);
    int oy = menuY;
    if(!mainMenu.empty()) for(unsigned int i = 0; i < mainMenu.size(); i++){
        //Mainmenu buttons
        if(i == currSel && !inSubMenu) {
            drawRect(menuX-10, oy-10, 210+20, 25+20, mThemes->butCol, 5, mThemes->butBorderCol);
            drawText(menuX, oy, mThemes->selcolor, mainMenu[i].getName(), mThemes->fntMedium);
        } else {
            drawText(menuX, oy, mThemes->txtcolor, mainMenu[i].getName(), mThemes->fntMedium);
        }
        
        if(i == currSel) {
            //Submenu text
            drawText(subX + 30, subY + 30, mThemes->txtcolor, mainMenu[i].getDesc(), mThemes->fntSmall);
            if(!mainMenu[i].subMenu.empty()) for(unsigned int j = 0; j < mainMenu[i].subMenu.size(); j++){
                //Submenu buttons
                if(j == currSubSel && inSubMenu) {
                    drawRect(subX + 10, subY + 20 + ((j+1)*50), 200+20, 40, mThemes->butCol, 3, mThemes->butBorderCol);
                    drawText(subX + 30, subY + 30 + ((j+1)*50), mThemes->selcolor, mainMenu[i].subMenu[j].getName(), mThemes->fntMedium);
                }else{
                    drawText(subX + 30, subY + 30 + ((j+1)*50), mThemes->txtcolor, mainMenu[i].subMenu[j].getName(), mThemes->fntMedium);
                }
                if(j == currSubSel && currSel == menuOptionAbout) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(mRender._renderer, images[currSubSel]);
                    drawScaled(images[currSubSel], tex, 710, 120, images[currSubSel]->w/3, images[currSubSel]->h/3);
                }
            }
        }
        oy += 50;
    }
    SDL_RenderPresent(mRender._renderer);
}

/*
* Menu nav functions
*/
void UI::MenuUp() {
    Mix_PlayMusic(menuSel, 1);
    if(currSel > 0) currSel--;
    else currSel = mainMenu.size() - 1;
    currSubSel = 0;
}

void UI::MenuDown() {
    Mix_PlayMusic(menuSel, 1);
    if((unsigned int) currSel < mainMenu.size() - 1) currSel++;
    else currSel = 0;
    currSubSel = 0;
}

void UI::SubMenuUp() {
    u32 subSize = mainMenu[currSel].subMenu.size();
    if(subSize > 1) Mix_PlayMusic(menuSel, 1);
    if(currSubSel > 0) currSubSel--;
    else currSubSel = subSize - 1;
}

void UI::SubMenuDown() {
    u32 subSize = mainMenu[currSel].subMenu.size();
    if(subSize > 1) 
        Mix_PlayMusic(menuSel, 1);
    if((unsigned int) currSubSel < subSize - 1) 
        currSubSel++;
    else currSubSel = 0;
}

void UI::MenuSel() {
    Mix_PlayMusic(menuConfirm, 1);
    if(!inSubMenu){
        if(mainMenu[currSel].subMenu.empty()) 
            mainMenu[currSel].callFunc();
        else 
            inSubMenu = true;
    }else{
        mainMenu[currSel].subMenu[currSubSel].callFunc();
    }
}

void UI::MenuBack() {
    Mix_PlayMusic(menuBack, 1);
    if(inSubMenu)
        inSubMenu = false;
    else
        exitApp();
}

