set(COMPONENT_SRCS 
    "main.cpp" 
    "espobject.cpp" 
    "espapp.cpp" 
    "event.cpp" 
    "wifispot.cpp" 
    "connecting.cpp" 
    "serverjoin.cpp" 
    "storage.cpp"
    "protocol.cpp"
    "monitoring.cpp"
    "door.cpp"
    )
set(COMPONENT_ADD_INCLUDEDIRS "")
add_definitions(-DARDUINOJSON_ENABLE_STD_STRING=1)
register_component()
