@echo off

if exist docs\refman.pdf (
    del docs\refman.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_cpp.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_cpp.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_java.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_java.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_python.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_python.pdf
)

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: C++

:LBL_C++

@echo ==================================
@echo Generating C++ Documentation Files
@echo ==================================

timeout /t 2 > nul

doxygen docs\__internal__\Doxyfile_C++

@echo.
@echo.

xcopy docs\__internal__\make.bat docs\cpp\latex /y 

call docs\cpp\latex\make.bat

if exist docs\refman.pdf (
    del docs\refman.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_cpp.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_cpp.pdf
)

xcopy docs\cpp\latex\refman.pdf docs\ /y 
ren docs\refman.pdf gruppe2_verkehrszeichenerkennung_entwicklerdoku_cpp.pdf

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Java

:LBL_JAVA

@echo ===================================
@echo Generating Java Documentation Files
@echo ===================================

timeout /t 2 > nul

doxygen docs\__internal__\Doxyfile_Java

@echo.
@echo.

xcopy docs\__internal__\make.bat docs\java\latex /y 

call docs\java\latex\make.bat

if exist docs\refman.pdf (
    del docs\refman.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_java.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_java.pdf
)

xcopy docs\java\latex\refman.pdf docs\ /y 
ren docs\refman.pdf gruppe2_verkehrszeichenerkennung_entwicklerdoku_java.pdf

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Python

:LBL_PY

@echo =====================================
@echo Generating Python Documentation Files
@echo =====================================

timeout /t 2 > nul

doxygen docs\__internal__\Doxyfile_Python

@echo.
@echo.

xcopy docs\__internal__\make.bat docs\python\latex /y 

call docs\python\latex\make.bat

if exist docs\refman.pdf (
    del docs\refman.pdf
)

if exist docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_python.pdf (
    del docs\gruppe2_verkehrszeichenerkennung_entwicklerdoku_python.pdf
)

xcopy docs\python\latex\refman.pdf docs\ /y 
ren docs\refman.pdf gruppe2_verkehrszeichenerkennung_entwicklerdoku_python.pdf

pause