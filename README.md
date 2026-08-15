# Jill of the Jungle Reconstructed Source Code
By Justin Marshall

Source reconstruction of Jill of the Jungle Episodes 1 and 2, released under
the GPL. Both games are built from the shared sources in `src`; CMake selects
the original episode-specific data and behavior with `JILL_EP1` or `JILL_EP2`.

The recovered game builds as a 32-bit Windows executable. Its DOS WORX audio
boundary streams through statically linked OpenAL Soft. Jill's CMF music uses
the embedded AdLib patches and recovered WORX register behavior with the
vendored Nuked OPL core; libTiMidity and FreePats remain vendored for genuine
MIDI data. No OpenAL DLL or system MIDI synthesizer is required.

Build from the repository root with:

```powershell
cmake -S src -B build-win32 -A Win32
cmake --build build-win32 --config Release --target jill1 jill2 --parallel 2
```

The executables are written beside their respective data files:

- Episode 1: `E:\projects\jill1\ep1\jill1.exe`
- Episode 2: `E:\projects\jill1\ep2\jill2.exe`

The original 16-bit Episode 2 executable is preserved at
`E:\projects\jill1\ep2\original\JILL2.EXE`, because Windows treats that name
as the same path as the reconstructed `jill2.exe` output.

Run either executable normally to play. Use `--validate [level]` for a
noninteractive data/load check, or run the configured validation suite with:

```powershell
ctest --test-dir build-win32 -C Release --output-on-failure
```
