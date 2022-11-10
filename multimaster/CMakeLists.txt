# %Tag(FULLTEXT)%
cmake_minimum_required(VERSION 2.8.3)
project(multimaster)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
MESSAGE("FALGS: ${CMAKE_CXX_FLAGS}")
find_package(Boost REQUIRED COMPONENTS system)
## Find catkin and any catkin packages
find_package(catkin  REQUIRED COMPONENTS roscpp rospy tf roslib std_msgs)

## Declare ROS messages and services
#add_message_files(FILES Num.msg)
#add_service_files(FILES AddTwoInts.srv)

## Generate added messages and services
#generate_messages(DEPENDENCIES)

## Declare a catkin package
catkin_package(  INCLUDE_DIRS include/multimaster/)

## Build talker and listener
include_directories(include ${Boost_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS})

add_library(topic_tools src/shape_shifter.cpp)
target_link_libraries(topic_tools ${catkin_LIBRARIES})
add_dependencies(topic_tools multimaster_generate_messages_cpp)

add_executable(main src/main.cpp src/multimaster.cpp src/relay_topic.cpp)
target_link_libraries(main topic_tools ${catkin_LIBRARIES} ${Boost_LIBRARIES})
#add_dependencies(subscriber beginner_tutorials_generate_messages_cpp)

add_executable(foreign2host src/foreign2host.cpp src/multimaster.cpp src/relay_topic.cpp)
target_link_libraries(foreign2host topic_tools ${catkin_LIBRARIES} ${Boost_LIBRARIES})
#add_dependencies(subscriber beginner_tutorials_generate_messages_cpp)


add_executable(talker2 src/talker2.cpp)
target_link_libraries(talker2 ${catkin_LIBRARIES} )
add_dependencies(talker2 multimaster_generate_messages_cpp)




# %EndTag(FULLTEXT)%
