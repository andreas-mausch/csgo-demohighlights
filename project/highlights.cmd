@echo off
rem
rem clutches, 4K and 5K
rem

set demodir="C:\Users\neonew\Documents\Programmieren\csgo-demohighlights\project\demos"
set exe="C:\Users\neonew\Documents\Programmieren\csgo-demohighlights\project\Release\csgo-demohighlights.exe"

forfiles /m *.dem /p %demodir% /c "cmd /c echo @path & %exe% @path | grep -e 1vs2 -e 1vs3 -e 1vs4 -e 1vs5 -e 4K -e 5K"
