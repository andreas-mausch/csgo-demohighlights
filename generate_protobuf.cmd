@echo off

protobuf\protoc.exe --proto_path=.\protobuf --cpp_out=.\source\protobuf protobuf\cstrike15_usermessages_public.proto
protobuf\protoc.exe --proto_path=.\protobuf --cpp_out=.\source\protobuf protobuf\netmessages_public.proto
