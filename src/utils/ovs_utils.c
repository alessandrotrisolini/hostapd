#include <sys/wait.h>
#include "utils/includes.h"
#include "utils/common.h"
#include "utils/ovs_utils.h"

/*
 *	Macro for executing OvS commands
 */

#define run_ovs_cmd(p, ...) ({ \
	int rc = -1, status; \
	pid_t pid = fork(); \
	if (pid == 0) \
		exit(execl(p, p, ##__VA_ARGS__, NULL)); \
	if (pid > 0) { \
		while((rc = waitpid(pid, &status, 0)) == -1 && errno == EINTR); \
			rc = (rc == pid && WIFEXITED(status)) ? WEXITSTATUS(status) : -1; \
	} \
	rc; \
})


/*
 *	Add port to an OvS switch
 *
 *	# const char* ovs_br_name : name of the OvS switch
 *	# const char* ovs_port_name : name of the port to be added
 *
 */

int ovs_add_port(const char* ovs_br_name, const char* ovs_port_name) {

	if (run_ovs_cmd("/usr/bin/ovs-vsctl", "add-port", ovs_br_name, ovs_port_name))
		return -1;
	return 0;

}

/*
 *	Delete port to an OvS switch
 *
 *	# const char* ovs_br_name : name of the OvS switch
 *	# const char* ovs_port_name : name of the port to be deleted
 *
 */

int ovs_del_port(const char* ovs_br_name, const char* ovs_port_name) {

	if (run_ovs_cmd("/usr/bin/ovs-vsctl", "del-port", ovs_br_name, ovs_port_name))
		return -1;
	return 0;

}
