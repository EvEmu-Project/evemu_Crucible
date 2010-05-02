#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "eve-common" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${COMPONENT_NAME}" )
SET( COMPONENT_PACKETS_DIR "${PROJECT_BINARY_DIR}/src/${COMPONENT_NAME}" )

#########
# Files #
#########
# eve-common
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/EVECommonPCH.h"
     "${COMPONENT_INCLUDE_DIR}/EVEmuRevision.h"
     "${COMPONENT_INCLUDE_DIR}/EVEVersion.h" )
SET( SOURCE
     "" )

# eve-common/cache
SET( cache_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/cache/CachedObjectMgr.h" )
SET( cache_SOURCE
     "${COMPONENT_SOURCE_DIR}/cache/CachedObjectMgr.cpp" )

# eve-common/database
SET( database_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/database/EVEDBUtils.h"
     "${COMPONENT_INCLUDE_DIR}/database/RowsetReader.h"
     "${COMPONENT_INCLUDE_DIR}/database/RowsetToSQL.h" )
SET( database_SOURCE
     "${COMPONENT_SOURCE_DIR}/database/EVEDBUtils.cpp"
     "${COMPONENT_SOURCE_DIR}/database/RowsetReader.cpp"
     "${COMPONENT_SOURCE_DIR}/database/RowsetToSQL.cpp" )

# eve-common/destiny
SET( destiny_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/destiny/DestinyBinDump.h"
     "${COMPONENT_INCLUDE_DIR}/destiny/DestinyStructs.h" )
SET( destiny_SOURCE
     "${COMPONENT_SOURCE_DIR}/destiny/DestinyBinDump.cpp" )

# eve-common/marshal
SET( marshal_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/marshal/EVEMarshal.h"
     "${COMPONENT_INCLUDE_DIR}/marshal/EVEMarshalOpcodes.h"
     "${COMPONENT_INCLUDE_DIR}/marshal/EVEMarshalStringTable.h"
     "${COMPONENT_INCLUDE_DIR}/marshal/EVEUnmarshal.h" )
SET( marshal_SOURCE
     "${COMPONENT_SOURCE_DIR}/marshal/EVEMarshal.cpp"
     "${COMPONENT_SOURCE_DIR}/marshal/EVEMarshalStringTable.cpp"
     "${COMPONENT_SOURCE_DIR}/marshal/EVEUnmarshal.cpp" )

# eve-common/network
SET( network_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/network/EVEPktDispatch.h"
     "${COMPONENT_INCLUDE_DIR}/network/EVESession.h"
     "${COMPONENT_INCLUDE_DIR}/network/EVETCPConnection.h"
     "${COMPONENT_INCLUDE_DIR}/network/EVETCPServer.h"
     "${COMPONENT_INCLUDE_DIR}/network/packet_types.h" )
SET( network_SOURCE
     "${COMPONENT_SOURCE_DIR}/network/EVEPktDispatch.cpp"
     "${COMPONENT_SOURCE_DIR}/network/EVESession.cpp"
     "${COMPONENT_SOURCE_DIR}/network/EVETCPConnection.cpp" )

# eve-common/packets
SET( packets_INCLUDE
     "${COMPONENT_PACKETS_DIR}/packets/AccountPkts.h"
     "${COMPONENT_PACKETS_DIR}/packets/CorporationPkts.h"
     "${COMPONENT_PACKETS_DIR}/packets/Crypto.h"
     "${COMPONENT_PACKETS_DIR}/packets/Destiny.h"
     "${COMPONENT_PACKETS_DIR}/packets/DogmaIM.h"
     "${COMPONENT_PACKETS_DIR}/packets/General.h"
     "${COMPONENT_PACKETS_DIR}/packets/Character.h"
     "${COMPONENT_PACKETS_DIR}/packets/Inventory.h"
     "${COMPONENT_PACKETS_DIR}/packets/Language.h"
     "${COMPONENT_PACKETS_DIR}/packets/LSCPkts.h"
     "${COMPONENT_PACKETS_DIR}/packets/Manufacturing.h"
     "${COMPONENT_PACKETS_DIR}/packets/Market.h"
     "${COMPONENT_PACKETS_DIR}/packets/Missions.h"
     "${COMPONENT_PACKETS_DIR}/packets/ObjectCaching.h"
     "${COMPONENT_PACKETS_DIR}/packets/Standing2.h"
     "${COMPONENT_PACKETS_DIR}/packets/Tutorial.h"
     "${COMPONENT_PACKETS_DIR}/packets/Wallet.h" )
SET( packets_SOURCE
     "${COMPONENT_PACKETS_DIR}/packets/AccountPkts.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/CorporationPkts.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Crypto.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Destiny.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/DogmaIM.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/General.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Character.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Inventory.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Language.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/LSCPkts.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Manufacturing.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Market.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Missions.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/ObjectCaching.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Standing2.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Tutorial.cpp"
     "${COMPONENT_PACKETS_DIR}/packets/Wallet.cpp" )
SET( packets_XMLP
     "${COMPONENT_SOURCE_DIR}/packets/AccountPkts.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/CorporationPkts.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Crypto.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Destiny.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/DogmaIM.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/General.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Character.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Inventory.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Language.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/LSCPkts.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Manufacturing.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Market.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Missions.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/ObjectCaching.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Standing2.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Tutorial.xmlp"
     "${COMPONENT_SOURCE_DIR}/packets/Wallet.xmlp" )

# eve-common/python
SET( python_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/python/PyDumpVisitor.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyLookupDump.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyPacket.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyRep.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyTraceLog.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyVisitor.h"
     "${COMPONENT_INCLUDE_DIR}/python/PyXMLGenerator.h" )
SET( python_SOURCE
     "${COMPONENT_SOURCE_DIR}/python/PyDumpVisitor.cpp"
     "${COMPONENT_SOURCE_DIR}/python/PyLookupDump.cpp"
     "${COMPONENT_SOURCE_DIR}/python/PyPacket.cpp"
     "${COMPONENT_SOURCE_DIR}/python/PyRep.cpp"
     "${COMPONENT_SOURCE_DIR}/python/PyVisitor.cpp"
     "${COMPONENT_SOURCE_DIR}/python/PyXMLGenerator.cpp" )

# eve-common/python/classes
SET( python_classes_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/python/classes/PyDatabase.h"
     "${COMPONENT_INCLUDE_DIR}/python/classes/PyExceptions.h"
     "${COMPONENT_INCLUDE_DIR}/python/classes/PyUtils.h" )
SET( python_classes_SOURCE
     "${COMPONENT_SOURCE_DIR}/python/classes/PyDatabase.cpp"
     "${COMPONENT_SOURCE_DIR}/python/classes/PyExceptions.cpp"
     "${COMPONENT_SOURCE_DIR}/python/classes/PyUtils.cpp" )

# eve-common/tables
SET( tables_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/tables/invCategories.h"
     "${COMPONENT_INCLUDE_DIR}/tables/invGroups.h" )
SET( tables_SOURCE
     "" )

# eve-common/utils
SET( utils_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/utils/EVEUtils.h" )
SET( utils_SOURCE
     "${COMPONENT_SOURCE_DIR}/utils/EVEUtils.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${dep_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${COMPONENT_PACKETS_DIR}" )

SOURCE_GROUP( "include"                  FILES ${INCLUDE} )
SOURCE_GROUP( "include\\cache"           FILES ${cache_INCLUDE} )
SOURCE_GROUP( "include\\database"        FILES ${database_INCLUDE} )
SOURCE_GROUP( "include\\destiny"         FILES ${destiny_INCLUDE} )
SOURCE_GROUP( "include\\marshal"         FILES ${marshal_INCLUDE} )
SOURCE_GROUP( "include\\network"         FILES ${network_INCLUDE} )
SOURCE_GROUP( "include\\packets"         FILES ${packets_INCLUDE} )
SOURCE_GROUP( "include\\python"          FILES ${python_INCLUDE} )
SOURCE_GROUP( "include\\python\\classes" FILES ${python_classes_INCLUDE} )
SOURCE_GROUP( "include\\tables"          FILES ${tables_INCLUDE} )
SOURCE_GROUP( "include\\utils"           FILES ${utils_INCLUDE} )

SOURCE_GROUP( "src"                  FILES ${SOURCE} )
SOURCE_GROUP( "src\\cache"           FILES ${cache_SOURCE} )
SOURCE_GROUP( "src\\database"        FILES ${database_SOURCE} )
SOURCE_GROUP( "src\\destiny"         FILES ${destiny_SOURCE} )
SOURCE_GROUP( "src\\marshal"         FILES ${marshal_SOURCE} )
SOURCE_GROUP( "src\\network"         FILES ${network_SOURCE} )
SOURCE_GROUP( "src\\packets"         FILES ${packets_SOURCE} )
SOURCE_GROUP( "src\\packets\\xmlp"   FILES ${packets_XMLP} )
SOURCE_GROUP( "src\\python"          FILES ${python_SOURCE} )
SOURCE_GROUP( "src\\python\\classes" FILES ${python_classes_SOURCE} )
SOURCE_GROUP( "src\\tables"          FILES ${tables_SOURCE} )
SOURCE_GROUP( "src\\utils"           FILES ${utils_SOURCE} )

FILE( MAKE_DIRECTORY "${COMPONENT_PACKETS_DIR}/packets" )
ADD_CUSTOM_COMMAND( OUTPUT ${packets_INCLUDE} ${packets_SOURCE}
                    COMMAND "xmlpktgen"
                    ARGS -I "${COMPONENT_PACKETS_DIR}/packets"
                         -S "${COMPONENT_PACKETS_DIR}/packets"
                         ${packets_XMLP}
                    DEPENDS "xmlpktgen"
                    COMMENT "Generating packet files..." )

ADD_LIBRARY( "${COMPONENT_NAME}"
             ${INCLUDE}                ${SOURCE}
             ${cache_INCLUDE}          ${cache_SOURCE}
             ${database_INCLUDE}       ${database_SOURCE}
             ${destiny_INCLUDE}        ${destiny_SOURCE}
             ${marshal_INCLUDE}        ${marshal_SOURCE}
             ${network_INCLUDE}        ${network_SOURCE}
             ${packets_INCLUDE}        ${packets_SOURCE}        ${packets_XMLP}
             ${python_INCLUDE}         ${python_SOURCE}
             ${python_classes_INCLUDE} ${python_classes_SOURCE}
             ${tables_INCLUDE}         ${tables_SOURCE}
             ${utils_INCLUDE}          ${utils_SOURCE} )

TARGET_BUILD_PCH( "${COMPONENT_NAME}" "EVECommonPCH.h" "utils/EVEUtils.cpp" )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
SET( ${COMPONENT_NAME}_PACKETS_DIR "${COMPONENT_PACKETS_DIR}" PARENT_SCOPE )
