# - Define a function to build precompiled header for a target
#
# TARGET_BUILD_PCH( TARGET HEADER SOURCE )
#     Builds a precompiled header while building target.
#     Arguments:
#         TARGET - target to which add the precompiled header
#         HEADER - the header itself
#         SOURCE - the source the header is bound to, i.e. it is built during
#                  compilation of given source file
#

FUNCTION( TARGET_BUILD_PCH TARGET HEADER SOURCE )
    GET_FILENAME_COMPONENT( HEADER      "${HEADER}" ABSOLUTE )
    GET_FILENAME_COMPONENT( HEADER_NAME "${HEADER}" NAME )

    IF( MSVC )
        SET_PROPERTY( TARGET "${TARGET}" APPEND PROPERTY COMPILE_FLAGS "/Yu\"${HEADER_NAME}\"" )
        SET_PROPERTY( SOURCE "${SOURCE}" APPEND PROPERTY COMPILE_FLAGS "/Yc\"${HEADER_NAME}\"" )
    ELSE( MSVC )
        SET( CMAKE_CURRENT_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${TARGET}.dir" )
        SET( HEADER_GCH "${CMAKE_CURRENT_OUTPUT_DIR}/${HEADER_NAME}.gch" )

        INCLUDE_DIRECTORIES( BEFORE "${CMAKE_CURRENT_OUTPUT_DIR}" )
        ADD_CUSTOM_COMMAND( OUTPUT "${HEADER_GCH}"
                            COMMAND "${CMAKE_CXX_COMPILER}"
                                    ARGS "$(CXX_DEFINES)"
                                         "$(CXX_FLAGS)"
                                         -o "${HEADER_GCH}"
                                         -c "${HEADER}"
                            DEPENDS "${CMAKE_CURRENT_OUTPUT_DIR}/flags.make"
                            IMPLICIT_DEPENDS "CXX" "${HEADER}" )

        SET_PROPERTY( SOURCE ${SOURCE} APPEND PROPERTY OBJECT_DEPENDS "${HEADER_GCH}" )
    ENDIF( MSVC )
ENDFUNCTION( TARGET_BUILD_PCH )
