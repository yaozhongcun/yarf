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

不支持字符串数组,    如果有需要,用结构体包装数组

所有proto文件在一个文件夹内

仅支持单层namespace
协议必须使用Proto作为namespace

其他注释说明
    1 |gen|max|20|keyopt|

TODO:

DONE:
  additional opt

建议
  1 支持多层namespace??

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
# .h file
g_proto_ns="Proto"

h_begin="""
#ifndef $$macro$$
#define $$macro$$

#include <string>
using std::string;

#include "$$file_pre$$.pb.h"
"""
h_include="""
#include "$$file_pre$$.pb.h"
#include "$$file_pre$$_rawdata.h"
"""

h_ns_begin="""
namespace $$ns$$
{
"""

h_class_begin="""
struct $$class_name$$ {"""

h_single_field="""
    // field $$field$$
    $$field_type$$ $$field$$;
"""

h_string_field="""
    // field $$field$$
    static const uint32_t MAX_$$field_upper$$_SIZE = $$max$$;
    char $$field$$[MAX_$$field_upper$$_SIZE + 1];
    uint32_t $$field$$_size;

    bool set_$$field$$(const char* input) {
        if (nullptr == input ) { return false; }
        uint32_t unreachable_len = MAX_$$field_upper$$_SIZE + 1;
        uint32_t cur_len = strnlen(input, unreachable_len);
        if ( cur_len >= unreachable_len) { return false;}

        strncpy(this->$$field$$, input, cur_len);
        this->$$field$$_size = cur_len;
        this->$$field$$[cur_len] = '0';
        return true;
    }
    bool set_$$field$$(const char* input, uint32_t len) {
        if (nullptr == input ) { return false; }
        if ( len >=  MAX_$$field_upper$$_SIZE + 1 ) { return false; }
        strncpy(this->$$field$$, input, len);
        this->$$field$$_size = len;
        this->$$field$$[len] = '0';
        return true;
    }
"""

h_vec_field="""
    // field $$field$$
    static const uint32_t MAX_$$field_upper$$_SIZE = $$max$$;
    $$field_type$$ $$field$$[MAX_$$field_upper$$_SIZE];
    uint32_t $$field$$_size;
    // field $$field$$ opt func
    bool Add$$field_first_up$$(const $$field_type$$& input);
    $$field_type$$* Add$$field_first_up$$();
    $$field_type$$* Find$$field_first_up$$ByIdx(uint32_t idx);
    bool Del$$field_first_up$$ByIdx(const uint32_t idx);
"""
h_vec_field_more="""    $$field_type$$* Find$$field_first_up$$ByKey($$field_key_type$$ $$field_key_name$$);
    bool Del$$field_first_up$$ByKey($$field_key_type$$ $$field_key_name$$);
"""

h_class_end="""
    // encode & decode
    bool ToProtobuf(
        Proto::$$ns$$::$$class_name$$* target) const;
    bool FromProtobuf(const char* buf, size_t len);
    bool FromProtobuf(const Proto::$$ns$$::$$class_name$$& input);
};"""


h_end="""

} // namespace $$ns$$

#endif  // $$macro$$
"""

#############################################################
# .cpp file

cc_begin="""#include "$$file_pre$$_rawdata.h"
"""

cc_encode_begin="""
bool $$ns$$::$$class_name$$::ToProtobuf(
    Proto::$$ns$$::$$class_name$$* target) const {
    if (nullptr == target) {
        return false;
    }"""

cc_encode_simple_field="""
    target->set_$$field$$(this->$$field$$);
"""

cc_encode_struct_field="""
    if ( !this->$$field$$.ToProtobuf(target->mutable_$$field$$()) ) {
        return false;
    }
"""

cc_encode_string_field="""
    target->set_$$field$$(this->$$field$$, this->$$field$$_size);
"""

cc_encode_simple_vec_field="""
    for (uint32_t idx = 0; idx < this->$$field$$_size; ++idx) {
        target->add_$$field$$(this->$$field$$[idx]);
    }
"""

cc_encode_struct_vec_field="""
    for (uint32_t idx = 0; idx < this->$$field$$_size; ++idx) {
        auto new_$$field$$ = target->add_$$field$$();
        if (nullptr == new_$$field$$) {
            return false;
        }
        if (!this->$$field$$[idx].ToProtobuf(new_$$field$$)) {
            return false;
        }
    }
"""

cc_encode_end="""   
    return true;
}
"""

cc_decode_begin="""
bool $$ns$$::$$class_name$$::FromProtobuf(const char* buf, size_t len) {
    Proto::$$ns$$::$$class_name$$ input;
    if (!input.ParseFromArray(buf, len)){
        return false;
    }

    return FromProtobuf(input);
}

bool $$ns$$::$$class_name$$::FromProtobuf(const Proto::$$ns$$::$$class_name$$& input)
{"""

cc_decode_struct_vec_field="""
    auto cur_$$field$$_size = static_cast<uint32_t>(input.$$field$$_size());
    if ( cur_$$field$$_size  > MAX_$$field_upper$$_SIZE ) {
        return false;
    }
    this->$$field$$_size = cur_$$field$$_size;
    for (uint32_t idx = 0; idx < cur_$$field$$_size; ++idx) {
        if ( !this->$$field$$[idx].FromProtobuf(input.$$field$$(idx))){
            return false;
        }
    }
"""

cc_decode_simple_vec_field="""
    auto cur_$$field$$_size = static_cast<uint32_t>(input.$$field$$_size());
    if ( cur_$$field$$_size  > MAX_$$field_upper$$_SIZE ) {
        return false;
    }
    this->$$field$$_size = cur_$$field$$_size;
    for (uint32_t idx = 0; idx < cur_$$field$$_size; ++idx) {
        this->$$field$$[idx] = input.$$field$$(idx);
    }
"""

cc_decode_simple_field="""
    this->$$field$$ = input.$$field$$();
"""

cc_decode_string_field="""
    this->set_$$field$$(input.$$field$$().c_str(), input.$$field$$().size());
"""

cc_decode_struct_field="""
    if ( !this->$$field$$.FromProtobuf( input.$$field$$() ) ) {
        return false;
    }
"""


code_decode_end="""
    return true;
}
"""


cc_opt_func="""
$$field_type$$* $$ns$$::$$class_name$$::Add$$field_first_up$$(){
    if ($$field$$_size >= MAX_$$field_upper$$_SIZE) {
        return nullptr;
    }
    $$field$$_size++;
    return &($$field$$[$$field$$_size-1]);
}

bool $$ns$$::$$class_name$$::Add$$field_first_up$$(const $$field_type$$& input) {
    if ($$field$$_size >= MAX_$$field_upper$$_SIZE) {
        return false;
    }
    $$field$$[$$field$$_size] = input;
    $$field$$_size++;
    return true;
}

$$field_type$$* $$ns$$::$$class_name$$::Find$$field_first_up$$ByIdx(uint32_t idx) {
    if (idx >= $$field$$_size) {
        return nullptr;
    }
    return &($$field$$[idx]);
}
    
bool $$ns$$::$$class_name$$::Del$$field_first_up$$ByIdx(const uint32_t idx) {
    if (idx >= $$field$$_size) {
        return false;
    }
    if (idx == $$field$$_size - 1) {
        $$field$$_size--;
        return true;
    }

    char* dest = reinterpret_cast<char*>(&$$field$$[idx]);
    const char* src = reinterpret_cast<char*>(&$$field$$[idx+1]);
    const char* end = reinterpret_cast<char*>(&$$field$$[$$field$$_size]);
    memcpy(dest, src, end - src);
    $$field$$_size--;
    return true;
}
"""

cc_additional_opt_func="""
$$field_type$$* $$ns$$::$$class_name$$::Find$$field_first_up$$ByKey($$field_key_type$$ $$field_key_name$$) {
    for (uint32_t idx = 0; idx < $$field$$_size; ++idx) {
        if ($$field$$[idx].$$field_key_name$$ == $$field_key_name$$) {
            return &($$field$$[idx]);
        }
    }
    return nullptr;
}

bool $$ns$$::$$class_name$$::Del$$field_first_up$$ByKey($$field_key_type$$ $$field_key_name$$) {
    for (uint32_t idx = 0; idx < $$field$$_size; ++idx) {
        if ($$field$$[idx].$$field_key_name$$ == $$field_key_name$$) {
            return Del$$field_first_up$$ByIdx(idx);
        }
    }
    return false;
}
"""                                                           



def gen_rawdata():
    return


def gen_datamgr():
    return

#file_pre.proto  file_pre.pb.h  
#file_pre_rawdata.h
#file_pre_rawdata.cc
#file_pre_datamgr.h
#file_pre_datamgr.cc

def prototype2ctype(field):

    if field.TYPE_MESSAGE == field.type:
        #print(field.type_name)
        field_name_vec = field.type_name.split('.')
        field_type = ""
        for name in field_name_vec:
            if ( name != "" and name != g_proto_ns ):
                if field_type != "" :
                    field_type = field_type + "::" + name
                else:
                    field_type = name
        return field_type
    elif field.TYPE_DOUBLE == field.type:
        return "double"
    elif field.TYPE_UINT64 == field.type:
        return "uint64_t"
    elif field.TYPE_INT64 == field.type:
        return "int64_t"
    elif field.TYPE_INT32 == field.type:
        return "int32_t"        
    elif field.TYPE_BOOL == field.type:
        return "bool"
    elif field.TYPE_STRING == field.type:
        return "char"
    elif field.TYPE_UINT32 == field.type:
        return "uint32_t"

    return ""
    # TYPE_FLOAT TYPE_FIXED64 TYPE_FIXED32 
    # TYPE_GROUP 
    # TYPE_BYTES 
    # TYPE_ENUM TYPE_SFIXED32 TYPE_SFIXED64 TYPE_SINT32 TYPE_SINT64


KEYWORD_MSG = "message"
VAR_NAME_RE = "^[A-Za-z_][A-Za-z0-9_]*"

def read_proto(file):
    file_lines=[]
    struct_dict={}
    parse_instruct=[]

    line = file.readline()
    line_no = 0
    # read from file
    last_msg_name=""
    parse_begin = False
    parse_end = False

    while line:
        # loop 
        line_no = line_no + 1

        #get gen instruct
        if (not parse_end):
            tmp = line.lstrip()
            if (not parse_begin):
                if ( 0 == tmp.find('//gen_begin') ):
                    parse_begin = True
                    parse_instruct.append(tmp)
            else:
                if ( 0 == tmp.find('//gen_end') ):
                    parse_end = True
                else:
                    parse_instruct.append(tmp)

        #get struct dict & lines
        idx = line.find(KEYWORD_MSG)
        if ( -1 != idx):
            subline = line[idx + len(KEYWORD_MSG):]
            if ( -1 != subline.find(KEYWORD_MSG) ):
                print "line has two message def", line_no
                return None
            
            msg_name = re.search(VAR_NAME_RE, subline.lstrip())
            if (not msg_name):
                print "message keyword and name should be in one line", line_no
                return None

            msg_name = msg_name.group()
            struct_dict[msg_name]=[]
            struct_dict[msg_name].append(line_no)
            if ("" != last_msg_name):
                struct_dict[last_msg_name].append(line_no - 1)
            last_msg_name = msg_name

        file_lines.append(line)

        # loop 
        line = file.readline()

    struct_dict[last_msg_name].append(line_no)

    return file_lines, struct_dict, parse_instruct

g_gen_keyword = "|gen|max|"
g_gen_keyopt = "keyopt"


def find_field_opt(class_name, field_name, file_lines, struct_dict):
    if not struct_dict[class_name]:
        print("no in struct dict")
        return

    startline = struct_dict[class_name][0]
    endline = struct_dict[class_name][1]
    for line_no in range(startline-1, endline-1) :
        #print(line_no, len(file_lines))

        if not file_lines[line_no]: 
            print("no in file lines")
            continue

        print(file_lines[line_no], field_name, g_gen_keyword )

        if -1 == file_lines[line_no].find(field_name):
            continue;

        idx = file_lines[line_no].find(g_gen_keyword)
        if -1 == idx:
            #print(file_lines[line_no], field_name, g_gen_keyword )
            return

        gen_instruct = file_lines[line_no][idx+len(g_gen_keyword):]

        gen_instruct = gen_instruct.split("|")

        print(gen_instruct)
        if gen_instruct[1] == g_gen_keyopt:
            return gen_instruct[0], True
        else:
            return gen_instruct[0], False #max #additional 

g_all_proto = []

def find_key_info(full_name):
    print(full_name)
    full_name = full_name.split('::')
    full_name = full_name[ len(full_name) - 1 ]

    print(full_name)
    for proto_desc in g_all_proto:
        for msg_type in proto_desc.message_type:
            if msg_type.name == full_name:
                return msg_type.field[0]

def gen_code(file_pre, file_lines, struct_dict, parse_instruct, proto_desc):
    ###########################################################
    # 处理生成指令
    """
    file_pre
    ns
    macro
    class_name
    field
    field_type
    field_upper
    field_first_up
    field_key_name
    """
    header_file_name = file_pre+"_rawdata.h"
    cpp_file_name = file_pre+"_rawdata.cpp"

    header_file = open(header_file_name, 'w')
    cpp_file = open(cpp_file_name, 'w')

    if (len(parse_instruct) < 2):
        print("parse instruct err ")
        return
    #print(class_header_template)
    
    tmp = proto_desc.package.split('.')
    if (len(tmp)<2):
        print("proto should be in namespace")
        return
    if g_proto_ns != tmp[0]:
        print("proto should be in namespace" + g_proto_ns)
        return
    namespace = tmp[1]

    #

    
    macro = namespace.upper() + "_" + file_pre.upper() + "_RAWDATA_H_"

    h_content = ""
    cc_content = cc_begin.replace("$$file_pre$$", file_pre)    
    h_content = h_content + h_begin.replace("$$macro$$", macro).replace("$$file_pre$$", file_pre)    
    for inc in proto_desc.dependency:
        inc_pre = inc.split('.')
        inc_pre = inc_pre[0]
        h_content = h_content + h_include.replace("$$file_pre$$", inc_pre)
    h_content = h_content +  h_ns_begin.replace("$$ns$$", namespace);
    ###########################################################
    # 生成代码
    #header
        #declare
          #field dec
          #field_opt func
        #encode func 
          #field encode
        #decode func
          #field decode
    for msg_type in proto_desc.message_type:
        h_declare_content = ""
        cc_encode_content = ""
        cc_decode_content = ""
        cc_opt_content = ""

        #### class/encode/decode begin part
        class_name = msg_type.name
        h_declare_content = h_declare_content + h_class_begin.replace("$$class_name$$", class_name)
        cc_encode_content = cc_encode_content + cc_encode_begin.replace("$$ns$$", namespace).replace("$$class_name$$", class_name)
        cc_decode_content = cc_decode_content + cc_decode_begin.replace("$$ns$$", namespace).replace("$$class_name$$", class_name)
        print("gen msg: ", class_name)
        
        #### field opt in class/encode/decode
        for field in msg_type.field:
            #print(field)
            field_type = prototype2ctype(field)
            if ( "" == field_type):
                print("get field type error")
                return
            field_name = field.name
            field_upper = field_name.upper()
            field_first_up = first2upper(field_name)
            field_key_name = ""
            field_max = "1"

            if (field.label == field.LABEL_REPEATED):
                field_max, gen_keyopt = find_field_opt(class_name, field_name, file_lines, struct_dict)
                if (field.type == field.TYPE_MESSAGE):
                    cc_encode_content = cc_encode_content + cc_encode_struct_vec_field.replace("$$field$$", field_name)
                    cc_decode_content = cc_decode_content + cc_decode_struct_vec_field.replace("$$field$$", field_name).replace("$$field_upper$$", field_upper)

                elif (field.type == field.TYPE_STRING):
                    print("unsurpport string array")
                    return
                else:
                    cc_encode_content = cc_encode_content + cc_encode_simple_vec_field.replace("$$field$$", field_name)
                    cc_decode_content = cc_decode_content + cc_decode_simple_vec_field.replace("$$field$$", field_name).replace("$$field_upper$$", field_upper)

                h_declare_content = h_declare_content +\
                    h_vec_field.replace("$$field_type$$",field_type).\
                    replace("$$field$$", field_name).\
                    replace("$$field_upper$$", field_upper).\
                    replace("$$field_first_up$$", field_first_up).\
                    replace("$$max$$", field_max)

                cc_opt_content = cc_opt_content +\
                    cc_opt_func.replace("$$field_type$$", field_type).\
                    replace("$$field$$", field_name).\
                    replace("$$ns$$", namespace).\
                    replace("$$class_name$$", class_name).\
                    replace("$$field_first_up$$", field_first_up).\
                    replace("$$field_upper$$", field_upper)


                if gen_keyopt:
                    key_field_info = find_key_info(field_type)
                    field_key_type = prototype2ctype(key_field_info)
                    field_key_name = key_field_info.name
                    h_declare_content = h_declare_content +\
                        h_vec_field_more.replace("$$field_type$$",field_type).\
                        replace("$$field_first_up$$", field_first_up).\
                        replace("$$field_key_type$$", field_key_type).\
                        replace("$$field_key_name$$", field_key_name)

                    cc_opt_content = cc_opt_content +\
                        cc_additional_opt_func.replace("$$field_type$$", field_type).\
                        replace("$$field$$", field_name).\
                        replace("$$ns$$", namespace).\
                        replace("$$class_name$$", class_name).\
                        replace("$$field_first_up$$", field_first_up).\
                        replace("$$field_upper$$", field_upper).\
                        replace("$$field_key_type$$", field_key_type).\
                        replace("$$field_key_name$$", field_key_name)    
            else:
                if (field.type == field.TYPE_MESSAGE):
                    h_declare_content = h_declare_content + h_single_field.replace("$$field_type$$", field_type).replace("$$field$$", field_name)
                    cc_encode_content = cc_encode_content + cc_encode_struct_field.replace("$$field$$", field_name)
                    cc_decode_content = cc_decode_content + cc_decode_struct_field.replace("$$field$$", field_name)

                elif (field.type == field.TYPE_STRING):
                    field_max, gen_keyopt = find_field_opt(class_name, field_name, file_lines, struct_dict)
                    h_declare_content = h_declare_content +\
                        h_string_field.replace("$$field_upper$$", field_upper).\
                        replace("$$field$$", field_name).\
                        replace("$$max$$", field_max)
                    cc_encode_content = cc_encode_content + cc_encode_string_field.replace("$$field$$", field_name)
                    cc_decode_content = cc_decode_content + cc_decode_string_field.replace("$$field$$", field_name)
                else:
                    h_declare_content = h_declare_content + h_single_field.replace("$$field_type$$", field_type).replace("$$field$$", field_name)
                    cc_encode_content = cc_encode_content + cc_encode_simple_field.replace("$$field$$", field_name)
                    cc_decode_content = cc_decode_content + cc_decode_simple_field.replace("$$field$$", field_name)

        ### class/encode/decode end part
        h_declare_content = h_declare_content + h_class_end.replace("$$class_name$$", class_name).replace("$$ns$$", namespace)
        cc_encode_content = cc_encode_content + cc_encode_end
        cc_decode_content = cc_decode_content + code_decode_end

        h_content = h_content + h_declare_content
        cc_content = cc_content + cc_opt_content
        cc_content = cc_content + cc_encode_content
        cc_content = cc_content + cc_decode_content

    h_content = h_content + h_end.replace("$$ns$$", namespace).replace("$$macro$$", macro)
    header_file.write(h_content)
    cpp_file.write(cc_content)
    return

def get_protodesc(file_name, file_pre):

    ############################################
    if iswin():
        commond = "protoc.exe --python_out=. " + " ./" + file_name
    else:
        commond = "./protoc --python_out=. " + " ./" + file_name
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


def handle_proto(file_pre):
    file_name = file_pre + ".proto"

    ############################################
    try:
        file = open(file_name)
    except Exception as err:
        print err
        return
    file_lines, struct_dict, parse_instruct = read_proto(file)
    file.close()

    if (not file_lines):
        return
    
    proto_desc = get_protodesc(file_name, file_pre)
    g_all_proto.append(proto_desc)

    for inc in proto_desc.dependency:
        inc_pre = inc.split('.')
        inc_pre = inc_pre[0]
        depend_proto = get_protodesc(inc, inc_pre)
        g_all_proto.append(depend_proto)

    ############################################
    gen_code(file_pre, file_lines, struct_dict, parse_instruct, proto_desc)


def main():
    file_name=raw_input("input proto file name: ")
    handle_proto(file_name)
    file_name=raw_input("press enter to quit")

main()
