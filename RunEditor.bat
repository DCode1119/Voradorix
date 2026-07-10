@echo off
setlocal

pushd "%~dp0Editor"
npm run dev
popd

endlocal
