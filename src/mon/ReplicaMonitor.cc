// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * Copyright(c) 2020, Intel Corporation
 *
 * Author: Changcheng Liu <changcheng.liu@aliyun.com>
 */

#include "ReplicaMonitor.h"
#include "messages/MReplicaDaemonMap.h"

#define FN_NAME (__CEPH_ASSERT_FUNCTION == nullptr ? __func__ : __CEPH_ASSERT_FUNCTION)
#define dout_context g_ceph_context
#define dout_subsys ceph_subsys_cache_replica
#undef dout_prefix
#define dout_prefix _prefix(_dout, FN_NAME, mon, this)
static ostream& _prefix(std::ostream *_dout,
                        std::string_view func_name,
                        const Monitor *mon,
                        const ReplicaMonitor *replica_monitor) {
  return *_dout << func_name << ": " << "mon." << mon->name << "@" << mon->rank
                << "(" << mon->get_state_name() << ").ReplicaMonitor: ";
}

ReplicaMonitor::ReplicaMonitor(Monitor* monitor, Paxos *paxos, std::string service_name)
  : PaxosService(monitor, paxos, service_name)
{
}

void ReplicaMonitor::init()
{
  dout(10) << dendl;
}

void ReplicaMonitor::create_initial()
{
  cur_cache_replicadaemon_map.set_epoch(0);
  pending_cache_replicadaemon_map = cur_cache_replicadaemon_map;
  pending_cache_replicadaemon_map.set_epoch(cur_cache_replicadaemon_map.get_epoch() + 1);
}

void ReplicaMonitor::update_from_paxos(bool *need_bootstrap)
{
// TODO: Must implement pure virtual function
}

void ReplicaMonitor::create_pending()
{
  pending_cache_replicadaemon_map = cur_cache_replicadaemon_map;
  pending_cache_replicadaemon_map.set_epoch(cur_cache_replicadaemon_map.get_epoch() + 1);
  dout(10) << "cur_cache epoch: " << cur_cache_replicadaemon_map.get_epoch() << ", "
           << "pening_cache epoch: " << pending_cache_replicadaemon_map.get_epoch()
           << dendl;
}

void ReplicaMonitor::encode_pending(MonitorDBStore::TransactionRef mon_dbstore_tran)
{
  version_t version = get_last_committed() + 1;

#if 0
  if (pending_cache_replicadaemon_map.get_epoch() != version) {
    derr << "should not update the db, always continue here, need refine" << dendl;
  }
  ceph_assert(version == pending_cache_replicadaemon_map.get_epoch());
#endif

  // apply to paxos
  bufferlist pending_bl;
  pending_cache_replicadaemon_map.encode(pending_bl, mon->get_quorum_con_features());

  // put everything in the transaction
  put_version(mon_dbstore_tran, version, pending_bl);
  put_last_committed(mon_dbstore_tran, version);
}

void ReplicaMonitor::encode_full(MonitorDBStore::TransactionRef mon_dbstore_tran)
{
// Empty function
}

bool ReplicaMonitor::preprocess_query(MonOpRequestRef mon_op_req)
{
// TODO: Must implement pure virtual function
  return false;
}

bool ReplicaMonitor::prepare_update(MonOpRequestRef mon_op_req)
{
// TODO: Must implement pure virtual function
  return false;
}

void ReplicaMonitor::on_restart()
{
// TODO: Clear the pending map
}

void ReplicaMonitor::check_sub(Subscription *sub)
{
  // Only support subscribe "replicamap"
  ceph_assert(sub->type == "replicamap");

  ReplicaDaemonMap *replicadaemon_map = nullptr;
  ReplicaDaemonMap reply_map;
  //TODO build reply_map according to cur_cache_replicadaemon_map;
  replicadaemon_map = &cur_cache_replicadaemon_map;

  //reply subscription
  auto reply_msg = make_message<MReplicaDaemonMap>(*replicadaemon_map);
  sub->session->con->send_message(reply_msg.detach());
  if (sub->onetime) {
    mon->session_map.remove_sub(sub);
  } else {
    sub->next = cur_cache_replicadaemon_map.get_epoch() + 1;
  }
}

void ReplicaMonitor::decode_replicadaemon_map(bufferlist &replicadaemon_map_bl)
{
  cur_cache_replicadaemon_map.decode(replicadaemon_map_bl);
}

template<int dbg_level>
void ReplicaMonitor::print_map(const ReplicaDaemonMap& replicadaemon_map) const
{
  dout(dbg_level) << "print map\n";
  replicadaemon_map.print_map(*_dout);
  *_dout << dendl;
}
