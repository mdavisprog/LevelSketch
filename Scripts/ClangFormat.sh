#!/bin/bash

pushd "$(dirname "${BASH_SOURCE[0]}")"

clang-format --version
clang-format -i \
../Source/Core/*.cpp \
../Source/Core/*.hpp \
../Source/Core/Containers/*.cpp \
../Source/Core/Containers/*.hpp \
../Source/Core/Math/*.hpp \
../Source/Core/Memory/*.hpp \
../Source/Engine/*.cpp \
../Source/Engine/*.hpp \
../Source/Main/*.cpp \
../Source/Main/*.hpp \
../Source/Platform/*.cpp \
../Source/Platform/*.hpp \
../Source/Platform/Mac/*.hpp \
../Source/Platform/Mac/*.mm \
../Source/Platform/SDL2/*.cpp \
../Source/Platform/SDL2/*.hpp \
../Source/Platform/Windows/*.cpp \
../Source/Platform/Windows/*.hpp \
../Source/Render/*.cpp \
../Source/Render/*.hpp \
../Source/Render/DirectX/*.cpp \
../Source/Render/DirectX/*.hpp \
../Source/Render/Metal/*.hpp \
../Source/Render/Metal/*.mm \
../Source/Render/Vulkan/*.cpp \
../Source/Render/Vulkan/*.hpp \
../Source/Tests/*.cpp \
../Source/Tests/*.hpp \
../Source/Tests/Core/*.cpp \
../Source/Tests/Core/*.hpp \
../Source/Tests/Engine/*.cpp \
../Source/Tests/Engine/*.hpp \
../Source/Tests/Platform/*.cpp \
../Source/Tests/Platform/*.hpp

echo "Format complete."

popd
