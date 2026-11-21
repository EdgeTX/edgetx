set EDGETX_MAIN_DIR=C:\git\edgetx
set EDGETX_REPO=C:\git\edgetx
set EDGETX_BUILD_TOOLS=C:\git\edgetx\build-tools

set BUILD_RADIO=TX16s
set BUILD_OPTIONS=-DPCB=X10 -DPCBREV=TX16S

set BUILD_LANGUAGE=EN
set BUILD_DEBUG=no
set BUILD_FW=no
set BUILD_CPN=yes
set BUILD_INSTALLER=no

call "%EDGETX_MAIN_DIR%\edgetx_build.cmd"