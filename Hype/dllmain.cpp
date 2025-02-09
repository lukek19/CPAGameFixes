#include <cmath>
#include <cstdint>
#include <fstream>
#include <inipp.h>
#include <safetyhook.hpp>
#include <windows.h>


// Variables declared in Hype.ini
bool PatchFPSCap = false;
bool centerHUD = false;
uint32_t FPSValue = 50;
uint32_t xRes = 640;
uint32_t yRes = 480;
bool doRemoveCDCheck = true;
bool doMakePortable = true;

// Variables computed during configuration
unsigned char GameVersion = 0; // 0 unknown / 1 English Patch / 2 Spanish
bool skipFix = true;
float ARScale;
float xScale;
float yScale;
uint32_t OffsetBackground;
uint32_t OffsetLoadBar;

// Declare addresses (are set during configuration)
uint32_t addressGetCurrentDirectoryA;
uint8_t* addressCDCheck1;
uint8_t* addressCDCheck2;
uint8_t* addressCDCheck3;
uint8_t* addressCDCheck4;
uint8_t* addressCDCheck5;
uint8_t* addressCDCheck6;
uint8_t* addressCDCheck7;
uint8_t* addressCDCheck8;
uint8_t* addressAccessUbiIni1;
uint8_t* addressAccessUbiIni2;
uint8_t* addressStringUbiIni;
uint8_t* addressFPSValue;
uint8_t* addressFPSCapFloor;
uint8_t* addressResolution;
uint8_t* addressAdjustCameraToViewport;
uint8_t* addressCallBlitStretched16b;
uint32_t* addressIntLoadBarShift;
uint8_t* addressSubtractLoadBarShift1;
uint8_t* addressSubtractLoadBarShift2;
uint8_t* addressDraw2DSpriteWithUV;
uint8_t* addressTransformCoordinates;
uint8_t* addressDrawNumberOfEnergyPrisms;
uint8_t* addressDrawTextOnInventoryScreen;
uint8_t* addressCalibrate2D;
uint8_t* addressDrawMagicGauge;
uint8_t* addressDrawText;
uint8_t* addressDrawMap;
uint8_t* addressDrawMap2;

// Declare hooks
SafetyHookMid hook01{};
SafetyHookMid hook02{};
SafetyHookMid hook03{};
SafetyHookMid hook04{};
SafetyHookMid hook05{};
SafetyHookMid hook06{};
SafetyHookMid hook07{};


// Helper functions
template<typename T>
void Write(std::uint8_t* writeAddress, T value)
{
	DWORD oldProtect;
	VirtualProtect((LPVOID)(writeAddress), sizeof(T), PAGE_EXECUTE_WRITECOPY, &oldProtect);
	*(reinterpret_cast<T*>(writeAddress)) = value;
	VirtualProtect((LPVOID)(writeAddress), sizeof(T), oldProtect, &oldProtect);
}

void PatchBytes(std::uint8_t* address, const char* pattern, unsigned int numBytes)
{
	DWORD oldProtect;
	VirtualProtect((LPVOID)address, numBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((LPVOID)address, pattern, numBytes);
	VirtualProtect((LPVOID)address, numBytes, oldProtect, &oldProtect);
}

// Hooks

// FOV
void FOVMidHook(SafetyHookContext& ctx) {
	float NewFOV = 2.0f * atanf(tanf(*reinterpret_cast<float*>(ctx.eax + 0x64) / 2.0f) * ARScale);
	_asm {fld NewFOV}
}

// Backgrounds and load bar (centered only)
void VignetteHook(SafetyHookContext& ctx) {
	int WSrc = *reinterpret_cast<int*>(ctx.esp + 0x10);
	// Divide WDst by ARScale
	*reinterpret_cast<int*>(ctx.esp) = static_cast<int>(round(static_cast<float>(*reinterpret_cast<int*>(ctx.esp)) / ARScale));
	/* If WSrc = 640 (background image), shift BufferDst by OffsetBackground
	   If WSrc = 512 (loading bar), shift BufferDst by OffsetLoadBar */
	if (WSrc == 640) {
		*reinterpret_cast<int*>(ctx.esp + 0x8) = *reinterpret_cast<int*>(ctx.esp + 0x8) + OffsetBackground;
	}
	else if (WSrc == 512) {
		*reinterpret_cast<int*>(ctx.esp + 0x8) = *reinterpret_cast<int*>(ctx.esp + 0x8) + OffsetLoadBar;
	}
}

// Load bar shift
/* After the infos about the load bar are read from <levelname>pgb.bin, the game correctly scales the
	coordinates (xMin, xMax, yMin, yMax) with the new resolution. There is also an x-shift (in pixels),
	which the game does not scale. This hook fixes this. */
void SubtractLoadBarShiftHook(SafetyHookContext& ctx) {
	ctx.edx = ctx.edx - static_cast<int>(round(static_cast<float>(*addressIntLoadBarShift) * xScale));
}

// 2D sprites (centered)
void Draw2DSpriteWithUVHookCentered(SafetyHookContext& ctx) {
	// Prevent fade-in effect from being centered
	if ((*reinterpret_cast<float*>(ctx.esp + 0x08) != 0.0f || *reinterpret_cast<float*>(ctx.esp + 0x0c) != 640.0f))
	{
		*reinterpret_cast<float*>(ctx.esp + 0x08) = (*reinterpret_cast<float*>(ctx.esp + 0x08) - 320.0f) / ARScale + 320.0f;
		*reinterpret_cast<float*>(ctx.esp + 0x0c) = (*reinterpret_cast<float*>(ctx.esp + 0x0c) - 320.0f) / ARScale + 320.0f;
	}
}

/* 2D sprites (stretched)
   Note: If the 2D sprite is rotated by +/- 90 degrees (+/- PI/2) then it gets stretched vertically
	instead of horizontally. This hook un-stretches it verticallly and stretches it horizontally.
	Fixes some menu side bars (e.g. escape menu & do-you-really-want-to-quit-the-game menu) */ 
void Draw2DSpriteWithUVHookStretched(SafetyHookContext& ctx) {
	if (abs(*reinterpret_cast<float*>(ctx.esp + 0x28)) == 1.570796326794896f)
	{	
		float xShift = (*reinterpret_cast<float*>(ctx.esp + 0x0c) - *reinterpret_cast<float*>(ctx.esp + 0x08)) * (1.0f - 1.0f / ARScale) / 2.0f;
		float yShift = (*reinterpret_cast<float*>(ctx.esp + 0x14) - *reinterpret_cast<float*>(ctx.esp + 0x10)) * (ARScale - 1.0f) / 2.0f;
		*reinterpret_cast<float*>(ctx.esp + 0x08) = *reinterpret_cast<float*>(ctx.esp + 0x08) + xShift;
		*reinterpret_cast<float*>(ctx.esp + 0x0c) = *reinterpret_cast<float*>(ctx.esp + 0x0c) - xShift;
		*reinterpret_cast<float*>(ctx.esp + 0x10) = *reinterpret_cast<float*>(ctx.esp + 0x10) - yShift;
		*reinterpret_cast<float*>(ctx.esp + 0x14) = *reinterpret_cast<float*>(ctx.esp + 0x14) + yShift;
	}
}

/* Magic gauge (stretched only)
	Note: As a 3D element, the magic gauge is automatically centered and needs to be re-stretched in stretched mode. */
void MagicGaugeHookStretched(SafetyHookContext& ctx) {
	*reinterpret_cast<float*>(ctx.esp + 0x18) = ((*reinterpret_cast<float*>(ctx.esp + 0x18) - 320.0f) * ARScale + 320.0f);
	*reinterpret_cast<float*>(ctx.esp + 0x24) = ((*reinterpret_cast<float*>(ctx.esp + 0x24) - 320.0f) * ARScale + 320.0f);
}

// This hook generally un-stretches the on-screen text (centered only)
void UnStretchTextHook(SafetyHookContext& ctx) {
	*reinterpret_cast<float*>(ctx.esp + 0x08) = (*reinterpret_cast<float*>(ctx.esp + 0x08) - 320.0f) / ARScale + 320.0f;
	*reinterpret_cast<float*>(ctx.esp + 0x0c) = (*reinterpret_cast<float*>(ctx.esp + 0x0c) - 320.0f) / ARScale + 320.0f;
}

// This hook fixes the size of the map
void MapSizeHook (SafetyHookContext& ctx) {
	*reinterpret_cast<int*>(ctx.esp + 0x08) = static_cast<int>(round(static_cast<float>(*reinterpret_cast<int*>(ctx.esp + 0x08)) * xScale));
	*reinterpret_cast<int*>(ctx.esp + 0x10) = static_cast<int>(round(static_cast<float>(*reinterpret_cast<int*>(ctx.esp + 0x10)) * xScale));
	*reinterpret_cast<int*>(ctx.esp + 0x18) = static_cast<int>(round(static_cast<float>(*reinterpret_cast<int*>(ctx.esp + 0x18)) * xScale));
	*reinterpret_cast<int*>(ctx.esp + 0x1c) = static_cast<int>(round(static_cast<float>(*reinterpret_cast<int*>(ctx.esp + 0x1c)) * yScale));
}

// Detect game version and set addresses
void DetectGame(void)
{
	HMODULE exeModule = GetModuleHandle(NULL);

	unsigned char* pBase = (unsigned char*)exeModule;
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)pBase;
	IMAGE_NT_HEADERS* pNtHeader = (IMAGE_NT_HEADERS*)(pBase + pDosHeader->e_lfanew);

	time_t timestamp = pNtHeader->FileHeader.TimeDateStamp;

	if (!pNtHeader->OptionalHeader.NumberOfRvaAndSizes) {
		return;
	}

	IMAGE_DATA_DIRECTORY* pExpDir = &pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	IMAGE_EXPORT_DIRECTORY* pExports = (IMAGE_EXPORT_DIRECTORY*)(pBase + pExpDir->VirtualAddress);
	if (!pExpDir->Size || !pExports->Name) 
		return;

	char const* pName = (char*)(pBase + pExports->Name);

	// Official patch exe dated 2000-01-26 16:26:44 UTC
	if (!strcmp(pName, "MaiD3Dvr_bleu.exe") && (timestamp == 948904004)) {

		GameVersion = 1;

		addressGetCurrentDirectoryA = 0x5980c4;
		addressCDCheck1 = reinterpret_cast<uint8_t*>(0x407b26);
		addressCDCheck2 = reinterpret_cast<uint8_t*>(0x407bfc);
		addressCDCheck3 = reinterpret_cast<uint8_t*>(0x407d69);
		addressCDCheck4 = reinterpret_cast<uint8_t*>(0x407e3f);
		addressCDCheck5 = reinterpret_cast<uint8_t*>(0x40cb10);
		addressCDCheck6 = reinterpret_cast<uint8_t*>(0x40f1c7);
		addressCDCheck7 = reinterpret_cast<uint8_t*>(0x410845);
		addressCDCheck8 = reinterpret_cast<uint8_t*>(0x41091b);
		addressAccessUbiIni1 = reinterpret_cast<uint8_t*>(0x408411);
		addressAccessUbiIni2 = reinterpret_cast<uint8_t*>(0x492cd4);
		addressStringUbiIni = reinterpret_cast<uint8_t*>(0x5b97b0);
		addressFPSValue = reinterpret_cast<uint8_t*>(0x5cc5d8);
		addressFPSCapFloor = reinterpret_cast<uint8_t*>(0x435b00);
		addressResolution = reinterpret_cast<uint8_t*>(0x4085e5);
		addressAdjustCameraToViewport = reinterpret_cast<uint8_t*>(0x46fc39);
		addressCallBlitStretched16b = reinterpret_cast<uint8_t*>(0x4f866a);
		addressIntLoadBarShift = reinterpret_cast<uint32_t*>(0x858f0c);
		addressSubtractLoadBarShift1 = reinterpret_cast<uint8_t*>(0x4f90f5);
		addressSubtractLoadBarShift2 = reinterpret_cast<uint8_t*>(0x4f915e);
		addressDraw2DSpriteWithUV = reinterpret_cast<uint8_t*>(0x46ffe0);
		addressTransformCoordinates = reinterpret_cast<uint8_t*>(0x42eee0);
		addressDrawNumberOfEnergyPrisms = reinterpret_cast<uint8_t*>(0x44be40);
		addressDrawTextOnInventoryScreen = reinterpret_cast<uint8_t*>(0x414fc0);
		addressCalibrate2D = reinterpret_cast<uint8_t*>(0x552ab0);
		addressDrawMagicGauge = reinterpret_cast<uint8_t*>(0x444920);
		addressDrawText = reinterpret_cast<uint8_t*>(0x470340);
		addressDrawMap = reinterpret_cast<uint8_t*>(0x4126a0);
		addressDrawMap2 = reinterpret_cast<uint8_t*>(0x4fa260);
	}
	// Spanish exe dated 1999-11-25 19:27:10 UTC
	else if (!strcmp(pName, "MaiD3Dvr_bleu.exe") && (timestamp == 943558030)) {

		GameVersion = 2;

		addressGetCurrentDirectoryA = 0x5990c8;
		addressCDCheck1 = reinterpret_cast<uint8_t*>(0x407ba6);
		addressCDCheck2 = reinterpret_cast<uint8_t*>(0x407c7c);
		addressCDCheck3 = reinterpret_cast<uint8_t*>(0x407de9);
		addressCDCheck4 = reinterpret_cast<uint8_t*>(0x407ebf);
		addressCDCheck5 = reinterpret_cast<uint8_t*>(0x40cc40);
		addressCDCheck6 = reinterpret_cast<uint8_t*>(0x40f2f7);
		addressCDCheck7 = reinterpret_cast<uint8_t*>(0x410975);
		addressCDCheck8 = reinterpret_cast<uint8_t*>(0x410a4b);
		addressAccessUbiIni1 = reinterpret_cast<uint8_t*>(0x408491);
		addressAccessUbiIni2 = reinterpret_cast<uint8_t*>(0x492e04);
		addressStringUbiIni = reinterpret_cast<uint8_t*>(0x5ba758);
		addressFPSValue = reinterpret_cast<uint8_t*>(0x5cd500);
		addressFPSCapFloor = reinterpret_cast<uint8_t*>(0x435d70);
		addressResolution = reinterpret_cast<uint8_t*>(0x408665);
		addressAdjustCameraToViewport = reinterpret_cast<uint8_t*>(0x46fd59);
		addressCallBlitStretched16b = reinterpret_cast<uint8_t*>(0x4f8a2a);
		addressIntLoadBarShift = reinterpret_cast<uint32_t*>(0x85860c);
		addressSubtractLoadBarShift1 = reinterpret_cast<uint8_t*>(0x4f94b5);
		addressSubtractLoadBarShift2 = reinterpret_cast<uint8_t*>(0x4f951e);
		addressDraw2DSpriteWithUV = reinterpret_cast<uint8_t*>(0x470100);
		addressTransformCoordinates = reinterpret_cast<uint8_t*>(0x42f140);
		addressDrawNumberOfEnergyPrisms = reinterpret_cast<uint8_t*>(0x44c0a0);
		addressDrawTextOnInventoryScreen = reinterpret_cast<uint8_t*>(0x415100);
		addressCalibrate2D = reinterpret_cast<uint8_t*>(0x552e70);
		addressDrawMagicGauge = reinterpret_cast<uint8_t*>(0x444b70);
		addressDrawText = reinterpret_cast<uint8_t*>(0x470460);
		addressDrawMap = reinterpret_cast<uint8_t*>(0x4127d0);
		addressDrawMap2 = reinterpret_cast<uint8_t*>(0x4fa620);
	}
}

// Config
void Configuration(void)
{
	// Inipp initialization
	inipp::Ini<char> ini;
	std::ifstream iniFile("Hype.ini");
	if (!iniFile)
		return;

	ini.parse(iniFile);
	ini.strip_trailing_comments();

	// Get values
	inipp::get_value(ini.sections["Resolution"], "xRes", xRes);
	inipp::get_value(ini.sections["Resolution"], "yRes", yRes);
	inipp::get_value(ini.sections["HUD"], "CenterHUDAndBackground", centerHUD);
	inipp::get_value(ini.sections["FPS"], "TargetFPS", FPSValue);
	inipp::get_value(ini.sections["FPS"], "PatchDeltaTiming", PatchFPSCap);
	inipp::get_value(ini.sections["Other"], "RemoveCDCheck", doRemoveCDCheck);
	inipp::get_value(ini.sections["Other"], "MakePortable", doMakePortable);

	// Compute auxiliary values
	xScale = static_cast<float>(xRes) / 640.0f;
	yScale = static_cast<float>(yRes) / 480.0f;
	ARScale = xScale / yScale;
	// The last "*2" is necessary because each pixel occupies two bytes in the buffer
	OffsetBackground = static_cast<int>(round(0.5f * xRes * (1.0f - 1.0f / ARScale))) * 2;
	// The load bar is shifted 60/640 = 3/32 from the left edge of the screen.
	OffsetLoadBar = static_cast<int>(round((0.5f - 3.0f / 32.0f) * xRes * (1.0f - 1.0f / ARScale))) * 2;

	skipFix = false;
}

// Initialize
void Init(void)
{
	DetectGame();
	if (!GameVersion) {
		MessageBox(0, TEXT("The EXE file is not compatible with this fix."), TEXT("Hype Fix"), MB_ICONWARNING);
		return;
	}
	else {
		Configuration();
		if (skipFix) {
			MessageBox(0, TEXT("Hype.ini not found."), TEXT("Hype Fix"), MB_ICONWARNING);
		}
	}
}

// Fixes
void RemoveCDCheck(void)
{
	PatchBytes(addressCDCheck1 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck1 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck2 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck2 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck3 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck3 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck4 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck4 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck5 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck5 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck6 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck6 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck7 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck7 + 47, "\x90\x90", 2);
	PatchBytes(addressCDCheck8 + 30, "\x90\x90", 2);
	PatchBytes(addressCDCheck8 + 47, "\x90\x90", 2);
}

void FixFPS(void)
{
	Write(addressFPSValue, FPSValue);
	if (PatchFPSCap)
	{
		PatchBytes(addressFPSCapFloor + 7, "\x01", 1);
		PatchBytes(addressFPSCapFloor + 11, "\x01", 1);
	}
}

void MakePortable(void)
{
	PatchBytes(addressAccessUbiIni1, "\x50\x68\x04\x01\x00\x00", 6);
	Write(addressAccessUbiIni1 + 8, addressGetCurrentDirectoryA);
	PatchBytes(addressAccessUbiIni2, "\x50\x68\x04\x01\x00\x00", 6);
	Write(addressAccessUbiIni2 + 8, addressGetCurrentDirectoryA);
	PatchBytes(addressStringUbiIni, "/Ubi.ini\x00\x00\x00\x00\x00\x00\x00\x00", 16);
}

void ChangeResolution(void)
{
	Write(addressResolution + 1, yRes);
	Write(addressResolution + 6, xRes);
}

void ChangeFOV(void)
{
	PatchBytes(addressAdjustCameraToViewport, "\x90\x90\x90", 3); // NOP out original FLD instruction
	hook01 = safetyhook::create_mid(addressAdjustCameraToViewport + 3, FOVMidHook); // Create hook
}

void FixHUD(void)
{
	// Fix load bar position. LoadBarShift gets subtracted from both xMin and xMax -> Need to hook two instructions.
	PatchBytes(addressSubtractLoadBarShift1, "\x90\x90\x90\x90\x90\x90", 6); // NOP out original subtraction
	hook02 = safetyhook::create_mid(addressSubtractLoadBarShift1, SubtractLoadBarShiftHook);
	PatchBytes(addressSubtractLoadBarShift2, "\x90\x90\x90\x90\x90\x90", 6); // NOP out original subtraction
	hook03 = safetyhook::create_mid(addressSubtractLoadBarShift2, SubtractLoadBarShiftHook);
	/* Patch coordinate transformation function
	   Note: This fixes the positioning of
		(a) the number of arrows (b) the amount of money when talking to a merchant (c) the magic gauge */ 
	PatchBytes(addressTransformCoordinates +3 , "\xb8\x80\x02\x00\x00", 5);
	// Fix position of number of energy prisms during jewel charge
	PatchBytes(addressDrawNumberOfEnergyPrisms + 3, "\xb8\x80\x02\x00\x00", 5);
	// Fix position of text on inventory screen
	PatchBytes(addressDrawTextOnInventoryScreen + 6, "\xb8\x80\x02\x00\x00", 5);
	// Fix position of text boxes (patch depends on game version)
	if (GameVersion == 1) {
		PatchBytes(addressCalibrate2D + 29, "\xba\x80\x02\x00\x00\x90", 6);
		PatchBytes(addressCalibrate2D + 35, "\xb9\xe0\x01\x00\x00\x90", 6);
	}
	else if (GameVersion == 2) {
		PatchBytes(addressCalibrate2D + 34, "\xba\x80\x02\x00\x00\x90", 6);
		PatchBytes(addressCalibrate2D + 47, "\xb9\xe0\x01\x00\x00\x90", 6);
		PatchBytes(addressCalibrate2D + 115, "\xb9\xe0\x01\x00\x00\x90", 6);
	}

	// Fix text on map screen
	PatchBytes(addressDrawMap + 0x672, "\xb8\x80\x02\x00\x00", 5);
	// Fix map size
	hook04 = safetyhook::create_mid(addressDrawMap2, MapSizeHook);

	if (centerHUD) {
		// Center/un-stretch background images and loading bar
		hook05 = safetyhook::create_mid(addressCallBlitStretched16b, VignetteHook);
		// Center/un-stretch several 2D sprite HUD elements
		hook06 = safetyhook::create_mid(addressDraw2DSpriteWithUV, Draw2DSpriteWithUVHookCentered);
		// Center/un-stretch text
		hook07 = safetyhook::create_mid(addressDrawText, UnStretchTextHook);
		
	} 
	else {
		// Fix rotated 2D sprites being vertically stretched
		hook05 = safetyhook::create_mid(addressDraw2DSpriteWithUV, Draw2DSpriteWithUVHookStretched);
		// Re-stretch magic gauge
		hook06 = safetyhook::create_mid(addressDrawMagicGauge + 0xc3, MagicGaugeHookStretched);
	}
}

void Main(void)
{
	Init();
	if (!skipFix) {
		if (doRemoveCDCheck) {
			RemoveCDCheck();
		}
		if (doMakePortable) {
			MakePortable();
		}
		FixFPS();
		ChangeResolution();
		ChangeFOV();
		FixHUD();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		Main();
		break;
	}

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}