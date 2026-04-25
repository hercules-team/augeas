(*
Module: Test_Puppetfile
  Provides unit tests and examples for the <Puppetfile> lens.
*)
module Test_Puppetfile =

(* Test: Puppetfile.lns *)
test Puppetfile.lns get "forge \"https://forgeapi.puppetlabs.com\" # the default forge

mod 'puppetlabs-razor'
mod 'puppetlabs-ntp', \"0.0.3\"

mod 'puppetlabs-apt',
  :git => \"git://github.com/puppetlabs/puppetlabs-apt.git\"

mod 'puppetlabs-stdlib',
  :git => \"git://github.com/puppetlabs/puppetlabs-stdlib.git\"

mod 'puppetlabs-apache', '0.6.0',
  :github_tarball => 'puppetlabs/puppetlabs-apache'

metadata # we want metadata\n" =
  { "forge" = "https://forgeapi.puppetlabs.com"
    { "#comment" = "the default forge" } }
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
  { "metadata" { "#comment" = "we want metadata" } }

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

(* Comment: after module name comma
    This conflicts with the comma comment tree below *)
test Puppetfile.lns get "mod 'data' # eol comment\n" = *

(* Comment: after first comma *)
test Puppetfile.lns get "mod 'data', # eol comment
  # and another
  '1.2.3'\n" =
  { "1" = "data"
    { "#comment" = "eol comment" }
    { "#comment" = "and another" }
    { "@version" = "1.2.3" } }

(* Comment: after version
    Current culprit: need two \n *)
test Puppetfile.lns get "mod 'data', '1.2.3' # eol comment\n" = *
test Puppetfile.lns get "mod 'data', '1.2.3' # eol comment\n\n" =
  { "1" = "data"
    { "@version" = "1.2.3" { "#comment" = "eol comment" } } }

(* Comment: eol after version comma *)
test Puppetfile.lns get "mod 'data', '1.2.3', # a comment
    :local => true\n" =
  { "1" = "data"
    { "@version" = "1.2.3" }
    { "#comment" = "a comment" }
    { "local" = "true" } }

(* Comment: after version comma with newline *)
test Puppetfile.lns get "mod 'data', '1.2.3',
 # a comment
    :local => true\n" =
  { "1" = "data"
    { "@version" = "1.2.3" }
    { "#comment" = "a comment" }
    { "local" = "true" } }

(* Comment: eol before opts, without version *)
test Puppetfile.lns get "mod 'data', # a comment
    # :ref => 'abcdef',
    :local => true\n" =
  { "1" = "data"
    { "#comment" = "a comment" }
    { "#comment" = ":ref => 'abcdef'," }
    { "local" = "true" } }

(* Comment: after opt comma *)
test Puppetfile.lns get "mod 'data', '1.2.3',
    :ref => 'abcdef', # eol comment
    :local => true\n" =
  { "1" = "data"
    { "@version" = "1.2.3" }
    { "ref" = "abcdef" }
    { "#comment" = "eol comment" }
    { "local" = "true" } }

(* Comment: in opts *)
test Puppetfile.lns get "mod 'data', '1.2.3',
    :ref => 'abcdef',
    # a comment
    :local => true\n" =
  { "1" = "data"
    { "@version" = "1.2.3" }
    { "ref" = "abcdef" }
    { "#comment" = "a comment" }
    { "local" = "true" } }

(* Comment: after last opt *)
test Puppetfile.lns get "mod 'data', '1.2.3',
    :local => true # eol comment\n\n" =
  { "1" = "data"
    { "@version" = "1.2.3" }
    { "local" = "true" }
    { "#comment" = "eol comment" } }
