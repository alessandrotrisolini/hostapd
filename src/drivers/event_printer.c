#include "utils/common.h"
#include "common/wpa_ctrl.h"
#include "utils/eloop.h"

#define CONFIG_CTRL_IFACE_DIR "/var/run/wpa_supplicant"

static struct wpa_ctrl *ctrl_conn;
static int wpa_cli_attached = 0;
static void wpa_cli_receive(int sock, void* eloop_ctx, void *sock_ctx);

static void wpa_cli_close_connection(void)
{
        if (ctrl_conn == NULL)
                return;

        if (wpa_cli_attached) {
                wpa_ctrl_detach(ctrl_conn);
                wpa_cli_attached = 0;
		eloop_unregister_read_sock(wpa_ctrl_get_fd(ctrl_conn));
        }

        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
}

static void wpa_cli_receive(int sock, void* eloop_ctx, void *sock_ctx){
	wpa_cli_recv_pending(ctrl_conn, 0);
}

static void wpa_cli_recv_pending(struct wpa_ctrl *ctrl, int action_monitor)
{
        while (wpa_ctrl_pending(ctrl) > 0) {
                char buf[4096];
                size_t len = sizeof(buf) - 1;
                if (wpa_ctrl_recv(ctrl, buf, &len) == 0) {
                        buf[len] = '\0';
                        if (action_monitor)
                                wpa_cli_action_process(buf);
                        else {
                                cli_event(buf);
                                if (wpa_cli_show_event(buf)) {
                                        edit_clear_line();
                                        printf("\r%s\n", buf);
                                        edit_redraw();
                                }
                        }
                } else {
                        printf("Could not read pending message.\n");
                        break;
                }
        }

        if (wpa_ctrl_pending(ctrl) < 0) {
                printf("Connection to wpa_supplicant lost - trying to "
                       "reconnect\n");
                wpa_cli_reconnect();
        }
}


int main(){

	const char *global = NULL;

	if(eloop_init()){
		printf("eloop init returned 'true'\n");
		return -1;
	}

	ctrl_conn = wpa_ctrl_open(global);

	if(ctrl_conn == NULL){
		printf("wpa_ctrl_open failed\n");
		return -1;
	}

	if(wpa_ctrl_attach(ctrl_conn) == -1){
		printf("wpa_ctrl_attach failed\n");
		return -1;
	}
	wpa_cli_attached = 1;	

	eloop_register_read_sock(
		wpa_ctrl_get_fd(ctrl_conn), 
		wpa_cli_receive, 
		NULL, NULL);

	wpa_cli_close_connection();
	eloop_destroy();

	return 0;
	
}
