(*
Module: Test_Puppetfile
  Provides unit tests and examples for the <Puppetfile> lens.
*)
module Test_Puppetfile =

(* Test: Puppetfile.lns *)
test Puppetfile.lns get "forge \"https://forgeapi.puppetlabs.com\"

mod 'puppetlabs-razor'
mod 'puppetlabs-ntp', \"0.0.3\"

mod 'puppetlabs-apt',
  :git => \"git://github.com/puppetlabs/puppetlabs-apt.git\"

mod 'puppetlabs-stdlib',
  :git => \"git://github.com/puppetlabs/puppetlabs-stdlib.git\"

mod 'puppetlabs-apache', '0.6.0',
  :github_tarball => 'puppetlabs/puppetlabs-apache'

metadata\n" =
  { "forge" = "https://forgeapi.puppetlabs.com" }
  {  }
  { "1" = "puppetlabs-razor" }
  { "2" = "puppetlabs-ntp"
    { "@version" = "0.0.3" }
  }
  {  }
  { "3" = "puppetlabs-apt"
    { "git" = "git://github.com/puppetlabs/puppetlabs-apt.git" }
  }
  {  }
  { "4" = "puppetlabs-stdlib"
    { "git" = "git://github.com/puppetlabs/puppetlabs-stdlib.git" }
  }
  {  }
  { "5" = "puppetlabs-apache"
    { "@version" = "0.6.0" }
    { "github_tarball" = "puppetlabs/puppetlabs-apache" }
  }
  {  }
  { "metadata" }

(* Test: Puppetfile.lns
     Complex version conditions *)
test Puppetfile.lns get "mod 'puppetlabs/stdlib',        '< 5.0.0'
mod 'theforeman/concat_native', '>= 1.3.0 < 1.4.0'
mod 'herculesteam/augeasproviders', '2.1.x'\n" =
  { "1" = "puppetlabs/stdlib"
    { "@version" = "< 5.0.0" }
  }
  { "2" = "theforeman/concat_native"
    { "@version" = ">= 1.3.0 < 1.4.0" }
  }
  { "3" = "herculesteam/augeasproviders"
    { "@version" = "2.1.x" }
  }

(* Test: Puppetfile.lns
     Owner is not mandatory if git is given *)
test Puppetfile.lns get "mod 'stdlib',
  :git => \"git://github.com/puppetlabs/puppetlabs-stdlib.git\"\n" =
  { "1" = "stdlib"
    { "git" = "git://github.com/puppetlabs/puppetlabs-stdlib.git" } }


(* Issue #427 *)
test Puppetfile.lns get "mod 'puppetlabs/apache', :latest\n" =
  { "1" = "puppetlabs/apache"
    { "latest" } }

test Puppetfile.lns get "mod 'data',
  :git    => 'ssh://git@stash.example.com/bp/puppet-hiera.git',
  :branch => :control_branch,
  :default_branch => 'development',
  :install_path   => '.'\n" =
  { "1" = "data"
    { "git" = "ssh://git@stash.example.com/bp/puppet-hiera.git" }
    { "branch" = ":control_branch" }
    { "default_branch" = "development" }
    { "install_path" = "." } }
