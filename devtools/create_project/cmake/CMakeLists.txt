cmake_minimum_required(VERSION 3.2)
project(create_project)


set(SOURCE_FILES
	../build_setup.cpp
	../build_setup.h
	../cmake.cpp
	../cmake.h
	../codeblocks.cpp
	../codeblocks.h
	../create_project.cpp
	../create_project.h
	../msbuild.cpp
	../msbuild.h
	../msvc.cpp
	../msvc.h
	../project_provider.cpp
	../project_provider.h
	../xcode.cpp
	../xcode.h
    )

add_executable(create_project ${SOURCE_FILES})
if (WIN32)
    target_link_libraries(create_project rpcrt4 advapi32)
endif ()
set_property(TARGET create_project PROPERTY CXX_STANDARD 11)
set_property(TARGET create_project PROPERTY CXX_STANDARD_REQUIRED ON)
