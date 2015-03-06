
# enable/disable cmake debug messages related to this pile
set (PROGRESS_DEBUG_MSG OFF)

# make sure support code is present; no harm
# in including it twice; the user, however, should have used
# pileInclude() from pile_support.cmake module.
include(pile_support)

# initialize this module
macro    (progressInit
          ref_cnt_use_mode)

    # default name
    if (NOT PROGRESS_INIT_NAME)
        set(PROGRESS_INIT_NAME "Progress")
    endif ()

    # compose the list of headers and sources
    set(PROGRESS_HEADERS
        "progress.h")
    set(PROGRESS_SOURCES
        "progress.cc")
    set(PROGRESS_QT_MODS
        "Core")

    pileSetSources(
        "${PROGRESS_INIT_NAME}"
        "${PROGRESS_HEADERS}"
        "${PROGRESS_SOURCES}")

    pileSetCommon(
        "${PROGRESS_INIT_NAME}"
        "0;0;1;d"
        "ON"
        "${ref_cnt_use_mode}"
        ""
        "category1"
        "tag1;tag2")

endmacro ()
