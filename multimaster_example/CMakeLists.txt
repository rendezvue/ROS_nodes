# %Tag(FULLTEXT)%
cmake_minimum_required(VERSION 2.8.3)
project(multimaster_example)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
MESSAGE("FALGS: ${CMAKE_CXX_FLAGS}")

## Find catkin and any catkin packages
find_package(catkin REQUIRED COMPONENTS 
    roscpp 
    rospy 
    roslib 
    std_msgs
    nav_msgs)

## Declare ROS messages and services
#add_message_files(FILES Num.msg)
add_service_files(FILES AddTwoInts.srv)

## Generate added messages and services
generate_messages(
    DEPENDENCIES
    std_msgs
)

## Declare a catkin package
catkin_package()

## Build talker and listener
include_directories(include ${catkin_INCLUDE_DIRS})

add_executable(multimaster_example src/multimaster_example.cpp)
target_link_libraries(multimaster_example ${catkin_LIBRARIES})

add_executable(multimaster_odom src/multimaster_odom.cpp)
target_link_libraries(multimaster_odom ${catkin_LIBRARIES})

add_executable(multimaster_service_call src/multimaster_service_call.cpp)
target_link_libraries(multimaster_service_call ${catkin_LIBRARIES})
add_dependencies(multimaster_service_call multimaster_example_generate_messages_cpp)

add_executable(multimaster_service_call_client src/multimaster_service_call_client.cpp)
target_link_libraries(multimaster_service_call_client ${catkin_LIBRARIES})
add_dependencies(multimaster_service_call_client multimaster_example_generate_messages_cpp)

add_executable(talker src/talker.cpp)
target_link_libraries(talker ${catkin_LIBRARIES})
add_dependencies(talker multimaster_example_generate_messages_cpp)


# %EndTag(FULLTEXT)%
