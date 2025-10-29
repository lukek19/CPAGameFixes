#include <cmath>
#include <cstdint>
#include <fstream>
#include <inipp.h>
#include <safetyhook.hpp>
#include <windows.h>


// Variables declared in Donald.ini
bool doCompatibilityPatch = true;
bool doDisableSync = false;
bool doNoCD = true;
bool doMakePortable = true;

// Variables computed during configuration
unsigned char GameVersion = 0; // 0 unknown
bool skipFix = true;
float AR = 4.0f/3.0f; // 4:3 fallback AR if parsing fails
float ARScale;

// Declare addresses (are set during configuration)
uint8_t* addressCompatibilityPatch;
uint8_t* addressFlipDeviceWithSyncro;
uint8_t* addressAdjustCameraToViewport;
uint8_t* addressCDCheck;
uint8_t* addressFormatStrings1;
uint8_t* addressFormatStrings2;
uint8_t* addressFormatStrings3;
uint8_t* addressFormatStrings4;
uint8_t* addressFormatStrings5;
uint8_t* addressAccessUbiIni1;
uint8_t* addressAccessUbiIni2;
uint8_t* addressAccessUbiIni3;
uint8_t* addressStringUbiIni1;
uint8_t* addressStringUbiIni2;
uint8_t* addressDraw2DSpriteWithPercent;
uint8_t* addressGet3DVertexFromScreenPos;
uint8_t* addressFloatWidthA;
uint8_t* addressFloatWidthB;

// Declare hooks
SafetyHookMid hook01{};
SafetyHookMid hook02{};
SafetyHookMid hook03{};

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

// Draw2DSpriteWithPercent
void Draw2DSpriteWithPercentHook(SafetyHookContext& ctx) {
	*reinterpret_cast<float*>(ctx.esp + 0x8) = (*reinterpret_cast<float*>(ctx.esp + 0x8) - 50.0f) / ARScale + 50.0f; // xMin
	*reinterpret_cast<float*>(ctx.esp + 0x10) = (*reinterpret_cast<float*>(ctx.esp + 0x10) - 50.0f) / ARScale + 50.0f; // xMax
}

// Get3DVertexFromScreenPos
void Get3DVertexFromScreenPosHook(SafetyHookContext& ctx) {
	*reinterpret_cast<float*>(ctx.esp + 0xc) = (*reinterpret_cast<float*>(ctx.esp + 0xc) - 0.5f) / ARScale + 0.5f;

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

	// Version dated 16/10/2000 13:58:07 UTC
	if (!strcmp(pName, "MainWinf.exe") && (timestamp == 971704687)) {

		GameVersion = 1;

		addressCompatibilityPatch = reinterpret_cast<uint8_t*>(0x448e05);
		addressFlipDeviceWithSyncro = reinterpret_cast<uint8_t*>(0x4a21d7);
		addressAdjustCameraToViewport = reinterpret_cast<uint8_t*>(0x423855);
		addressCDCheck = reinterpret_cast<uint8_t*>(0x449ed0);
		addressFormatStrings1 = reinterpret_cast<uint8_t*>(0x4a141b);
		addressFormatStrings2 = reinterpret_cast<uint8_t*>(0x4a2910);
		addressFormatStrings3 = reinterpret_cast<uint8_t*>(0x4a44c4);
		addressFormatStrings4 = reinterpret_cast<uint8_t*>(0x4a4930);
		addressFormatStrings5 = reinterpret_cast<uint8_t*>(0x4a8e88);
		addressAccessUbiIni1 = reinterpret_cast<uint8_t*>(0x405c6d);
		addressAccessUbiIni2 = reinterpret_cast<uint8_t*>(0x405ced);
		addressAccessUbiIni3 = reinterpret_cast<uint8_t*>(0x405d70);
		addressStringUbiIni1 = reinterpret_cast<uint8_t*>(0x4a1160);
		addressStringUbiIni2 = reinterpret_cast<uint8_t*>(0x4a1490);
		addressDraw2DSpriteWithPercent = reinterpret_cast<uint8_t*>(0x42b060);
		addressGet3DVertexFromScreenPos = reinterpret_cast<uint8_t*>(0x423490);
		addressFloatWidthA = reinterpret_cast<uint8_t*>(0x49fa20);
		addressFloatWidthB = reinterpret_cast<uint8_t*>(0x49fa6c);
		
	}
}

// Config
void Configuration(void)
{
	// Inipp initialization
	inipp::Ini<char> ini;
	std::ifstream iniFile("Donald.ini");
	if (!iniFile)
		return;

	ini.parse(iniFile);
	ini.strip_trailing_comments();

	std::string ratioStr;
	
		// Get values
	inipp::get_value(ini.sections["Aspect Ratio"], "AR", ratioStr);
	inipp::get_value(ini.sections["Other"], "CompatibilityPatch", doCompatibilityPatch);
	inipp::get_value(ini.sections["Other"], "DisableInternalSync", doDisableSync);
	inipp::get_value(ini.sections["Other"], "NoCD", doNoCD);
	inipp::get_value(ini.sections["Other"], "MakePortable", doMakePortable);

	// Parse aspect ratio. Expected format: "W:H" (e.g. "16:9")
	std::istringstream iss(ratioStr);
	float width = 0.0f, height = 0.0f;
	char sep = 0;
	if (iss >> width >> sep >> height && sep == ':' && height != 0.0f) {
		AR = width / height;
	}

	// Compute auxiliary values
	ARScale = AR / (4.0f / 3.0f);

	skipFix = false;
}

// Initialize
void Init(void)
{
	DetectGame();
	if (!GameVersion) {
		MessageBox(0, TEXT("The EXE file is not compatible with this fix."), TEXT("Donald Fix"), MB_ICONWARNING);
		return;
	}
	else {
		Configuration();
		if (skipFix) {
			MessageBox(0, TEXT("Donald.ini not found."), TEXT("Donald Fix"), MB_ICONWARNING);
		}
	}
}

// Fixes

void CompatibilityPatch(void)
{
	PatchBytes(addressCompatibilityPatch, "\x6e\x5f\x5c\x00\xff\x15\xc0\x54\x5c\x00\x83\xc4\x04", 13);
}

void DisableSync(void)
{
	PatchBytes(addressFlipDeviceWithSyncro, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);
}

void NoCD(void)
{
	PatchBytes(addressCDCheck, "\xc3", 1);
	PatchBytes(addressFormatStrings1, "\x3f\x25\x63\x5c\x25\x73\x2f\x25\x73\x2f\x25\x73\x2e\x25\x73\x00\x00\x25\x63\x5c\x25\x73\x2f\x2a\x00", 25); 	// "?%c:\\%s/%s/%s.%s" -> "?%c\\%s/%s/%s.%s", "%c:\\%s/*" -> "%c\\%s/*"
	PatchBytes(addressFormatStrings2, "\x25\x63\x5c\x25\x73\x00", 6); // "%c:\\%s" -> "%c\\%s"
	PatchBytes(addressFormatStrings3, "\x25\x63\x5c\x25\x73\x5c\x25\x73\x00", 9); // "%c:\\%s\\%s" -> "%c\\%s\\%s"
	PatchBytes(addressFormatStrings4, "\x25\x63\x5c\x00", 4); // "%c:\\" -> "%c\\"
	PatchBytes(addressFormatStrings5, "\x2e", 1); // "" -> "."
}

void MakePortable(void)
{
	// Instead of calling GetWindowsDirectoryA(), write "." to the address in EAX
	PatchBytes(addressAccessUbiIni1, "\x66\xc7\x00\x2e\x00\x90\x90\x90\x90\x90\x90\x90", 12); // MOV word ptr [EAX],0x2e / NOP (x7)
	PatchBytes(addressAccessUbiIni2, "\x66\xc7\x00\x2e\x00\x90\x90\x90\x90\x90\x90\x90", 12); // MOV word ptr [EAX],0x2e / NOP (x7)
	PatchBytes(addressAccessUbiIni3, "\x66\xc7\x00\x2e\x00\x90\x90\x90\x90\x90\x90\x90", 12); // MOV word ptr [EAX],0x2e / NOP (x7)
	PatchBytes(addressStringUbiIni1, "\x5c\x55\x62\x69\x2e\x69\x6e\x69\x00\x00\x00\x00\x00\x00\x00\x00", 16); // "\\UbiSoft\\Ubi.ini" -> "\\Ubi.ini"
	PatchBytes(addressStringUbiIni2, "\x2f\x55\x62\x69\x2e\x69\x6e\x69\x00\x00\x00\x00\x00\x00\x00\x00", 16); // "/UbiSoft/Ubi.ini" -> "/Ubi.ini"
}

void ChangeFOV(void)
{
	PatchBytes(addressAdjustCameraToViewport, "\x90\x90\x90", 3); // NOP out original FLD instruction
	hook01 = safetyhook::create_mid(addressAdjustCameraToViewport, FOVMidHook); // Create hook
}

void FixHUD(void)
{
	hook02 = safetyhook::create_mid(addressDraw2DSpriteWithPercent, Draw2DSpriteWithPercentHook);
	hook03 = safetyhook::create_mid(addressGet3DVertexFromScreenPos, Get3DVertexFromScreenPosHook);
	Write(addressFloatWidthA, 1.0f / (640.0f * ARScale));
	// The following two patches are needed to fix the text in Rayman 2. I don't think they are needed in this game, but it shouldn't harm to apply them anyway.
	Write(addressFloatWidthB, 1.0f / (640.0f * ARScale));
	Write(addressFloatWidthB + 4, 1.0f / (480.0f * ARScale));
}

void Main(void)
{
	Init();
	if (!skipFix) {
		if (doCompatibilityPatch) {
			CompatibilityPatch();
		}
		if (doDisableSync) {
			DisableSync();
		}
		if (doNoCD) {
			NoCD();
		}
		if (doMakePortable) {
			MakePortable();
		}
		if (ARScale != 1.0f) { // Fix only needed when AR not 4:3
			ChangeFOV();
			FixHUD();
		}
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