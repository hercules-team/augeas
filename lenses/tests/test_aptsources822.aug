module Test_Aptsources822 =

(* Test multiple values, multi-line values and multiple source stanzas *)
let sources1 = "Enabled: yes
Types: deb deb-src
URIs: http://deb.debian.org/debian
Suites: bullseye
 bullseye-backports
Components: main non-free-firmware
Allow-Insecure: no
Signed-By:
 -----BEGIN PGP PUBLIC KEY BLOCK-----
 .
 mDMEYCQjIxYJKwYBBAHaRw8BAQdAD/P5Nvvnvk66SxBBHDbhRml9ORg1WV5CvzKY
 CuMfoIS0BmFiY2RlZoiQBBMWCgA4FiEErCIG1VhKWMWo2yfAREZd5NfO31cFAmAk
 IyMCGyMFCwkIBwMFFQoJCAsFFgIDAQACHgECF4AACgkQREZd5NfO31fbOwD6ArzS
 dM0Dkd5h2Ujy1b6KcAaVW9FOa5UNfJ9FFBtjLQEBAJ7UyWD3dZzhvlaAwunsk7DG
 3bHcln8DMpIJVXht78sL
 =IE0r
 -----END PGP PUBLIC KEY BLOCK-----

Enabled: no
URIs: http://dl.google.com/linux/chrome/deb
Suites: stable
Components: main
Architectures: amd64
"
test Aptsources822.lns get sources1 =
  { "1"
    { "Enabled" = "yes" }
    { "Types"
      { "1" = "deb" }
      { "2" = "deb-src" }
    }
    { "URIs" { "1" = "http://deb.debian.org/debian" } }
    { "Suites"
      { "1" = "bullseye\n" }
      { "2" = "bullseye-backports" }
    }
    { "Components"
      { "1" = "main" }
      { "2" = "non-free-firmware" }
    }
    { "Allow-Insecure" = "no" }
    { "Signed-By"
      { "1" = "-----BEGIN" }
      { "2" = "PGP" }
      { "3" = "PUBLIC" }
      { "4" = "KEY" }
      { "5" = "BLOCK-----\n" }
      { "6" = ".\n" }
      { "7" = "mDMEYCQjIxYJKwYBBAHaRw8BAQdAD/P5Nvvnvk66SxBBHDbhRml9ORg1WV5CvzKY\n" }
      { "8" = "CuMfoIS0BmFiY2RlZoiQBBMWCgA4FiEErCIG1VhKWMWo2yfAREZd5NfO31cFAmAk\n" }
      { "9" = "IyMCGyMFCwkIBwMFFQoJCAsFFgIDAQACHgECF4AACgkQREZd5NfO31fbOwD6ArzS\n" }
      { "10" = "dM0Dkd5h2Ujy1b6KcAaVW9FOa5UNfJ9FFBtjLQEBAJ7UyWD3dZzhvlaAwunsk7DG\n" }
      { "11" = "3bHcln8DMpIJVXht78sL\n" }
      { "12" = "=IE0r\n" }
      { "13" = "-----END" }
      { "14" = "PGP" }
      { "15" = "PUBLIC" }
      { "16" = "KEY" }
      { "17" = "BLOCK-----" }
    }
  }
  { }
  { "2"
    { "Enabled" = "no" }
    { "URIs" { "1" = "http://dl.google.com/linux/chrome/deb" } }
    { "Suites" { "1" = "stable" } }
    { "Components" { "1" = "main" } }
    { "Architectures" { "1" = "amd64" } }
  }

let sources2 = "Enabled: yes
Types: deb deb-src
URIs: http://archive.ubuntu.com/ubuntu
Suites: disco disco-updates disco-security disco-backports
Components: main universe multiverse restricted

Enabled: yes
Types: deb
URIs: http://ppa.launchpad.net/system76/pop/ubuntu
Suites: disco
Components: main
"
test Aptsources822.lns get sources2 =
  { "1"
    { "Enabled" = "yes" }
    { "Types"
      { "1" = "deb" }
      { "2" = "deb-src" }
    }
    { "URIs" { "1" = "http://archive.ubuntu.com/ubuntu" } }
    { "Suites"
      { "1" = "disco" }
      { "2" = "disco-updates" }
      { "3" = "disco-security" }
      { "4" = "disco-backports" }
    }
    { "Components"
      { "1" = "main" }
      { "2" = "universe" }
      { "3" = "multiverse" }
      { "4" = "restricted" }
    }
  }
  { }
  { "2"
    { "Enabled" = "yes" }
    { "Types" { "1" = "deb" } }
    { "URIs" { "1" = "http://ppa.launchpad.net/system76/pop/ubuntu" } }
    { "Suites" { "1" = "disco" } }
    { "Components" { "1" = "main" } }
  }

(* Test adding nodes to tree *)
test Aptsources822.lns put "Types: deb\n" after set "/1/Enabled" "yes" = "Types: deb\nEnabled: yes\n"
test Aptsources822.lns put "Types: deb\n" after
     set "/1/URIs/1" "uri1\n";
     set "/1/URIs/2" "uri2" = "Types: deb\nURIs: uri1\n uri2\n"
