@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

PUSHD "%~dp0"

clang-format --version
clang-format -i^
 ..\Source\Core\*.cpp^
 ..\Source\Core\*.hpp^
 ..\Source\Core\Containers\*.cpp^
 ..\Source\Core\Containers\*.hpp^
 ..\Source\Core\Math\*.hpp^
 ..\Source\Core\Memory\*.hpp^
 ..\Source\Engine\*.cpp^
 ..\Source\Engine\*.hpp^
 ..\Source\Engine\ECS\*.cpp^
 ..\Source\Engine\ECS\*.hpp^
 ..\Source\GUI\*.cpp^
 ..\Source\GUI\*.hpp^
 ..\Source\Main\*.cpp^
 ..\Source\Main\*.hpp^
 ..\Source\Platform\*.cpp^
 ..\Source\Platform\*.hpp^
 ..\Source\Platform\SDL2\*.cpp^
 ..\Source\Platform\SDL2\*.hpp^
 ..\Source\Platform\Windows\*.cpp^
 ..\Source\Platform\Windows\*.hpp^
 ..\Source\Render\*.cpp^
 ..\Source\Render\*.hpp^
 ..\Source\Render\DirectX\*.cpp^
 ..\Source\Render\DirectX\*.hpp^
 ..\Source\Render\Vulkan\*.cpp^
 ..\Source\Render\Vulkan\*.hpp^
 ..\Source\Tests\*.cpp^
 ..\Source\Tests\*.hpp^
 ..\Source\Tests\Core\*.cpp^
 ..\Source\Tests\Core\*.hpp^
 ..\Source\Tests\Engine\*.cpp^
 ..\Source\Tests\Engine\*.hpp^
 ..\Source\Tests\Platform\*.cpp^
 ..\Source\Tests\Platform\*.hpp

 ECHO Format complete.

POPD

ENDLOCAL