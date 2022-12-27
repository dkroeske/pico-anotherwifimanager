#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "dhcpserver.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/apps/httpd.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

struct ap_t {
	char ssid[32];
	int16_t rssi;
	uint8_t auth_mode;
};

struct aps_t {
	struct ap_t ap[10];
	uint8_t index;
};

struct aps_t aps = {0};


const char *ap_name = "PicoPixelz config";
const char *password = NULL;
const char *ssi_example_tags[] = {
	"ssid_0",
	"ssid_1",
	"ssid_2",
	"ssid_3",
	"ssid_4",
	"ssid_5",
	"ssid_6",
	"ssid_7",
	"ssid_8",
	"ssid_9",
};

const char *CGIFormHandler(int index, int numParams, char *param[], char *value[]);

const tCGI FORM_CGI = {"/form.cgi", CGIFormHandler};

const char *CGIFormHandler(int index, int numParams, char *param[], char *value[])
{
	switch (index) {
		case 0: {
			
			for(uint8_t idx = 0; idx < numParams; idx++){
				printf("param %s: value %s\n", param[idx], value[idx]);
			}
		}
		break;

		default: {
			printf("CGIForm index: %d Huh?!\n", index);
		}
	}

	return "/thx.html";
}

static uint16_t ssi_handler(int index, char *insert, int len) {

	printf("SSI index %d\n", index);

	switch( index ) {
		case 0:
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			snprintf(insert, len, "%s", aps.ap[index].ssid);
			break;
		case 3:
			snprintf(insert, len, "%s", "");
			break;
	}

	return (uint16_t) strlen(insert);
}

static void ssi_init() {
	size_t idx;
	for(idx = 0; idx < LWIP_ARRAYSIZE(ssi_example_tags); idx++) {
		LWIP_ASSERT("LWIP_HTTPD_MAX_TAG_NAME_LEN to short",
		strlen(ssi_example_tags[idx]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
	}
	
	http_set_ssi_handler(ssi_handler, ssi_example_tags, LWIP_ARRAYSIZE(ssi_example_tags));
}

static void cgi_init() {
	http_set_cgi_handlers(&FORM_CGI, 1);
}

void debug_aps(void) {
	for(uint8_t idx = 0; idx < aps.index; idx++) {
		printf("ssid: %-32s rssi: %4d, %d\n", aps.ap[idx].ssid, aps.ap[idx].rssi, aps.ap[idx].auth_mode);
	}
}

int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {
		
		if( aps.index < 10 ) {
			// Do not add empty ssid's
			if(strlen((const char *)result->ssid) != 0) {
				// Check if ssid is already in the list
				bool exists = false;
				for(uint8_t idx = 0; idx < aps.index && !exists; idx++ ) {
					if( !strcmp(aps.ap[idx].ssid, (const char *) result -> ssid) ) {
						exists = true;
					}
				}
				if( !exists ) {
					strcpy(aps.ap[aps.index].ssid, (const char *)result->ssid);
					aps.ap[aps.index].rssi = result->rssi;
					aps.ap[aps.index].auth_mode = result->auth_mode;
			
					aps.index++;
				}
			}
		}							

//        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
//            result->ssid, result->rssi, result->channel,
//            result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
//            result->auth_mode);
    }
    return 0;
}

#ifdef PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS
#undef PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS
#define PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 1
#endif

int main(){

	stdio_usb_init();
	sleep_ms(2000);
//	stdio_init_all();
	printf("*\n");

	if( cyw43_arch_init()) {
		printf("cyw43_arch_init() failed!\n");
	}

	cyw43_arch_enable_sta_mode();

	cyw43_wifi_scan_options_t scan_options = {0};
	aps.index = 0;
   int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
	while(cyw43_wifi_scan_active(&cyw43_state)) ;
   if (err == 0) {
   	printf("\nPerforming wifi scan\n");
   } else {
		printf("Failed to start scan: %d\n", err);
   }
	
	debug_aps();

	
//	cyw43_arch_deinit();
//	sleep_ms(2000);
	

/*	absolute_time_t scan_test = nil_time;
	bool scan_in_progress = false;
   while(true) {
        if (absolute_time_diff_us(get_absolute_time(), scan_test) < 0) {
            if (!scan_in_progress) {
                cyw43_wifi_scan_options_t scan_options = {0};
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
                if (err == 0) {
                    printf("\nPerforming wifi scan\n");
                    scan_in_progress = true;
                } else {
                    printf("Failed to start scan: %d\n", err);
                    scan_test = make_timeout_time_ms(10000); // wait 10s and scan again
                }
            } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
                scan_test = make_timeout_time_ms(10000); // wait 10s and scan again 
                scan_in_progress = false; 
            }
        }
}
*/


//	if( cyw43_arch_init()) {
//		printf("cyw43_arch_init() failed!\n");
//	}
	
	cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
	
	ip4_addr_t gw, mask;
	IP4_ADDR(&gw, 192,168,4,1);
	IP4_ADDR(&mask, 255, 255, 255, 0);

	dhcp_server_t dhcp_server;
	dhcp_server_init(&dhcp_server, &gw, &mask);

	// ssi init
	ssi_init();
	cgi_init();
	
	// Start webserver
	httpd_init();

	while(true) {
		printf("Elapsed time: %lld [ms]\n", time_us_64()/1000);
		sleep_ms(2500);
	}


}
