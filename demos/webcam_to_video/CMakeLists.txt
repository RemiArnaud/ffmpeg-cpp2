link_libraries(${PROJECT_NAME})
set(prgs
  webcam_stream_to_MKV
  webcam_stream_to_MPEG2
  webcam_stream_to_MPEG4
  webcam_stream_to_SOUND
  webcam_stream_to_VP9
  remux_webcam
  remux_webcam_to_MKV
)
set(prgsX
  remux_webcam
  remux_webcam_to_MKV
)

foreach(prg ${prgs})
  include_directories(../../sources)
  add_executable(${prg})
  target_sources(${prg} PRIVATE ${prg}.cpp getch.c)
  add_dependencies(${prg} ${PROJECT_NAME})
endforeach()
