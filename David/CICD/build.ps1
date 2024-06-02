# Build script for Windows
$UE4Path = "C:\Program Files\Epic Games\UE_5.0"
$ProjectPath = "${{ github.workspace }}\YourProject\YourProject.uproject"

& "$UE4Path\Engine\Build\BatchFiles\Build.bat" YourProjectEditor Win64 Development -project="$ProjectPath" -waitmutex -nocompileeditor -noclean