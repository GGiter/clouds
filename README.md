# clouds
The main goal of the project is to try render realistic sky with atmospheric effects fully procedurally (using SDFs, raymarching etc). <br />
Project was tested only on Windows machines and is WIP. <br />
In images folder you can find screenshots from current version of the program.

# Installation

Install openvdb by using vpckg:
https://github.com/microsoft/vcpkg

After that in console type in:

*vcpkg install openvdb:x64-windows* <br />
*vcpkg install freeimage:x64-windows* <br />
*vcpkg install glad:x64-windows* <br />
*vcpkg install assimp:x64-windows* <br />
*vcpkg install jsoncons:x64-windows* <br />
*vcpkg integrate install* <br />

Change configuration in solution to *Release* to compile.
