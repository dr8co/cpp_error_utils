cmake_minimum_required(VERSION 3.20)

cmake_dependent_option(CPP_ERR_DOC_DARK_THEME "Package the library" ON "CPP_ERR_BUILD_DOC" OFF)

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
set(DOXYGEN_PROJECT_ICON "${PROJECT_SOURCE_DIR}/img/favicon.ico")

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN YES)

# Configure theming of the documentation
if (CPP_ERR_DOC_DARK_THEME)
    include(FetchContent)
    fetchcontent_declare(
            doxygen-awesome-css
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
            GIT_TAG main
    )
    fetchcontent_makeavailable(doxygen-awesome-css)

    # Save the location the files were cloned into
    # This allows us to get the path to doxygen-awesome.css
    fetchcontent_getproperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_DISABLE_INDEX NO)
    set(DOXYGEN_FULL_SIDEBAR NO)
    set(DOXYGEN_HTML_COPY_CLIPBOARD NO) # Doxygen >= 1.10.0
    set(DOXYGEN_HTML_COLORSTYLE LIGHT)

    set(DOXYGEN_HTML_EXTRA_STYLESHEET
            "${AWESOME_CSS_DIR}/doxygen-awesome.css"
            "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only.css"
            "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
    )

    # Set the generated header file as the header file for the documentation
    set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_BINARY_DIR}/header.html")

    set(DOXYGEN_HTML_EXTRA_FILES
            # Dark Mode Toggle
            "${AWESOME_CSS_DIR}/doxygen-awesome-darkmode-toggle.js"

            # Fragment Copy Button
            "${AWESOME_CSS_DIR}/doxygen-awesome-fragment-copy-button.js"

            # Paragraph Linking
            "${AWESOME_CSS_DIR}/doxygen-awesome-paragraph-link.js"

            # Interactive TOC
            "${AWESOME_CSS_DIR}/doxygen-awesome-interactive-toc.js"

    )

    # Configure the header file
    execute_process(COMMAND "${PROJECT_SOURCE_DIR}/scripts/edit_doxyfile_headers.sh"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            TIMEOUT 10
            COMMAND_ERROR_IS_FATAL ANY
    )

endif ()

doxygen_add_docs(api_docs
        "${PROJECT_SOURCE_DIR}/README.md"
        "${PROJECT_SOURCE_DIR}/include"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
)
