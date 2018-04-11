/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * pb_config.cc
 *
 *  Created on: 2016年5月9日
 *      Author: YAOZHONGCUN
 */

#include <fcntl.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/json_util.h>

#include "yarf/config/pb_config.h"
#include "yarf/err.h"
#include "yarf/log.h"

using yarf::config::CPbConfig;

int CPbConfig::LoadConf(const char* path, google::protobuf::Message *conf) {
  if (nullptr == path || nullptr == conf) {
    return yarf::err::kArg;
  }
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return yarf::err::kFileIo;
  }

  google::protobuf::io::FileInputStream conf_file(fd);
  conf_file.SetCloseOnDelete(true);

  if (!google::protobuf::TextFormat::Parse(&conf_file, conf)) {
    return yarf::err::kFileIo;
  }
  return yarf::kOk;
}

void CPbConfig::Output(const google::protobuf::Message *conf) {
  if (nullptr == conf) {
    return;
  }

  google::protobuf::util::JsonPrintOptions json_opt;
  json_opt.add_whitespace = true;
  json_opt.always_print_primitive_fields = true;

  std::string output;
  google::protobuf::util::MessageToJsonString(*conf, &output, json_opt);

  DEBUG_LOG("msg: %s", output.c_str());
}

