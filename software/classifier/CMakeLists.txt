cmake_minimum_required (VERSION 2.6)
project (classifier)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

find_package( OpenCV REQUIRED )



#find_package( Caffe REQUIRED)
set(Caffe_INCLUDE_DIRS "/home/guada/dev/caffe/build/install/include;/usr/include/atlas;/usr/include")
set(Caffe_LIBRARIES "/home/guada/dev/caffe/build/lib/libcaffe.so")

find_package(Boost COMPONENTS system filesystem REQUIRED)

set(GLOG_LIBRARIES /usr/lib/x86_64-linux-gnu/libglog.so)

message( "boost = " ${Boost_SYSTEM_LIBRARY})

include_directories( ${OpenCV_INCLUDE_DIRS} ${Caffe_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
add_executable(classifier ${SOURCES})
target_link_libraries( classifier serial ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${Boost_LIBRARIES}  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} ${GLOG_LIBRARIES})

