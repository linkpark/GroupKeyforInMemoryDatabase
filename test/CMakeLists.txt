INCLUDE_DIRECTORIES( ${GROUPKEY_SOURCE_DIR}/src )
INCLUDE_DIRECTORIES( ${GROUPKEY_SOURCE_DIR}/third_party )

#SET( TEST_LISTS main.cpp DGroupKeyTest.cpp)
SET( TEST_LISTS main.cpp ColumnTableTest.cpp TableManagerTest.cpp BitCompressedVectorTest.cpp OrderedDictionaryTest.cpp DGroupKeyTest.cpp ToolSetTest.cpp)
#SET( TEST_LISTS main.cpp ColumnTableTest.cpp OrderedDictionaryTest.cpp)
SET( SRC_LISTS_DIR ${GROUPKEY_SOURCE_DIR}/src  )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

AUX_SOURCE_DIRECTORY( ${SRC_LISTS_DIR} DIR_SRCS )
#AUX_SOURCE_DIRECTORY( ${SRC_LISTS_DIR}/log DIR_SRCS_LOG )
#AUX_SOURCE_DIRECTORY( ${SRC_LISTS_DIR}/mutex DIR_SRCS_MUTEX )

#if CMAKE_C_COMPILER is not defined, it occurs error when compiling flex
SET(CMAKE_C_COMPILER "g++") 
ADD_DEFINITIONS("-g")
ADD_DEFINITIONS("-DDBUGGROUPKEY")
ADD_DEFINITIONS("--std=c++11")

ADD_EXECUTABLE( test ${TEST_LISTS} ${DIR_SRCS} )
#ADD_EXECUTABLE( pressure-test PressureTest.cpp ${DIR_SRCS})

TARGET_LINK_LIBRARIES(test ${GROUPKEY_SOURCE_DIR}/lib/libprotobuf.a ${GROUPKEY_SOURCE_DIR}/lib/libgtest.a ${GROUPKEY_SOURCE_DIR}/lib/libglog.a pthread boost_system boost_serialization )
#TARGET_LINK_LIBRARIES(pressure-test ${GROUPKEY_SOURCE_DIR}/lib/libprotobuf.a ${GROUPKEY_SOURCE_DIR}/lib/libgtest.a ${GROUPKEY_SOURCE_DIR}/lib/libglog.a pthread boost_system ${GROUPKEY_SOURCE_DIR}/lib/libboost_serialization.a)

