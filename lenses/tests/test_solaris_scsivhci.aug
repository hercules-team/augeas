(* Test for solaris_scsivhci lens *)
module Test_solaris_scsivhci = 

  let ddiforceload = "ddi-forceload =
  \"misc/scsi_vhci/scsi_vhci_f_asym_sun\",
  \"misc/scsi_vhci/scsi_vhci_f_asym_lsi\",
  # line3 (scsi_vhci_f_asym_emc) comment
  \"misc/scsi_vhci/scsi_vhci_f_asym_emc\",
  \"misc/scsi_vhci/scsi_vhci_f_sym_emc\",
  \"misc/scsi_vhci/scsi_vhci_f_sym_hds\",
  \"misc/scsi_vhci/scsi_vhci_f_sym\",           # Symmetrical
# \"misc/scsi_vhci/scsi_vhci_f_tpgs_tape\",
# \"misc/scsi_vhci/scsi_vhci_f_tape\",
  \"misc/scsi_vhci/scsi_vhci_f_tpgs\";          # T10
"
  let file = "name=\"scsi_vhci\" class=\"root\";\n"
  let loadbalance = "load-balance=\"logical-block\";\n"
  let autofailback = "auto-failback=\"enable\";\n"

  let comments_block1 = "#
# Automatic failback configuration
# possible values are auto-failback=\"enable\" or auto-failback=\"disable\"
"

  let failoveroverride = "scsi-vhci-failover-override = 
  \"3PARdataVV\", \"f_sym\",
\"COMPELNTCompellent Vol\", \"f_sym\",
    \"HITACHI HUS723020ALS640\", \"f_sym\", # Hitachi UltraStar SAS HDD 2 TB
    \"HITACHI HUSSL4010ASS600\", \"f_sym\", # Hitachi UltraStar SAS SSD 100Gb
#   \"DGC     VRAID\", \"f_asym_emc\",
#   \"HITACHI DF600F\", \"f_sym\",
    \"HITACHI HUS156060VLS600\", \"f_sym\";
"

  test Solaris_ScsiVhci.lns get ddiforceload =
    { "ddi-forceload"
      { "1" = "misc/scsi_vhci/scsi_vhci_f_asym_sun" }
      { "2" = "misc/scsi_vhci/scsi_vhci_f_asym_lsi" }
      { "3" = "misc/scsi_vhci/scsi_vhci_f_asym_emc"
        { "#comment" = "line3 (scsi_vhci_f_asym_emc) comment" }
      }
      { "4" = "misc/scsi_vhci/scsi_vhci_f_sym_emc" }
      { "5" = "misc/scsi_vhci/scsi_vhci_f_sym_hds" }
      { "6" = "misc/scsi_vhci/scsi_vhci_f_sym" }
      { "7" = "misc/scsi_vhci/scsi_vhci_f_tpgs"
        { "#comment" = "Symmetrical" }
        { "#comment" = "\"misc/scsi_vhci/scsi_vhci_f_tpgs_tape\"," }
        { "#comment" = "\"misc/scsi_vhci/scsi_vhci_f_tape\"," }
      }
      { "#comment" = "T10" }
    }

  test Solaris_ScsiVhci.lns put ddiforceload
    after
    rm "/ddi-forceload/4" ;
    set "/ddi-forceload/3/#comment[1]" "just added line3 comment" ;
    set "/ddi-forceload/5/#comment[1]" "just added line5 comment" ;
    set "/ddi-forceload/7/#comment[4]" "just added line7 comment" ;
    set "/ddi-forceload/1" "new value"
    =
"ddi-forceload =
  \"new value\",
  \"misc/scsi_vhci/scsi_vhci_f_asym_lsi\",
  # just added line3 comment
  \"misc/scsi_vhci/scsi_vhci_f_asym_emc\",
# just added line5 comment

  \"misc/scsi_vhci/scsi_vhci_f_sym_hds\",
  \"misc/scsi_vhci/scsi_vhci_f_sym\",           # Symmetrical
# \"misc/scsi_vhci/scsi_vhci_f_tpgs_tape\",
# \"misc/scsi_vhci/scsi_vhci_f_tape\",

# just added line7 comment
  \"misc/scsi_vhci/scsi_vhci_f_tpgs\";          # T10
"

  test Solaris_ScsiVhci.lns get failoveroverride =
    { "scsi-vhci-failover-override"
      { "1"
        { "id" = "3PARdataVV" }
        { "module" = "f_sym" }
      }
      { "2"
        { "id" = "COMPELNTCompellent Vol" }
        { "module" = "f_sym" }
      }
      { "3"
        { "id" = "HITACHI HUS723020ALS640" }
        { "module" = "f_sym" }
      }
      { "4"
        { "#comment" = "Hitachi UltraStar SAS HDD 2 TB" }
        { "id" = "HITACHI HUSSL4010ASS600" }
        { "module" = "f_sym" }
      }
      { "5"
        { "#comment" = "Hitachi UltraStar SAS SSD 100Gb" }
        { "#comment" = "\"DGC     VRAID\", \"f_asym_emc\"," }
        { "#comment" = "\"HITACHI DF600F\", \"f_sym\"," }
        { "id" = "HITACHI HUS156060VLS600" }
        { "module" = "f_sym" }
      }
    }

  test Solaris_ScsiVhci.lns put failoveroverride
    after
      rm "/scsi-vhci-failover-override/1" ;
      set "/scsi-vhci-failover-override/2/id" "NEWID   NEWPID" ;
      set "/scsi-vhci-failover-override/2/module" "my_module" ;
      set "/scsi-vhci-failover-override/6/id" "HITACHI HUS156060VLS600" ;
      set "/scsi-vhci-failover-override/6/module" "f_sym" ;
      set "/scsi-vhci-failover-override/7/id" "HITACHI HUS156060VLS600" ;
      set "/scsi-vhci-failover-override/7/module" "f_sym" ;
      rm "/scsi-vhci-failover-override/5"
    =
"scsi-vhci-failover-override =
\"NEWID   NEWPID\", \"my_module\",
    \"HITACHI HUS723020ALS640\", \"f_sym\", # Hitachi UltraStar SAS HDD 2 TB
    \"HITACHI HUSSL4010ASS600\", \"f_sym\",
\t\"HITACHI HUS156060VLS600\", \"f_sym\",
\t\"HITACHI HUS156060VLS600\", \"f_sym\";
"

  let conf =
      "# CDDL HEADER START\n" .
      file .
      loadbalance .
      comments_block1 .
      autofailback

  test Solaris_ScsiVhci.lns get conf =
      { "#comment" = "CDDL HEADER START" }
      { "file"
        { "name" = "scsi_vhci" }
        { "class" = "root" }
      }
      { "load-balance" = "logical-block"}
      { "#comment" }
      { "#comment" = "Automatic failback configuration" }
      { "#comment" = "possible values are auto-failback=\"enable\" or auto-failback=\"disable\"" }
      { "auto-failback" = "enable"}
