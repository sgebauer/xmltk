# Microsoft Developer Studio Project File - Name="xmltkdfa" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xmltkdfa - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "xmltkdfa.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "xmltkdfa.mak" CFG="xmltkdfa - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "xmltkdfa - Win32 Release" ("Win32 (x86) Static Library" 用)
!MESSAGE "xmltkdfa - Win32 Debug" ("Win32 (x86) Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xmltkdfa - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /I "..\include" /D "NDEBUG" /D "_WINDOWS" /D "TOKEN_PARSER" /D "WIN32" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "xmltkdfa - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I ".\Win32" /I "..\include" /D "_DEBUG" /D "_WINDOWS" /D "TOKEN_PARSER" /D "WIN32" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "xmltkdfa - Win32 Release"
# Name "xmltkdfa - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\addressComp.c
# End Source File
# Begin Source File

SOURCE=.\Automata.cxx
# End Source File
# Begin Source File

SOURCE=.\dfafilter.cxx
# End Source File
# Begin Source File

SOURCE=.\Error.cxx
# End Source File
# Begin Source File

SOURCE=.\List.cxx
# End Source File
# Begin Source File

SOURCE=.\Node.cxx
# End Source File
# Begin Source File

SOURCE=.\Query.cxx
# End Source File
# Begin Source File

SOURCE=.\Root.cxx
# End Source File
# Begin Source File

SOURCE=.\Variable.cxx
# End Source File
# Begin Source File

SOURCE=.\XPath.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Automata.h
# End Source File
# Begin Source File

SOURCE=.\Base.h
# End Source File
# Begin Source File

SOURCE=.\Error.h
# End Source File
# Begin Source File

SOURCE=.\List.h
# End Source File
# Begin Source File

SOURCE=.\Node.h
# End Source File
# Begin Source File

SOURCE=.\operator.h
# End Source File
# Begin Source File

SOURCE=.\Predicate.h
# End Source File
# Begin Source File

SOURCE=.\Query.h
# End Source File
# Begin Source File

SOURCE=.\Root.h
# End Source File
# Begin Source File

SOURCE=.\Variable.h
# End Source File
# Begin Source File

SOURCE=.\XPath.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Yacc Files"

# PROP Default_Filter "y"
# Begin Source File

SOURCE=.\xmatch.y

!IF  "$(CFG)" == "xmltkdfa - Win32 Release"

# Begin Custom Build
InputPath=.\xmatch.y

BuildCmds= \
	bison --yacc --defines --no-lines --verbose $(InputPath) \
	move /y y.tab.c y.tab.cxx \
	move /y y.tab.h xmatch.tab.h \
	

"y.tab.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"xmatch.tab.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"y.output" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "xmltkdfa - Win32 Debug"

# Begin Custom Build
InputPath=.\xmatch.y

BuildCmds= \
	bison --yacc --defines --no-lines --verbose $(InputPath) \
	move /y y.tab.c y.tab.cxx \
	move /y y.tab.h xmatch.tab.h \
	

"y.tab.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"xmatch.tab.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"y.output" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Lex Files"

# PROP Default_Filter "l"
# Begin Source File

SOURCE=.\xmatch.l

!IF  "$(CFG)" == "xmltkdfa - Win32 Release"

# Begin Custom Build
InputPath=.\xmatch.l

"lex.yy.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex $(InputPath) 
	move /y lex.yy.c lex.yy.cxx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "xmltkdfa - Win32 Debug"

# Begin Custom Build
InputPath=.\xmatch.l

"lex.yy.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex $(InputPath) 
	move /y lex.yy.c lex.yy.cxx 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\lex.yy.cxx
# End Source File
# Begin Source File

SOURCE=.\makefile.token
# End Source File
# Begin Source File

SOURCE=.\makefile.xerces
# End Source File
# Begin Source File

SOURCE=.\Win32\xmatch.h
# End Source File
# Begin Source File

SOURCE=.\y.tab.cxx
# End Source File
# Begin Source File

SOURCE=.\y.tab.h
# End Source File
# End Target
# End Project
