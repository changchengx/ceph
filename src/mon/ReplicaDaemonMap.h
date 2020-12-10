// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * Copyright(c) 2020, Intel Corporation
 *
 * Author: Changcheng Liu <changcheng.liu@aliyun.com>
 */

#ifndef CEPH_REPLICADAEMONMAP_H
#define CEPH_REPLICADAEMONMAP_H

#include "include/types.h"
#include "msg/msg_types.h"

enum ReplicaDaemonStatus : uint32_t {
  STATE_BOOTING,
  STATE_ACTIVE,
  STATE_STOPPING,
  STATE_DOWN
};

struct ReplicaDaemonState {
  version_t commit_epoch = 0;
  ReplicaDaemonStatus daemon_status = STATE_BOOTING;
  entity_addrvec_t replica_route_addr;

  void encode(bufferlist& replicadaemon_state_bl, uint64_t features = 0) const {
    ENCODE_START(0, 0, replicadaemon_state_bl);
    encode(commit_epoch, replicadaemon_state_bl);
    encode(daemon_status, replicadaemon_state_bl);
    encode(replica_route_addr, replicadaemon_state_bl, features);
    ENCODE_FINISH(replicadaemon_state_bl);
  }

  void decode(bufferlist::const_iterator& replicadaemon_state_bl_it) {
    uint32_t daemon_status = 0;
    DECODE_START(0, replicadaemon_state_bl_it);
    decode(commit_epoch, replicadaemon_state_bl_it);
    decode(daemon_status, replicadaemon_state_bl_it);
    decode(replica_route_addr, replicadaemon_state_bl_it);
    DECODE_FINISH(replicadaemon_state_bl_it);
    this->daemon_status = static_cast<ReplicaDaemonStatus>(daemon_status);
  }

  void print_state(std::ostream& oss) const;
};
WRITE_CLASS_ENCODER(ReplicaDaemonState)

class ReplicaDaemonMap {
public:
  ReplicaDaemonMap();

  void encode(bufferlist& replicadaemon_map_bl, uint64_t features) const;
  void decode(bufferlist::const_iterator& replicadaemon_map_bl_it);
  void decode(bufferlist& replicadaemon_map_bl) {
    auto bl_it = replicadaemon_map_bl.cbegin();
    decode(bl_it);
  }

  epoch_t get_epoch() const {
    return epoch;
  }
  void set_epoch(epoch_t epoch) {
    this->epoch = epoch;
  }
  std::vector<ReplicaDaemonState>& get_replicadaemons_stateref(void) {
    return replicadaemons_state;
  }
  void print_map(std::ostream& oss) const;

  bool empty() {
    return replicadaemons_state.empty();
  }

private:
  epoch_t epoch = 0;
  std::vector<ReplicaDaemonState> replicadaemons_state;
};
WRITE_CLASS_ENCODER_FEATURES(ReplicaDaemonMap)

#endif // defined CEPH_REPLICADAEMONMAP_H
