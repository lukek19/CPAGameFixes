###########
# Imports #
###########
import pefile
import re
import struct

####################
# Helper functions #
####################


def scan_bytes(bytes_object, compiled_regex, result_name, base_address=0):
    results = []

    # Find all matches
    for match in compiled_regex.finditer(bytes_object):
        relative_address = match.start()
        absolute_address = relative_address + base_address
        results.append(absolute_address)

    # Print results
    if not results:
        print(f"ERROR: No address found for {result_name}")
    else:
        for i, address in enumerate(results):
            suffix = f"{i + 1}" if len(results) > 1 else ""
            print(f"{result_name}{suffix}: 0x{address:x}")


######################
# Read and parse EXE #
######################

path_to_exe = "MaiD3Dvr_bleu.exe"

# Parse EXE
pe = pefile.PE(path_to_exe)

pe_base_address = pe.OPTIONAL_HEADER.ImageBase  # should be 0x400000

# Find .text and .data section
for i in range(len(pe.sections)):
    if pe.sections[i].Name[0:5] == b'.text':
        text_section = pe.sections[i]
    if pe.sections[i].Name[0:5] == b'.data':
        data_section = pe.sections[i]

text_section_data = text_section.get_data()
text_section_base_address = pe_base_address + text_section.VirtualAddress  # should be 0x401000
data_section_data = data_section.get_data()
data_section_base_address = pe_base_address + data_section.VirtualAddress

# Determine the addresses of the functions GetCurrentDirectoryA and GetWindowsDirectoryA
address_GetCurrentDirectoryA = 0
address_GetWindowsDirectoryA = 0
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    if entry.dll.decode("ascii").lower() == "kernel32.dll":
        for imp in entry.imports:
            if imp.name.decode("ascii").lower() == "getcurrentdirectorya":
                address_GetCurrentDirectoryA = imp.address
            if imp.name.decode("ascii").lower() == "getwindowsdirectorya":
                address_GetWindowsDirectoryA = imp.address
    break

if address_GetCurrentDirectoryA == 0:
    print("ERROR: address_GetCurrentDirectoryA not found.")
elif address_GetWindowsDirectoryA == 0:
    print("ERROR: address_GetWindowsDirectoryA not found.")
else:
    print(f"addressGetCurrentDirectoryA: 0x{address_GetCurrentDirectoryA:x}")

print(f"TimeDateStamp: {pe.FILE_HEADER.TimeDateStamp}")
print(f"ModuleName: {pe.DIRECTORY_ENTRY_EXPORT.name.decode()}")

################
# Address scan #
################

# Scan for CD Check addresses
regExp = re.compile(rb'\x74\x50((\x85\xed)|(\x3b\xeb)).{26}\x74\x18.{15}\x74\x07', re.DOTALL)
scan_bytes(text_section_data, regExp, "addressCDCheck", text_section_base_address)

# Scan for Code that accesses Ubi.ini
regExp = re.compile(rb'\x68\x04\x01\x00\x00\x50\xff\x15' + address_GetWindowsDirectoryA.to_bytes(4, byteorder="little"),
                    re.DOTALL)
scan_bytes(text_section_data, regExp, "addressAccessUbiIni", text_section_base_address)
# Scan for the path of Ubi.ini
regExp = re.compile(b'/UbiSoft/Ubi.ini', re.IGNORECASE)
scan_bytes(data_section_data, regExp, "addressStringUbiIni", data_section_base_address)

# Scan for the string "%d  fps" (preceded by the number 50)
regExp = re.compile(rb'\x32\x00\x00\x00\x25\x64\x20\x20\x66\x70\x73')
scan_bytes(data_section_data, regExp, "addressFPSValue", data_section_base_address)
# Scan for the delta timing function that determines the FPS cap
regExp = re.compile(rb"""\xa1.{4}                     # mov eax ds:[...]
                         \x83\xf8\x10                 # cmp eax,0x10
                         \x73\x06                     # jae 0x8
                         \xb8\x10\x00\x00\x00         # mov eax,0x10
                         \xc3                         # ret
                         \x83\xf8\x50                 # cmp eax,0x50
                         \x76\x05                     # jbe 0x7
                         \xb8\x50\x00\x00\x00         # mov eax,0x50
                         \xc3                         # ret""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressFPSCapFloor", text_section_base_address)

# Scan for resolution bytes
regExp = re.compile(rb"""\xff\xd5                # call ebp
                         \x8d\x44\x24\x10        # lea aax,[esp+0x10]
                         \x50                    # push eax
                         \xe8.{4}                # call GLI_vSetTextureMode
                         \x83\xc4\x04            # add esp,0x4
                         \x68\xe0\x01\x00\x00    # push 0x168 (=360)
                         \x68\x80\x02\x00\x00    # push 0x280 (=640)""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressResolution", text_section_base_address + 15)

# Scan for the relevant part of the function AdjustCameraToViewport
regExp = re.compile(rb"""\xd9\x40\x64       # fld    DWORD PTR [eax+0x64]
                         \xd8\x0d.{4}       # fmul   DWORD PTR ds:...
                         \xd9\x44\x24\x1c   # fld    DWORD PTR [esp+0x1c]
                         \xd8\x0d.{4}       # fmul   DWORD PTR ds:...
                         \xd9\xe8           # fld1
                         \xd9\xca           # fxch   st(2)
                         \xd9\x54\x24\x0c   # fst    DWORD PTR [esp+0xc]
                         \xd9\xf2           # fptan
                         \xd9\xc9           # fxch   st(1) """, re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressAdjustCameraToViewport", text_section_base_address)

# Scan for a certain call of the function BlitStretched16b (needed to un-stretch backgrounds)
regExp = re.compile(rb"""\xe8.{4}                                    # call   GLD_fn_vBlitStretched16b
                         \x83\xc4\x1c                                # add    esp,0x1c
                         \x8d\xb5\x18\x01\x00\x00                    # lea    esi,[ebp+0x118]
                         \x6a\x07                                    # push   0x7
                         \x56                                        # push   esi
                         \xc7\x85\x14\x01\x00\x00\x02\x00\x00\x00    # mov    DWORD PTR [ebp+0x114],0x2""",
                    re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressCallBlitStretched16b", text_section_base_address)

# Scan for the function that reads <LevelName>pgb.bin and performs some computations to draw the load bar
regExp = re.compile(rb"""\x2b\x15(.{4})         # sub    edx,DWORD PTR [LoadBarShift]
                         \x89\x15.{4}           # mov    DWORD PTR ds:...,edx
                         \x56                   # push   esi
                         \x6a\x01               # push   0x1
                         \x8d\x54\x24\x18       # lea    edx,[esp+0x18]
                         \x6a\x04               # push   0x4
                         \x52                   # push   edx
                         \xff\xd7               # call   edi
                         \x8b\x4c\x24\x20       # mov    ecx,DWORD PTR [esp+0x20]
                         \xb8\x89\x88\x88\x88   # mov    eax,0x88888889
                         .{72}                  # ...
                         \x2b\x15\1             # sub    edx,DWORD PTR [LoadBarShift]""", re.VERBOSE | re.DOTALL)
result = regExp.search(text_section_data)
if result:
    print("addressIntLoadBarShift:", hex(struct.unpack("<L", text_section_data[result.start()+2: result.start()+6])[0]))
    print(f"addressSubtractLoadBarShift1: 0x{text_section_base_address + result.start():x}")
    print(f"addressSubtractLoadBarShift2: 0x{text_section_base_address + result.start() + 105:x}")
else:
    print("ERROR: addressIntLoadBarShift, addressSubtractLoadBarShift1 and addressSubtractLoadBarShift2 not found")

# Scan for function Draw2DSpriteWithUV
regExp = re.compile(rb"""\x55                           # push   ebp
                         \x8b\xec                       # mov    ebp,esp
                         \x81\xec\x98\x00\x00\x00       # sub    esp,0x98
                         \x53                           # push   ebx
                         \x56                           # push   esi
                         \x57                           # push   edi
                         \x8b\x7d\x08                   # mov    edi,DWORD PTR [ebp+0x8]
                         \x81\x7f\x0c\x80\x02\x00\x00   # cmp    DWORD PTR [edi+0xc],0x280
                         \x75\x0d                       # jne    0x25
                         \x81\x7f\x08\xe0\x01\x00\x00   # cmp    DWORD PTR [edi+0x8],0x1e0
                         \x0f\x84\x99\x00\x00\x00       # je     0xbe""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDraw2DSpriteWithUV", text_section_base_address)

# Scan for function that transforms coordinates
regExp = re.compile(rb"""\x83\xec\x08                     # sub    esp,0x8
                         \xa1.{4}                         # mov    eax,ds:...
                         \xc7\x44\x24\x04\x00\x00\x00\x00 # mov    DWORD PTR [esp+0x4],0x0
                         \x89\x44\x24\x00                 # mov    DWORD PTR [esp+0x0],eax
                         \x8b\x44\x24\x0c                 # mov    eax,DWORD PTR [esp+0xc]
                         \xdf\x6c\x24\x00                 # fild   QWORD PTR [esp+0x0]
                         \xc7\x40\x04\x7b\x14\x2e\xbe     # mov    DWORD PTR [eax+0x4],0xbe2e147b""",
                    re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressTransformCoordinates", text_section_base_address)

# Scan for function that draws the number of energy prisms during jewel charge
regExp = re.compile(rb"""\x83\xec\x08                     # sub    esp,0x8
                         \xa1.{4}                         # mov    eax,ds:...
                         \xc7\x44\x24\x04\x00\x00\x00\x00 # mov    DWORD PTR [esp+0x4],0x0
                         \x89\x44\x24\x00                 # mov    DWORD PTR [esp+0x0],eax
                         \x56                             # push   esi
                         \xdf\x6c\x24\x04                 # fild   QWORD PTR [esp+0x4]
                         \x57                             # push   edi""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawNumberOfEnergyPrisms", text_section_base_address)

# Scan for function that draws text on inventory screen
regExp = re.compile(rb"""\x81\xec\x10\x01\x00\x00         # sub    esp,0x110
                         \xa1.{4}                         # mov    eax,ds:...
                         \xc7\x44\x24\x0c\x00\x00\x00\x00 # mov    DWORD PTR [esp+0xc],0x0
                         \x89\x44\x24\x08                 # mov    DWORD PTR [esp+0x8],eax
                         \x56                             # push   esi
                         \xdf\x6c\x24\x0c                 # fild   QWORD PTR [esp+0xc]
                         \xdb\x84\x24\x18\x01\x00\x00     # fild   DWORD PTR [esp+0x118]
                         \xdb\x84\x24\x1c\x01\x00\x00     # fild   DWORD PTR [esp+0x11c]""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawTextOnInventoryScreen", text_section_base_address)

# Scan for function "Calibrate2D" that is responsible for text box placement
regExp = re.compile(rb"""\x83\xec\x10                   # sub    esp,0x10
                         \x8d\x44\x24\x04               # lea    eax,[esp+0x4]
                         \x8d\x4c\x24\x00               # lea    ecx,[esp+0x0]
                         \x50                           # push   eax
                         \x51                           # push   ecx
                         \x6a\x01                       # push   0x1
                         \xe8.{4}                       # call   ...
                         \x83\xc4\x04                   # add    esp,0x4
                         \x50                           # push   eax""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressCalibrate2D", text_section_base_address)

# Scan for function that draws the magic gauge
regExp = re.compile(rb"""\xa1.{4}                       # mov    eax,ds:...
                         \x83\xec\x24                   # sub    esp,0x24
                         \x66\x8b\x48\x26               # mov    cx,WORD PTR [eax+0x26]
                         \x53                           # push   ebx
                         \x66\x89\x0d.{4}               # mov    WORD PTR ds:...,cx
                         \x66\x8b\x50\x24               # mov    dx,WORD PTR [eax+0x24]
                         \x66\x89\x15.{4}               # mov    WORD PTR ds:...,dx
                         \x8a\x50\x28                   # mov    dl,BYTE PTR [eax+0x28]""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawMagicGauge", text_section_base_address)

# Scan for function that draws text on screen
regExp = re.compile(rb"""\x55                           # push   ebp
                         \x8b\xec                       # mov    ebp,esp
                         \x81\xec\x88\x00\x00\x00       # sub    esp,0x88
                         \x53                           # push   ebx
                         \x56                           # push   esi
                         \x8b\x75\x08                   # mov    esi,DWORD PTR [ebp+0x8]
                         \x57                           # push   edi
                         \x81\x7e\x0c\x80\x02\x00\x00   # cmp    DWORD PTR [esi+0xc],0x280""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawText", text_section_base_address)

# Scan for a function that draws the map (needed to fix text on map screen)
regExp = re.compile(rb"""\x83\xec\x28                   # sub    esp,0x28
                         \x8b\x0d(.{4})                 # mov    ecx,DWORD PTR ds:(1)
                         \x33\xc0                       # xor    eax,eax
                         \x89\x44\x24\x00               # mov    DWORD PTR [esp+0x0],eax
                         \x53                           # push   ebx
                         \x89\x41\x10                   # mov    DWORD PTR [ecx+0x10],eax
                         \x8b\x15\1                     # mov    edx,DWORD PTR ds:(1)
                         \x55                           # push   ebp""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawMap", text_section_base_address)

# Scan for another function that draws the map (needed to fix text on map size)
regExp = re.compile(rb"""\x66\x8b\x0d.{4}               # mov    cx,WORD PTR ds:...
                         \x66\x8b\x15.{4}               # mov    dx,WORD PTR ds:...
                         \x83\xec\x64                   # sub    esp,0x64
                         \x8d\x44\x24\x00               # lea    eax,[esp+0x0]
                         \x53                           # push   ebx
                         \x55                           # push   ebp
                         \x50                           # push   eax
                         \x51                           # push   ecx
                         \x52                           # push   edx""", re.VERBOSE | re.DOTALL)
scan_bytes(text_section_data, regExp, "addressDrawMap2", text_section_base_address)