cmake_minimum_required(VERSION 3.20)

find_package(Doxygen)

if (NOT Doxygen_FOUND)
    message(WARNING "Doxygen not found. Targets for building documentation are not available")
    return()
endif ()

set(DOXYGEN_EXCLUDE_SYMBOLS "std,std::*,detail::*,*::detail::*,*::detail")

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_JAVADOC_AUTOBRIEF YES)

set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set(DOXYGEN_PROJECT_LOGO "${PROJECT_SOURCE_DIR}/img/small_logo.jpg")

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN YES)

doxygen_add_docs(api_doc
        "${PROJECT_SOURCE_DIR}/README.md"
        "${PROJECT_SOURCE_DIR}/include"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
)
