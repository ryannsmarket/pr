


rem "compare" - image magick compare program

set SRC=mmrest-1,bravura-mmrest,gonville-mmrest,mmrest-2,mmrest-4,mmrest-5,mmrest-6,mmrest-7,mmrest-8,mmrest-9, ^
 mmrest-10,fmrest-1,fmrest-2,fmrest-3,fmrest-4,fmrest-5,fmrest-6,measure-repeat-1, ^
 noteheadposition-1,valign-1,valign-2,valign-3,emmentaler-1,bravura-1,gonville-1,emmentaler-2,bravura-2,gonville-2, ^
 emmentaler-3,bravura-3,gonville-3,emmentaler-4,bravura-4,gonville-4,emmentaler-5,bravura-5,gonville-5, ^
 emmentaler-6,bravura-6,gonville-6,emmentaler-7,bravura-7,gonville-7, ^
 emmentaler-8,bravura-8,gonville-8,emmentaler-9,bravura-9,gonville-9, ^
 emmentaler-10,bravura-10,gonville-10,emmentaler-11,bravura-11,gonville-11, ^
 emmentaler-text-1,gonville-text-1,bravura-text-1,musejazz-text-1, ^
 emmentaler-text-2,gonville-text-2,bravura-text-2,musejazz-text-2, ^
 emmentaler-text-3,gonville-text-3,bravura-text-3,musejazz-text-3, ^
 frametext,ottava,slurs-1,slurs-2,slurs-3,hairpins-1,pedal-1,line-1,line-2,line-3,line-4,gliss-1,gliss-2, ^
 chord-layout-1,chord-layout-2,chord-layout-3,chord-layout-4,chord-layout-5, ^
 chord-layout-6,chord-layout-7,chord-layout-8,chord-layout-9,chord-layout-10, ^
 chord-layout-11,chord-layout-12,chord-layout-13,chord-layout-14,chord-layout-15,cross-1, ^
 accidental-1,accidental-2,accidental-3,accidental-4,accidental-5, ^
 accidental-6,accidental-7,accidental-8,accidental-9, ^
 tie-1,tie-2,tie-3,grace-1,grace-2,grace-3,grace-4,tuplets-1,breath-1 ^
 harmony-1,harmony-2,harmony-3,harmony-4,harmony-5,harmony-6,harmony-7,harmony-8,harmony-9,harmony-10,harmony-11, ^
 beams-1,beams-2,beams-3,beams-4,beams-5,beams-6,beams-7,beams-8,beams-9, ^
 user-offset-1,user-offset-2,chord-space-1,chord-space-2,tablature-1,image-1, ^
 lyrics-1,lyrics-2,lyrics-3,lyrics-4,voice-1,voice-2,slash-1,slash-2,system-1,system-2

set MSCORE=..\win32install\bin\musescore.exe
set DPI=130
set F=vtest.html

rd /s/q html
md html
cd html

FOR /D %%a IN (%SRC%) DO (
      echo process %%a
      xcopy ..\%%a-ref.png .
      ..\%MSCORE% ..\%%a.mscz -r %DPI% -o %%a.png
      compare -metric AE -fuzz 50%% %%a-1.png %%a-ref.png %%a-diff.png
)

xcopy ..\style.css .

del /q %F%

echo ^<html^> >> %F%
echo   ^<head^> >> %F%
echo     ^<link rel="stylesheet" type="text/css" href="style.css"^> >> %F%
echo   ^<head^> >> %F%
echo   ^<body^> >> %F%
echo     ^<div id="topbar"^> >> %F%
echo       ^<span^>Current^</span^> >> %F%
echo       ^<span^>Reference^</span^> >> %F%
echo       ^<span^>Comparison^</span^> >> %F%
echo     ^</div^> >> %F%
echo     ^<div id="topmargin"^>^</div^> >> %F%
FOR /D %%a IN (%SRC%) DO (
      echo     ^<h2 id="%%a"^>%%a ^<a class="toc-anchor" href="#%%a"^>#^</a^>^</h2^> >> %F%
      echo     ^<div^> >> %F%
      echo       ^<img src="%%a-1.png"^> >> %F%
      echo       ^<img src="%%a-ref.png"^> >> %F%
      echo       ^<img src="%%a-diff.png"^> >> %F%
      echo     ^</div^> >> %F%
)
echo   ^</body^> >> %F%
echo ^</html^> >> %F%

%F%
cd ..

