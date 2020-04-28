[![Build Status](https://travis-ci.org/hercules-team/augeas.svg?branch=master)](https://travis-ci.org/hercules-team/augeas)

Introduction
------------

  Augeas is a library and command line tool that focuses on the most basic
  problem in handling Linux configurations programmatically: editing actual
  configuration files in a controlled manner.

  To that end, Augeas exposes a tree of all configuration settings (well,
  all the ones it knows about) and a simple local API for manipulating the
  tree. Augeas then modifies underlying configuration files according to
  the changes that have been made to the tree; it does as little modeling
  of configurations as possible, and focuses exclusively on transforming
  the tree-oriented syntax of its public API to the myriad syntaxes of
  individual configuration files.

  This focus on editing sets Augeas apart from any other configuration tool
  I know of. Hopefully, Augeas will form a more solid foundation on which
  these tools can be built; with a clean, simple API these tools should
  be able to focus more on their core concerns and less on the mechanics
  of running sed, grep, awk, etc. to tweak a config file.

  If all you need is a tool to edit configuration files, you only need to
  concern yourself with the handful of public API calls that Augeas exposes
  (or their equivalent language bindings). However, to teach Augeas about a
  new file format, you need to describe that file format in Augeas's domain
  specific language (a very small subset of ML) Documentation for that
  language can be found on the Augeas website at http://augeas.net/ If you
  do that, please contribute the description if at all possible, or include
  it in the distribution of your software - all you need to do for that is
  add a couple of text files, there is no need to change existing
  code. Ultimately, Augeas should describe all config files commonly found
  on a Linux system.

Non-goals
---------

Augeas is as much defined by the things it does _not_ try to accomplish
as by its goals:

* No abstraction from native config format, i.e. the organization of
  the tree mirrors closely how the native config files are organized
* No cross-platform abstraction - what is logically the same value may
  live in different places in the tree on different
  distributions. Dealing with that should be left to a higher-level
  tool
* No remote management support. Augeas is a local API, other ways of
  access to Augeas should be built on top of it
* No (or very little) modelling. Augeas is focused on syntax
  transformation, not on any higher-level understanding of
  configuration.

The above non-goals are of course important concerns in
practice. Historically though, too many config mgmt projects have failed
because they set their sights too high and tried to address syntax
transformation, modelling, remote support, and scalable management all in
one. That leads to a lack of focus, and to addressing each of those goals
unsatisfactorily.

Building
--------

These instructions apply to building a released tarball. If you want to
build from a git checkout, see the file HACKING.

See the generic instructions in INSTALL. Generally,

      ./configure
      make && make install
should be all that is needed.

You need to have readline-devel installed. On systems that support
SELinux, you should also install libselinux-devel.

Documentation
-------------

Documentation can be found on Augeas' website http://augeas.net/ The site
also contains information on how to get in touch, what you can do to help
etc.

License
-------

Augeas is released under the [Lesser General Public License, Version 2.1](http://www.gnu.org/licenses/lgpl-2.1.html)
See the file COPYING for details.
