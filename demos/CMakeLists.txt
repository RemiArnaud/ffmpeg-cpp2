set(dirs
  demo
  decode_audio
  decode_video
  difference
  encode_audio
  encode_video
  filtering_audio
  filtering_video
  print_info
  remuxing
  simple_interface_demo
  webcam_to_video
)
foreach(dir ${dirs})
add_subdirectory(${dir})
endforeach()
