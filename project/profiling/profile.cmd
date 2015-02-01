set pt="C:\Program Files (x86)\Microsoft Visual Studio 10.0\Team Tools\Performance Tools"

c:
cd "C:\Users\neonew\Documents\Programmieren\csgo-demohighlights\project\Release-Profile"

%pt%\vsperfcmd /admin:driver,start /admin:service,start
%pt%\vsperfcmd /start:sample /output:profiling.vsp
%pt%\vsperfcmd /launch:csgo-demohighlights.exe
%pt%\vsperfcmd /shutdown
%pt%\vsperfreport /summary:all profiling.vsp
