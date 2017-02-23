#ifndef OVS_UTILS
#define OVS_UTILS

int ovs_add_port(const char* ovs_br_name, const char* ovs_port_name);
int ovs_del_port(const char* ovs_br_name, const char* ovs_port_name);

#endif /* OVS_UTILS */