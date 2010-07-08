# - Define macro to determine architecture
#
# TEST_X64( VARIABLE )
#     Check if the system is 64bit or not
#     Arguments:
#         VARIABLE - variable to store the result to
#

MACRO( TEST_X64 VARIABLE )
    IF( NOT DEFINED ${VARIABLE} )
        MESSAGE( STATUS "Check if the system is 64bit" )

        IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            MESSAGE( STATUS "Check if the system is 64bit - yes" )
            SET( ${VARIABLE} 1 CACHE INTERNAL "Result of TEST_X64" FORCE )
        ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            MESSAGE( STATUS "Check if the system is 64bit - no" )
            SET( ${VARIABLE} 0 CACHE INTERNAL "Result of TEST_X64" FORCE )
        ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    ENDIF( NOT DEFINED ${VARIABLE} )
ENDMACRO( TEST_X64 )
