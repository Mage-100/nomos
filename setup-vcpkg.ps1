echo "Clonning vcpkg.git"

git clone https://www.github.com/microsoft/vcpkg.git

Set-Location .\vcpkg
.\bootstrap-vcpkg.bat
Set-Location ..\
