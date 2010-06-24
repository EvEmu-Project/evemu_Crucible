# - Define macro to build precompiled header for a target
#
# TARGET_BUILD_PCH( TARGET PRECOMPILED_HEADER BOUND_SOURCE )
#     Builds a precompiled header while building target; currently MSVC support only.
#     Arguments:
#         TARGET             - target to which add the precompiled header
#         PRECOMPILED_HEADER - the header itself
#         BOUND_SOURCE       - the source the header is bound to, i.e. it is built during
#                              compilation of given source file
#

MACRO( TARGET_BUILD_PCH TARGET PRECOMPILED_HEADER BOUND_SOURCE )
    IF( MSVC )
        GET_PROPERTY( FLAGS TARGET ${TARGET} PROPERTY COMPILE_FLAGS )
        SET_PROPERTY( TARGET ${TARGET}       PROPERTY COMPILE_FLAGS "${FLAGS} /Yu\"${PRECOMPILED_HEADER}\"" )
        SET_PROPERTY( SOURCE ${BOUND_SOURCE} PROPERTY COMPILE_FLAGS "${FLAGS} /Yc\"${PRECOMPILED_HEADER}\"" )
    ENDIF( MSVC )
ENDMACRO( TARGET_BUILD_PCH )
