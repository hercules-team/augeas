(*
Module: Test_Keepalived
  Provides unit tests and examples for the <Keepalived> lens.
*)

module Test_Keepalived =

(* Variable: conf
   A full configuration file *)
   let conf = "! This is a comment 
! Configuration File for keepalived 

global_defs { 
   ! this is who emails will go to on alerts 
   notification_email { 
        admins@example.com 
    fakepager@example.com 
    ! add a few more email addresses here if you would like 
   } 
   notification_email_from admins@example.com 

   smtp_server 127.0.0.1  ! I use the local machine to relay mail 
   smtp_connect_timeout 30 

   ! each load balancer should have a different ID 
   ! this will be used in SMTP alerts, so you should make 
   ! each router easily identifiable 
   lvs_id LVS_EXAMPLE_01 
} 

vrrp_sync_group VG1 { 
   group { 
      inside_network  # name of vrrp_instance (below)
      outside_network # One for each moveable IP.  
   } 
} 

vrrp_instance VI_1 { 
        state MASTER 
        interface eth0 

	track_interface {
		eth0 # Back
		eth1 # DMZ
	}
	track_script {
		check_apache2    # weight = +2 si ok, 0 si nok
	}
	garp_master_delay 5
	priority 50
	advert_int 2
	authentication {
		auth_type PASS
		auth_pass mypass
	}
	virtual_ipaddress {
		10.234.66.146/32 dev eth0
	}
     
        lvs_sync_daemon_interface eth0 
	ha_suspend

       notify_master   \"/svr/scripts/notify_master.sh\"
       notify_backup   \"/svr/scripts/notify_backup.sh\"
       notify_fault    \"/svr/scripts/notify_fault.sh\"

    ! each virtual router id must be unique per instance name! 
        virtual_router_id 51 

    ! MASTER and BACKUP state are determined by the priority 
    ! even if you specify MASTER as the state, the state will 
    ! be voted on by priority (so if your state is MASTER but your 
    ! priority is lower than the router with BACKUP, you will lose 
    ! the MASTER state) 
    ! I make it a habit to set priorities at least 50 points apart 
    ! note that a lower number is lesser priority - lower gets less vote 
        priority 150 

    ! how often should we vote, in seconds? 
        advert_int 1 

    ! send an alert when this instance changes state from MASTER to BACKUP 
        smtp_alert 

    ! this authentication is for syncing between failover servers 
    ! keepalived supports PASS, which is simple password 
    ! authentication 
    ! or AH, which is the IPSec authentication header. 
    ! I don't use AH 
    ! yet as many people have reported problems with it 
        authentication { 
                auth_type PASS 
                auth_pass example 
        } 

    ! these are the IP addresses that keepalived will setup on this 
    ! machine. Later in the config we will specify which real 
        ! servers  are behind these IPs 
    ! without this block, keepalived will not setup and takedown the 
    ! any IP addresses 
     
        virtual_ipaddress { 
                192.168.1.11 
                10.234.66.146/32 dev vlan933 # parse it well
        ! and more if you want them 
        } 
} 

virtual_server 192.168.1.11 22 { 
    delay_loop 6 

    ! use round-robin as a load balancing algorithm 
    lb_algo rr 

    ! we are doing NAT 
    lb_kind NAT 
    nat_mask 255.255.255.0 

    protocol TCP 

    ! there can be as many real_server blocks as you need 

    real_server 10.20.40.10 22 { 

    ! if we used weighted round-robin or a similar lb algo, 
    ! we include the weight of this server 

        weight 1 

    ! here is a health checker for this server. 
    ! we could use a custom script here (see the keepalived docs) 
    ! but we will just make sure we can do a vanilla tcp connect() 
    ! on port 22 
    ! if it fails, we will pull this realserver out of the pool 
    ! and send email about the removal 
        TCP_CHECK { 
                connect_timeout 3 
        connect_port 22 
        } 
    } 
} 

virtual_server_group DNS_1 {
    192.168.0.1 22
    10.234.55.22-25 36
    10.45.58.59/32 27
}

vrrp_script chk_apache2 {       # Requires keepalived-1.1.13
script \"killall -0 apache2\"   # faster
interval 2                      # check every 2 seconds
weight 2                        # add 2 points of prio if OK
}

! that's all
"


(* Test: Keepalived.lns
   Test the full <conf> *)
   test Keepalived.lns get conf =
     { "#comment" = "This is a comment" }
     { "#comment" = "Configuration File for keepalived" }
     {}
     { "global_defs"
       { "#comment" = "this is who emails will go to on alerts" }
       { "notification_email"
            { "email" = "admins@example.com" }
            { "email" = "fakepager@example.com" }
            { "#comment" = "add a few more email addresses here if you would like" } }
       { "notification_email_from" = "admins@example.com" }
       { }
       { "smtp_server" = "127.0.0.1"
         { "#comment" = "I use the local machine to relay mail" } }
       { "smtp_connect_timeout" = "30" }
       {}
       { "#comment" = "each load balancer should have a different ID" }
       { "#comment" = "this will be used in SMTP alerts, so you should make" }
       { "#comment" = "each router easily identifiable" }
       { "lvs_id" = "LVS_EXAMPLE_01" } }
     {}
     { "vrrp_sync_group" = "VG1"
       { "group"
         { "inside_network"
           { "#comment" = "name of vrrp_instance (below)" } }
         { "outside_network"
           { "#comment" = "One for each moveable IP." } } } }
     {}
     { "vrrp_instance" = "VI_1"
       { "state" = "MASTER" }
       { "interface" = "eth0" }
       { }
       { "track_interface"
         { "eth0" { "#comment" = "Back" } }
         { "eth1" { "#comment" = "DMZ" } } }
       { "track_script"
         { "check_apache2" { "#comment" = "weight = +2 si ok, 0 si nok" } } }
       { "garp_master_delay" = "5" }
       { "priority" = "50" }
       { "advert_int" = "2" }
       { "authentication"
         { "auth_type" = "PASS" }
         { "auth_pass" = "mypass" } }
       { "virtual_ipaddress"
         { "ipaddr" = "10.234.66.146"
           { "prefixlen" = "32" }
           { "dev" = "eth0" } } }
       { }
       { "lvs_sync_daemon_interface" = "eth0" }
       { "ha_suspend" }
       { }
       { "notify_master" = "\"/svr/scripts/notify_master.sh\"" }
       { "notify_backup" = "\"/svr/scripts/notify_backup.sh\"" }
       { "notify_fault" = "\"/svr/scripts/notify_fault.sh\"" }
       { }
       { "#comment" = "each virtual router id must be unique per instance name!" }
       { "virtual_router_id" = "51" }
       { }
       { "#comment" = "MASTER and BACKUP state are determined by the priority" }
       { "#comment" = "even if you specify MASTER as the state, the state will" }
       { "#comment" = "be voted on by priority (so if your state is MASTER but your" }
       { "#comment" = "priority is lower than the router with BACKUP, you will lose" }
       { "#comment" = "the MASTER state)" }
       { "#comment" = "I make it a habit to set priorities at least 50 points apart" }
       { "#comment" = "note that a lower number is lesser priority - lower gets less vote" }
       { "priority" = "150" }
       { }
       { "#comment" = "how often should we vote, in seconds?" }
       { "advert_int" = "1" }
       { }
       { "#comment" = "send an alert when this instance changes state from MASTER to BACKUP" }
       { "smtp_alert" }
       { }
       { "#comment" = "this authentication is for syncing between failover servers" }
       { "#comment" = "keepalived supports PASS, which is simple password" }
       { "#comment" = "authentication" }
       { "#comment" = "or AH, which is the IPSec authentication header." }
       { "#comment" = "I don't use AH" }
       { "#comment" = "yet as many people have reported problems with it" }
       { "authentication"
         { "auth_type" = "PASS" }
         { "auth_pass" = "example" } }
       { }
       { "#comment" = "these are the IP addresses that keepalived will setup on this" }
       { "#comment" = "machine. Later in the config we will specify which real" }
       { "#comment" = "servers  are behind these IPs" }
       { "#comment" = "without this block, keepalived will not setup and takedown the" }
       { "#comment" = "any IP addresses" }
       { }
       { "virtual_ipaddress"
         { "ipaddr" = "192.168.1.11" }
         { "ipaddr" = "10.234.66.146"
           { "prefixlen" = "32" }
           { "dev" = "vlan933" }
           { "#comment" = "parse it well" } }
         { "#comment" = "and more if you want them" } } }
       { }
       { "virtual_server"
         { "ip" = "192.168.1.11" }
         { "port" = "22" }
         { "delay_loop" = "6" }
         { }
         { "#comment" = "use round-robin as a load balancing algorithm" }
         { "lb_algo" = "rr" }
         { }
         { "#comment" = "we are doing NAT" }
         { "lb_kind" = "NAT" }
         { "nat_mask" = "255.255.255.0" }
         { }
         { "protocol" = "TCP" }
         { }
         { "#comment" = "there can be as many real_server blocks as you need" }
         { }
         { "real_server"
           { "ip" = "10.20.40.10" }
           { "port" = "22" }
           { "#comment" = "if we used weighted round-robin or a similar lb algo," }
           { "#comment" = "we include the weight of this server" }
           { }
           { "weight" = "1" }
           { }
           { "#comment" = "here is a health checker for this server." }
           { "#comment" = "we could use a custom script here (see the keepalived docs)" }
           { "#comment" = "but we will just make sure we can do a vanilla tcp connect()" }
           { "#comment" = "on port 22" }
           { "#comment" = "if it fails, we will pull this realserver out of the pool" }
           { "#comment" = "and send email about the removal" }
           { "TCP_CHECK"
             { "connect_timeout" = "3" }
             { "connect_port" = "22" } } } }
       { }
       { "virtual_server_group" = "DNS_1"
         { "vip"
	   { "ipaddr" = "192.168.0.1" }
	   { "port" = "22" } }
	 { "vip"
	   { "ipaddr" = "10.234.55.22-25" }
	   { "port" = "36" } }
	 { "vip"
	   { "ipaddr" = "10.45.58.59"
	     { "prefixlen" = "32" } }
	   { "port" = "27" } } }
       { }
       { "vrrp_script" = "chk_apache2"
         { "#comment" = "Requires keepalived-1.1.13" }
         { "script" = "\"killall -0 apache2\""
           { "#comment" = "faster" } }
         { "interval" = "2"
           { "#comment" = "check every 2 seconds" } }
         { "weight" = "2"
           { "#comment" = "add 2 points of prio if OK" } } }
       { }
       { "#comment" = "that's all" }

