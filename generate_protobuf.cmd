@echo off

"D:\Program Files (x86)\Google Protobuf\protoc.exe" --proto_path=".\protobuf" --proto_path="D:\Program Files (x86)\Google Protobuf\src" --cpp_out=.\source\protobuf .\protobuf\cstrike15_usermessages_public.proto
"D:\Program Files (x86)\Google Protobuf\protoc.exe" --proto_path=".\protobuf" --proto_path="D:\Program Files (x86)\Google Protobuf\src" --cpp_out=.\source\protobuf .\protobuf\netmessages_public.proto
