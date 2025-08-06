# PCOMM Scroll wheel and double click support mod

Adds support for using the scroll wheel as PF8/PF7 to IBM Personal Communications. Optionally also adds support for using double click as ENTER. This is already in PCOMM 12 through the Hotspots configuration, but it's implemented badly and slows down normal clicks considerably. This mod doesn't.

This project was developed for PCOMM Version 12 (32-bit). It will likely not work on other versions, but shouldn't be too difficult to adapt.

## Installation

Build each folder into a separate 32-bit DLL (named `hook.dll` and `psapi.dll`). Copy both DLLs to `C:\Program Files (x86)\IBM\Personal Communications`. Copy `C:\Windows\SysWOW64\psapi.dll` there too, but rename it to `psapiorig.dll`.
