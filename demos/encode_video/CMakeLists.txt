link_libraries(${PROJECT_NAME})
set(prgs
  encode_to_MPEG2_video
  encode_to_MPEG4_video
  encode_to_H264_video
  encode_MJPEG_webcam_to_VP9_video
)
foreach(prg ${prgs})
  include_directories(../../sources)
  add_executable(${prg})
  target_sources(${prg} PRIVATE ${prg}.cpp)
  add_dependencies(${prg} ${PROJECT_NAME})
endforeach()
