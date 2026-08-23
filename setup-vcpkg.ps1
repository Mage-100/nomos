echo "Clonning vcpkg.git"

git clone https://www.github.com/microsoft/vcpkg.git

Set-Location .\vcpkg
# The data collected by Microsoft is anonymous.
# You can opt-out of telemetry by re-running the bootstrap-vcpkg script with -disableMetrics,
.\bootstrap-vcpkg.bat -disableMetrics
Set-Location ..\
