@echo off

set buildDir=%~dp0build

if exist %buildDir% (  
  rd /s /q %buildDir%
)
