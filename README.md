# Jill of the Jungle Reconstructed Source Code
By Justin Marshall

Full source code recreation of Jill of the Jungle Episode 1 released under GPL

The recovered game builds as a 32-bit Windows executable. Its DOS WORX audio
boundary streams through statically linked OpenAL Soft. Jill's CMF music uses
the embedded AdLib patches and recovered WORX register behavior with the
vendored Nuked OPL core; libTiMidity and FreePats remain vendored for genuine
MIDI data. No OpenAL DLL or system MIDI synthesizer is required.

Build from the repository root with:

```powershell
cmake -S src -B build-win32 -A Win32
cmake --build build-win32 --config Release --target jill1 --parallel 2
```

The executable is written to `E:\projects\jill1\jill1.exe`. Run
`jill1.exe --validate` for the noninteractive data/load check, or run
`jill1.exe` to play.
