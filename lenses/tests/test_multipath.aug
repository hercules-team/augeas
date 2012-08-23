module Test_multipath =

  let conf = "# Blacklist all devices by default.
blacklist {
        devnode \"*\"
        wwid    *
}

# By default, devices with vendor = \"IBM\" and product = \"S/390.*\" are
# blacklisted. To enable mulitpathing on these devies, uncomment the
# following lines.
blacklist_exceptions {
	device {
		vendor	\"IBM\"
		product	\"S/390.*\"
	}
}

#
# Here is an example of how to configure some standard options.
#

defaults {
	udev_dir		/dev
	polling_interval 	10
	selector		\"round-robin 0\"
	path_grouping_policy	multibus
	getuid_callout		\"/sbin/scsi_id --whitelisted /dev/%n\"
	prio			alua
	path_checker		readsector0
	rr_min_io		100
	max_fds			8192
	rr_weight		priorities
	failback		immediate
	no_path_retry		fail
	user_friendly_names	yes
  dev_loss_tmo  30
  max_polling_interval  300
  verbosity 2
  reassign_maps yes
  fast_io_fail_tmo  5
  async_timeout 5
}

# Sections without empty lines in between
blacklist {
       wwid 26353900f02796769
	devnode \"^(ram|raw|loop|fd|md|dm-|sr|scd|st)[0-9]*\"

    # Comments and blank lines inside a section
	devnode \"^hd[a-z]\"

}
multipaths {
	multipath {
		wwid			3600508b4000156d700012000000b0000
		alias			yellow
		path_grouping_policy	multibus
		path_checker		readsector0
		path_selector		\"round-robin 0\"
		failback		manual
		rr_weight		priorities
		no_path_retry		5
	}
	multipath {
		wwid			1DEC_____321816758474
		alias			red
	}
}
devices {
	device {
		vendor			\"COMPAQ  \"
		product			\"HSV110 (C)COMPAQ\"
		path_grouping_policy	multibus
		getuid_callout          \"/sbin/scsi_id --whitelisted /dev/%n\"
		path_checker		readsector0
		path_selector		\"round-robin 0\"
		hardware_handler	\"0\"
		failback		15
		rr_weight		priorities
		no_path_retry		queue
	}
	device {
		vendor			\"COMPAQ  \"
		product			\"MSA1000         \"
		path_grouping_policy	multibus
		polling_interval	9
	}
}\n"

test Multipath.lns get conf =
  { "#comment" = "Blacklist all devices by default." }
  { "blacklist"
    { "devnode" = "*" }
    { "wwid"    = "*" } }
  { }
  { "#comment" = "By default, devices with vendor = \"IBM\" and product = \"S/390.*\" are" }
  { "#comment" = "blacklisted. To enable mulitpathing on these devies, uncomment the" }
  { "#comment" = "following lines." }
  { "blacklist_exceptions"
    { "device"
      { "vendor" = "IBM" }
      { "product" = "S/390.*" } } }
  { }
  { }
  { "#comment" = "Here is an example of how to configure some standard options." }
  { }
  { }
  { "defaults"
    { "udev_dir" = "/dev" }
    { "polling_interval" = "10" }
    { "selector" = "round-robin 0" }
    { "path_grouping_policy" = "multibus" }
    { "getuid_callout" = "/sbin/scsi_id --whitelisted /dev/%n" }
    { "prio" = "alua" }
    { "path_checker" = "readsector0" }
    { "rr_min_io" = "100" }
    { "max_fds" = "8192" }
    { "rr_weight" = "priorities" }
    { "failback" = "immediate" }
    { "no_path_retry" = "fail" }
    { "user_friendly_names" = "yes" }
    { "dev_loss_tmo" = "30" }
    { "max_polling_interval" = "300" }
    { "verbosity" = "2" }
    { "reassign_maps" = "yes" }
    { "fast_io_fail_tmo" = "5" }
    { "async_timeout" = "5" } }
  { }
  { "#comment" = "Sections without empty lines in between" }
  { "blacklist"
    { "wwid" = "26353900f02796769" }
    { "devnode" = "^(ram|raw|loop|fd|md|dm-|sr|scd|st)[0-9]*" }
    { }
    { "#comment" = "Comments and blank lines inside a section" }
    { "devnode" = "^hd[a-z]" }
    { } }
  { "multipaths"
    { "multipath"
      { "wwid" = "3600508b4000156d700012000000b0000" }
      { "alias" = "yellow" }
      { "path_grouping_policy" = "multibus" }
      { "path_checker" = "readsector0" }
      { "path_selector" = "round-robin 0" }
      { "failback" = "manual" }
      { "rr_weight" = "priorities" }
      { "no_path_retry" = "5" } }
    { "multipath"
      { "wwid" = "1DEC_____321816758474" }
      { "alias" = "red" } } }
  { "devices"
    { "device"
      { "vendor" = "COMPAQ  " }
      { "product" = "HSV110 (C)COMPAQ" }
      { "path_grouping_policy" = "multibus" }
      { "getuid_callout" = "/sbin/scsi_id --whitelisted /dev/%n" }
      { "path_checker" = "readsector0" }
      { "path_selector" = "round-robin 0" }
      { "hardware_handler" = "0" }
      { "failback" = "15" }
      { "rr_weight" = "priorities" }
      { "no_path_retry" = "queue" } }
    { "device"
      { "vendor" = "COMPAQ  " }
      { "product" = "MSA1000         " }
      { "path_grouping_policy" = "multibus" }
      { "polling_interval" = "9" } } }
