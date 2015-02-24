if (NOT PROGRESS_BUILD_MODE)
    set (PROGRESS_BUILD_MODE STATIC)
endif ()

include(pile_support)
pileInclude (Progress)
progressInit(${PROGRESS_BUILD_MODE})
