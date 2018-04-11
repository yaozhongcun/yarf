#!/usr/bin/python
# -*- coding: utf-8 -*-

import os,re,sys
import platform
import google.protobuf.text_format
import google.protobuf.json_format
import google.protobuf.descriptor
import google.protobuf.descriptor_pb2

"""
不支持以下数据类型
    # TYPE_FLOAT TYPE_FIXED64 TYPE_FIXED32 
    # TYPE_GROUP 
    # TYPE_BYTES 
    # TYPE_ENUM TYPE_SFIXED32 TYPE_SFIXED64 TYPE_SINT32 TYPE_SINT64

不支持import其他的proto文件    
"""

def islinux():
    a=platform.platform()
    if (1 == a.find("inux") ):
        return 1 
    else:
        return 0

def iswin():
    a=platform.platform()
    if (1 == a.find("indows") ):
        return 1 
    else:
        return 0

def first2upper(str):
    head = True
    ret = ""
    for ch in str:
        if (ch == '_'):
            head = True
            continue

        if (head):
            ret += ch.upper()
            head = False
        else:
            ret += ch
    return ret

#############################################################
# .conf file
conf_single_field="""$$field$$: 0 
"""

conf_string_field="""$$field$$: "" 
"""

conf_struct_field_begin="""$$field$$: {
"""

conf_struct_field_end="""}
"""

#############################################################
# .root_conf file
g_all_msg={}

def gen_single_code(field, indent):
    ret = ""
    for idx in range(1, indent):
    	ret = ret + " ";
    if (field.type == field.TYPE_MESSAGE):
    	field_name_vec = field.type_name.split('.')
    	field_type = field_name_vec[ len(field_name_vec)-1 ]

    	if ( None != g_all_msg[field_type] ):
    		ret = ret + conf_struct_field_begin.replace("$$field$$", field.name)
    		ret = ret + gen_code(g_all_msg[field_type], indent + 2)
    		for idx in range(1, indent):
				ret = ret + " ";
    		ret = ret + conf_struct_field_end
    	else:
    		print("no such field type", field_type)
    		return

    elif (field.type == field.TYPE_STRING):
    	ret = ret + conf_string_field.replace("$$field$$", field.name)
    else:
    	ret = ret + conf_single_field.replace("$$field$$", field.name)

    return ret

def gen_code(root_conf, indent):
    #### field opt in class/encode/decode
    ret = ""
    for field in root_conf.field:
    	ret = ret + gen_single_code(field, indent)
        if (field.label == field.LABEL_REPEATED):
        	ret = ret + gen_single_code(field, indent)

    return ret

def get_protodesc(file_name, file_pre):

    ############################################
    if iswin():
        commond = "protoc.exe --python_out=. " + " ./" + file_name
    else:
        commond = "./protoc --python_out=. " + " ./" + file_name
    os.system(commond)

    if iswin():
        commond = "protoc.exe --cpp_out=. " + " ./" + file_name
    else:
        commond = "./protoc --cpp_out=. " + " ./" + file_name
    os.system(commond)

    pbf = open(file_pre + "_pb2.py")
    line = pbf.readline()
    file_pb = ""
    while line:
        if -1 != line.find("serialized_pb"):
            file_pb=line[line.find("'")+1:line.rfind("'")]
            file_pb = file_pb.decode("string_escape")
            break
        line = pbf.readline()
    pbf.close()

    proto_desc = google.protobuf.descriptor_pb2.FileDescriptorProto()
    proto_desc.ParseFromString(file_pb)

    return proto_desc


def handle_proto(file_pre, root_conf):
    file_name = file_pre + ".proto"
    
    proto_desc = get_protodesc(file_name, file_pre)
    
    root_type = None  
    for msg_type in proto_desc.message_type:
		g_all_msg[msg_type.name] = msg_type
		if ( msg_type.name == root_conf):
			root_type = msg_type

    output = gen_code(root_type, 0)

    conf_file_name = file_pre+".pb.txt"
    conf_file = open(conf_file_name, 'w')
    conf_file.write(output)

def main():
    file_name=raw_input("input proto file name: ")
    root_conf=raw_input("input root conf: ")
    handle_proto(file_name, root_conf)
    file_name=raw_input("press enter to quit")

main()
