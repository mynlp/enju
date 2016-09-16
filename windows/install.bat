set INSTALL_DIR=enju-win

mkdir %INSTALL_DIR%\bin
copy /Y bin\enju.exe %INSTALL_DIR%\bin\enju.exe
copy /Y bin\enju-morph.exe %INSTALL_DIR%\bin\enju-morph.exe
copy /Y bin\stepp.exe %INSTALL_DIR%\bin\stepp.exe
copy /Y bin\mogura.exe %INSTALL_DIR%\bin\mogura.exe
copy /Y bin\bzip2.dll %INSTALL_DIR%\bin\bzip2.dll
copy /Y bin\zlib1.dll %INSTALL_DIR%\bin\zlib1.dll

mkdir %INSTALL_DIR%\lib\enju
if exist ..\DATA ( xcopy /Y /E /I ..\DATA %INSTALL_DIR%\lib\enju\DATA )

mkdir %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\init.lil      %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\Artistic      %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\COPYING       %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\lildoc.prl    %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\index.html    %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\index.ja.html %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\style.css   %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\lilfes.gif  %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\favicon.ico %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\fs.xsl      %INSTALL_DIR%\share\liblilfes
copy /Y ..\ll\fs.css      %INSTALL_DIR%\share\liblilfes
xcopy /Y /E /I ..\enju %INSTALL_DIR%\share\liblilfes\enju
xcopy /Y /E /I ..\mayz %INSTALL_DIR%\share\liblilfes\mayz
xcopy /Y /E /I ..\mogura %INSTALL_DIR%\share\liblilfes\mogura
xcopy /Y /E /I ..\ll\lillib %INSTALL_DIR%\share\liblilfes\lillib

mkdir %INSTALL_DIR%\share\stepp
xcopy /Y /E /I ..\stepp-tagger\models_brown-wsj02-21c %INSTALL_DIR%\share\stepp\models_brown-wsj02-21c 
xcopy /Y /E /I ..\stepp-tagger\models_medline         %INSTALL_DIR%\share\stepp\models_medline
xcopy /Y /E /I ..\stepp-tagger\models_wsj02-21c       %INSTALL_DIR%\share\stepp\models_wsj02-21c

copy /Y enju.bat %INSTALL_DIR%\enju.bat
copy /Y mogura.bat %INSTALL_DIR%\mogura.bat

copy /Y ..\COPYING %INSTALL_DIR%\COPYING
copy /Y ..\README %INSTALL_DIR%\README
copy /Y ..\README.ja %INSTALL_DIR%\README.ja

