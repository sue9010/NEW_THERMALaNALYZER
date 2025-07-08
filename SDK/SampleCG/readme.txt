================================================================================
    SampleCG Project File Overview
================================================================================

This document provides an overview of the key files within the SampleCG project and their respective roles.

SampleCG.sln
    This is the Visual Studio Solution file. It organizes the project and contains build configurations.

SampleCG.v12.suo
    This is a Visual Studio User Options file, storing user-specific settings for the solution.

SampleCG/ (Folder)
    This folder contains the actual project source code and resource files.

    CameraSetupDlg.cpp / CameraSetupDlg.h
        These files define the `CCameraSetupDlg` class, which implements a dialog box for configuring camera settings. It allows users to adjust various camera parameters such as display options, video size, video adjustments (brightness, contrast, zoom, rotation, palette), image adjustments (AGC, noise filters, edge enhancement), NUC (Non-Uniformity Correction), network settings, alarm outputs, alarm configurations, correction parameters (emissivity, transmission, atmosphere), and isotherm settings. It communicates these settings to the main application.

    ReadMe.txt
        This file provides an overview of the project and descriptions of its constituent files. (This file itself)

    resource.h
        This is a standard header file that defines resource IDs used throughout the application.

    SampleCG.aps
        This is a binary resource file used by the Visual C++ resource editor.

    SampleCG.cpp
        This is the main application source file, containing the `CSampleCGApp` application class.

    SampleCG.h
        This is the main header file for the application, including other project-specific headers and declaring the `CSampleCGApp` application class.

    SampleCG.rc
        This file lists all Microsoft Windows resources used by the program, including icons, bitmaps, and cursors.

    SampleCG.vcxproj
        This is the main project file for VC++ projects, containing information about the Visual C++ version, platforms, configurations, and project features.

    SampleCG.vcxproj.filters
        This file defines filters for VC++ projects, associating files with specific groupings (e.g., "Source Files") for display in the IDE.

    SampleCG.vcxproj.user
        This file stores user-specific settings for the Visual Studio project.

    SampleCGDlg.cpp / SampleCGDlg.h
        These files define the `CSampleCGDlg` class, which represents the main dialog of the application and defines its behavior.

    ScreenWnd.cpp / ScreenWnd.h
        These files define the `CScreenWnd` class, a custom static control used to display the thermal camera's video feed. It handles image rendering, scaling, offsetting, flipping, and mirroring. It also supports overlaying information such as mouse-hover temperature, hot/cold spot temperatures, and general messages. It uses GDI+ for graphics processing and handles various user interaction events.

    stdafx.cpp / stdafx.h
        These files are used to build a precompiled header (PCH) file (`SampleCG.pch`) and a precompiled types file (`StdAfx.obj`).

    targetver.h
        This header file contains Windows SDK version information.

    Debug/ (Folder)
        Contains files related to the debug build of the project.

    Release/ (Folder)
        Contains files related to the release build of the project.

    res/ (Folder)
        Contains resource files for the application.
        - SampleCG.ico: The application's icon file.
        - SampleCG.rc2: Contains resources not typically edited by the Visual C++ resource editor.

    x64/ (Folder)
        Contains build-related files for the 64-bit architecture.
