@echo off

"D:\Program Files (x86)\Google Protobuf\protoc.exe" --proto_path=".\source\protobuf" --proto_path="D:\Program Files (x86)\Google Protobuf\src" --cpp_out=".\source\protobuf\generated" ".\source\protobuf\cstrike15_usermessages_public.proto"
"D:\Program Files (x86)\Google Protobuf\protoc.exe" --proto_path=".\source\protobuf" --proto_path="D:\Program Files (x86)\Google Protobuf\src" --cpp_out=".\source\protobuf\generated" ".\source\protobuf\netmessages_public.proto"
