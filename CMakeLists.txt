cmake_minimum_required(VERSION 3.19)
project(GnosiCore LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Network)

qt_standard_project_setup()

qt_add_executable(GnosiCore
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_translations(
    TARGETS GnosiCore
    TS_FILES GnosiCore_ko_KR.ts
)

target_link_libraries(GnosiCore
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

install(TARGETS GnosiCore
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET GnosiCore
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
