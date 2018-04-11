/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * pb_config.h
 *
 *  Created on: 2016年5月9日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CONFIG_PB_CONFIG_H_
#define YARF_CONFIG_PB_CONFIG_H_

#include <google/protobuf/message.h>

namespace yarf {
namespace config {

class CPbConfig {
 public:
  CPbConfig() {
  }

  static int LoadConf(const char* path, google::protobuf::Message *conf);
  static void Output(const google::protobuf::Message *conf);

 private:
};

}  // namespace config
}  // namespace yarf

#endif  // YARF_CONFIG_PB_CONFIG_H_

