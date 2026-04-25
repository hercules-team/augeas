
module Test_ceph =

  let ceph_simple = "[global]
### http://ceph.com/docs/master/rados/configuration/general-config-ref/

    fsid                       = b4b2e571-fbbf-4ff3-a9f8-ab80f08b7fe6    # use `uuidgen` to generate your own UUID
    public network             = 192.168.0.0/24
    cluster network            = 192.168.0.0/24
"

  let ceph_extended = "##
# Sample ceph ceph.conf file.
##
# This file defines cluster membership, the various locations
# that Ceph stores data, and any other runtime options.

[global]
    ; Start-line comment
    log file                   = /var/log/ceph/$cluster-$name.log ; End-line comment

   [mon.alpha]
    host                       = alpha
    mon addr                   = 192.168.0.10:6789

  [mon.beta]
    host                       = beta
    mon addr                   = 192.168.0.11:6789

[mon.gamma]
    host                       = gamma
    mon addr                   = 192.168.0.12:6789


[mon]
mon initial members        = mycephhost

mon host                   = cephhost01,cephhost02
    mon addr                   = 192.168.0.101,192.168.0.102

mon osd nearfull ratio     = .85

    # logging, for debugging monitor crashes, in order of
    # their likelihood of being helpful :)
    debug ms                   = 1
    debug mon                  = 20
    debug paxos                = 20
    debug auth                 = 20

[mds]

    osd max backfills            = 5

    #osd mkfs type = {fs-type}
    #osd mkfs options {fs-type}   = {mkfs options}   # default for xfs is \"-f\"
    #osd mount options {fs-type}  = {mount options} # default mount option is \"rw, noatime\"
    osd mkfs type                = btrfs
    osd mount options btrfs      = noatime,nodiratime
    journal dio                  = false

    debug ms                     = 1
    debug osd                    = 20
    debug filestore              = 20
    debug journal                = 20

    filestore merge threshold    = 10

    osd crush update on start    = false

[osd.0]
    host                         = delta

[osd.1]
    host                         = epsilon

[osd.2]
    host                         = zeta

[osd.3]
    host                         = eta

    rgw dns name                 = radosgw.ceph.internal
"

  test Ceph.lns get ceph_simple =
  { "global"
    { "#comment" = "## http://ceph.com/docs/master/rados/configuration/general-config-ref/" }
    { }
    { "fsid" = "b4b2e571-fbbf-4ff3-a9f8-ab80f08b7fe6"
      { "#comment" = "use `uuidgen` to generate your own UUID" }
    }
    { "public network" = "192.168.0.0/24" }
    { "cluster network" = "192.168.0.0/24" }
  }

  test Ceph.lns get ceph_extended =
    { "#comment" = "#" }
    { "#comment" = "Sample ceph ceph.conf file." }
    { "#comment" = "#" }
    { "#comment" = "This file defines cluster membership, the various locations" }
    { "#comment" = "that Ceph stores data, and any other runtime options." }
    {  }
    { "global"
      { "#comment" = "Start-line comment" }
      { "log file" = "/var/log/ceph/$cluster-$name.log"
        { "#comment" = "End-line comment" }
      }
      {  }
    }
    { "mon.alpha"
      { "host" = "alpha" }
      { "mon addr" = "192.168.0.10:6789" }
      {  }
    }
    { "mon.beta"
      { "host" = "beta" }
      { "mon addr" = "192.168.0.11:6789" }
      {  }
    }
    { "mon.gamma"
      { "host" = "gamma" }
      { "mon addr" = "192.168.0.12:6789" }
      {  }
      {  }
    }
    { "mon"
      { "mon initial members" = "mycephhost" }
      {  }
      { "mon host" = "cephhost01,cephhost02" }
      { "mon addr" = "192.168.0.101,192.168.0.102" }
      {  }
      { "mon osd nearfull ratio" = ".85" }
      {  }
      { "#comment" = "logging, for debugging monitor crashes, in order of" }
      { "#comment" = "their likelihood of being helpful :)" }
      { "debug ms" = "1" }
      { "debug mon" = "20" }
      { "debug paxos" = "20" }
      { "debug auth" = "20" }
      {  }
    }
    { "mds"
      {  }
      { "osd max backfills" = "5" }
      {  }
      { "#comment" = "osd mkfs type = {fs-type}" }
      { "#comment" = "osd mkfs options {fs-type}   = {mkfs options}   # default for xfs is \"-f\"" }
      { "#comment" = "osd mount options {fs-type}  = {mount options} # default mount option is \"rw, noatime\"" }
      { "osd mkfs type" = "btrfs" }
      { "osd mount options btrfs" = "noatime,nodiratime" }
      { "journal dio" = "false" }
      {  }
      { "debug ms" = "1" }
      { "debug osd" = "20" }
      { "debug filestore" = "20" }
      { "debug journal" = "20" }
      {  }
      { "filestore merge threshold" = "10" }
      {  }
      { "osd crush update on start" = "false" }
      {  }
    }
    { "osd.0"
      { "host" = "delta" }
      {  }
    }
    { "osd.1"
      { "host" = "epsilon" }
      {  }
    }
    { "osd.2"
      { "host" = "zeta" }
      {  }
    }
    { "osd.3"
      { "host" = "eta" }
      {  }
      { "rgw dns name" = "radosgw.ceph.internal" }
    }

