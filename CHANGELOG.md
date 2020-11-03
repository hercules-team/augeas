# Changelog

## [Unreleased](https://github.com/hercules-team/augeas/tree/HEAD)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.12.0...HEAD)

**Closed issues:**

- postgresql.conf can't be parsed due to password\_encryption = scram-sha-256 [\#700](https://github.com/hercules-team/augeas/issues/700)
- Is it possible to import back in augeas, the output of a dump-xml command? [\#656](https://github.com/hercules-team/augeas/issues/656)
- Yum lens does not allow specifying multiple excludes [\#654](https://github.com/hercules-team/augeas/issues/654)
- Rsyslog.lns does not support multiple actions [\#652](https://github.com/hercules-team/augeas/issues/652)
- \[lens\] authinfo2 file for S3QL [\#646](https://github.com/hercules-team/augeas/issues/646)
- opendkim.conf failed to be parsed without error [\#643](https://github.com/hercules-team/augeas/issues/643)
- Postfix master: cannot parse unix-dgram values [\#635](https://github.com/hercules-team/augeas/issues/635)
- put\_failed for centos 7.6 pam module [\#634](https://github.com/hercules-team/augeas/issues/634)
- augeas Httpd lens don't allow ' \' [\#633](https://github.com/hercules-team/augeas/issues/633)
- Ssh: cannot parse Match options [\#632](https://github.com/hercules-team/augeas/issues/632)
- Update Changelog on Website [\#624](https://github.com/hercules-team/augeas/issues/624)
- What about making an official augeas docker image [\#623](https://github.com/hercules-team/augeas/issues/623)

**Merged pull requests:**

- postgresql.aug: Allow hyphen '-' in values that don't require quotes … [\#701](https://github.com/hercules-team/augeas/pull/701) ([marcinbarczynski](https://github.com/marcinbarczynski))
- Chrony: add new options [\#698](https://github.com/hercules-team/augeas/pull/698) ([mlichvar](https://github.com/mlichvar))
- Ssh: add Match keyword support [\#695](https://github.com/hercules-team/augeas/pull/695) ([granquet](https://github.com/granquet))
- Tmpfiles: few improvements to the types specification [\#694](https://github.com/hercules-team/augeas/pull/694) ([ptoscano](https://github.com/ptoscano))
- Sudoers: support new @include/@includedir directives [\#693](https://github.com/hercules-team/augeas/pull/693) ([ptoscano](https://github.com/ptoscano))
- Add function modified\(\) to select nodes which are marked as dirty [\#691](https://github.com/hercules-team/augeas/pull/691) ([georgehansper](https://github.com/georgehansper))
- Add CLI command 'preview' and API 'aug\_preview' to preview file contents [\#690](https://github.com/hercules-team/augeas/pull/690) ([georgehansper](https://github.com/georgehansper))
- src/fa.c: fixed error in regexp grammar in comment [\#689](https://github.com/hercules-team/augeas/pull/689) ([safinaskar](https://github.com/safinaskar))
- properties: support "/" in property names [\#680](https://github.com/hercules-team/augeas/pull/680) ([felixdoerre](https://github.com/felixdoerre))
- \* src/lens.c: Do not require that keys and labels not match '/' [\#679](https://github.com/hercules-team/augeas/pull/679) ([lutter](https://github.com/lutter))
- Allow underscore in postfix\_transport [\#678](https://github.com/hercules-team/augeas/pull/678) ([spacedog](https://github.com/spacedog))
- lense: add RFC3986 URL regex [\#676](https://github.com/hercules-team/augeas/pull/676) ([jcpunk](https://github.com/jcpunk))
- README.md: fix minor typo [\#669](https://github.com/hercules-team/augeas/pull/669) ([ja5087](https://github.com/ja5087))
- lib: remove extra @LIBS@ from pkg-config file [\#666](https://github.com/hercules-team/augeas/pull/666) ([ptoscano](https://github.com/ptoscano))
- Krb5: handle \[plugins\] subsection [\#663](https://github.com/hercules-team/augeas/pull/663) ([ptoscano](https://github.com/ptoscano))
- Mke2fs: add old\_bitmaps option in \[options\] [\#662](https://github.com/hercules-team/augeas/pull/662) ([ptoscano](https://github.com/ptoscano))
- Systemd: fix parsing of envvars with spaces [\#659](https://github.com/hercules-team/augeas/pull/659) ([ptoscano](https://github.com/ptoscano))
- Add Cmdline lens [\#658](https://github.com/hercules-team/augeas/pull/658) ([t-8ch](https://github.com/t-8ch))
- Add hourly schedule to logrotate lens. [\#655](https://github.com/hercules-team/augeas/pull/655) ([smithj4](https://github.com/smithj4))
- Rsyslog: support multiple actions in filters and selectors [\#653](https://github.com/hercules-team/augeas/pull/653) ([raphink](https://github.com/raphink))
- Add lens fail2ban [\#651](https://github.com/hercules-team/augeas/pull/651) ([Salokyn](https://github.com/Salokyn))
- Add Dockerfile [\#650](https://github.com/hercules-team/augeas/pull/650) ([Salokyn](https://github.com/Salokyn))
- Add lens Authinfo2 [\#649](https://github.com/hercules-team/augeas/pull/649) ([Salokyn](https://github.com/Salokyn))
- Grub: support '+' in kernel command line option names [\#647](https://github.com/hercules-team/augeas/pull/647) ([ptoscano](https://github.com/ptoscano))
- Support colons in the domain pattern of the limits lens [\#645](https://github.com/hercules-team/augeas/pull/645) ([XMol](https://github.com/XMol))
- Fix \#643 - Update of opendkim lens to match current opendkim.conf file format [\#644](https://github.com/hercules-team/augeas/pull/644) ([doc75](https://github.com/doc75))
- Few improvements to the Mke2fs lens [\#642](https://github.com/hercules-team/augeas/pull/642) ([ptoscano](https://github.com/ptoscano))
-  \* .travis.yml: install libtool-bin [\#639](https://github.com/hercules-team/augeas/pull/639) ([ptoscano](https://github.com/ptoscano))
- Add ocsinventory-agent.cfg to SimpleVars [\#637](https://github.com/hercules-team/augeas/pull/637) ([jcpunk](https://github.com/jcpunk))
- Postfix\_Master: Allow unix-dgram as type \(fix \#635\) [\#636](https://github.com/hercules-team/augeas/pull/636) ([raphink](https://github.com/raphink))
- A couple of augtool completion fixes [\#631](https://github.com/hercules-team/augeas/pull/631) ([ptoscano](https://github.com/ptoscano))
- Krb5: improve \[dbmodules\] and includes [\#630](https://github.com/hercules-team/augeas/pull/630) ([ptoscano](https://github.com/ptoscano))
- \* src/put.c \(split\_concat\): initialize regs earlier [\#629](https://github.com/hercules-team/augeas/pull/629) ([ptoscano](https://github.com/ptoscano))
- vim: recognize also 'excl' as statements [\#628](https://github.com/hercules-team/augeas/pull/628) ([ptoscano](https://github.com/ptoscano))
- Shellvars: exclude more tcsh profile scripts [\#627](https://github.com/hercules-team/augeas/pull/627) ([ptoscano](https://github.com/ptoscano))

## [release-1.12.0](https://github.com/hercules-team/augeas/tree/release-1.12.0) (2019-04-13)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.11.0...release-1.12.0)

**Fixed bugs:**

- Puppetfile lens can't parse "mod 'puppetlabs/apache', :latest" [\#427](https://github.com/hercules-team/augeas/issues/427)
- Ability to modify files under /proc [\#97](https://github.com/hercules-team/augeas/issues/97)

**Closed issues:**

- Nginx : upstream block not supported [\#617](https://github.com/hercules-team/augeas/issues/617)
- How to undo aug\_set when aug\_save fails? [\#613](https://github.com/hercules-team/augeas/issues/613)
- ssh.aug does not parse RekeyLimit .. perhaps others. [\#605](https://github.com/hercules-team/augeas/issues/605)
- ssh.aug does not parse many lines that can be found in /etc/ssh/ssh\_config [\#604](https://github.com/hercules-team/augeas/issues/604)
- Dovecot lens doesn't allow ! in block titles [\#598](https://github.com/hercules-team/augeas/issues/598)
- Multiline in /etc/pam.d/su breaks Augeas [\#590](https://github.com/hercules-team/augeas/issues/590)
- multipath.aug: quotation marks in key-value parameters [\#583](https://github.com/hercules-team/augeas/issues/583)
- Hjson Lens [\#365](https://github.com/hercules-team/augeas/issues/365)

**Merged pull requests:**

- Rsyslog: support dynamic file paths [\#622](https://github.com/hercules-team/augeas/pull/622) ([raphink](https://github.com/raphink))
- Update gnulib to 91584ed6 [\#621](https://github.com/hercules-team/augeas/pull/621) ([lutter](https://github.com/lutter))
- Puppetfile: allow comments in entries [\#620](https://github.com/hercules-team/augeas/pull/620) ([raphink](https://github.com/raphink))
- Puppetfile: allow symbols as \(optional\) values \(fix \#427\) [\#619](https://github.com/hercules-team/augeas/pull/619) ([raphink](https://github.com/raphink))
- Support unix sockets as server address \(fix \#617\) [\#618](https://github.com/hercules-team/augeas/pull/618) ([raphink](https://github.com/raphink))
- Fixes missing recognition of double quoted filenames. [\#611](https://github.com/hercules-team/augeas/pull/611) ([wornet-mwo](https://github.com/wornet-mwo))
- nsswitch: Add action merge [\#609](https://github.com/hercules-team/augeas/pull/609) ([frh](https://github.com/frh))
- Ssh: accept RekeyLimit [\#607](https://github.com/hercules-team/augeas/pull/607) ([lutter](https://github.com/lutter))
- Allow Creation of Hostname When File is Missing [\#606](https://github.com/hercules-team/augeas/pull/606) ([davidpfarrell](https://github.com/davidpfarrell))
- Added more pkinit\_\* options. [\#603](https://github.com/hercules-team/augeas/pull/603) ([smithj4](https://github.com/smithj4))
- Shellvars: accept variable as command [\#601](https://github.com/hercules-team/augeas/pull/601) ([raphink](https://github.com/raphink))
- Few simple Shellvars improvements [\#600](https://github.com/hercules-team/augeas/pull/600) ([ptoscano](https://github.com/ptoscano))
- Permit ! in dovecot block titles [\#599](https://github.com/hercules-team/augeas/pull/599) ([nward](https://github.com/nward))
- New lens: Anaconda [\#597](https://github.com/hercules-team/augeas/pull/597) ([ptoscano](https://github.com/ptoscano))
- Xorg: Accept empty values for options [\#596](https://github.com/hercules-team/augeas/pull/596) ([arnolda](https://github.com/arnolda))
- Rsyslog: bsd-like \#!/+/- are just comments without special meaning [\#595](https://github.com/hercules-team/augeas/pull/595) ([arnolda](https://github.com/arnolda))
- New lens: Semanage [\#594](https://github.com/hercules-team/augeas/pull/594) ([ptoscano](https://github.com/ptoscano))
- Pam: accept continuation lines [\#592](https://github.com/hercules-team/augeas/pull/592) ([lutter](https://github.com/lutter))
- A couple small fixes [\#589](https://github.com/hercules-team/augeas/pull/589) ([lutter](https://github.com/lutter))
- Fix sudoers lens: "always\_query\_group\_plugin" [\#588](https://github.com/hercules-team/augeas/pull/588) ([traylenator](https://github.com/traylenator))
- Allow parsing of options with equal sign [\#587](https://github.com/hercules-team/augeas/pull/587) ([emildragu](https://github.com/emildragu))
- lexer: disable 'input' and 'yyunput' functions [\#585](https://github.com/hercules-team/augeas/pull/585) ([ptoscano](https://github.com/ptoscano))
- Multipath: accept values enclosed in quotes [\#584](https://github.com/hercules-team/augeas/pull/584) ([lutter](https://github.com/lutter))
- Shellvars: allow and/or in @if conditions [\#582](https://github.com/hercules-team/augeas/pull/582) ([raphink](https://github.com/raphink))
- TOML lens [\#91](https://github.com/hercules-team/augeas/pull/91) ([raphink](https://github.com/raphink))

## [release-1.11.0](https://github.com/hercules-team/augeas/tree/release-1.11.0) (2018-08-24)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.10.1...release-1.11.0)

**Fixed bugs:**

- Redis lense doesn't support multiple IPs in bind command [\#194](https://github.com/hercules-team/augeas/issues/194)

**Closed issues:**

- httpd lens Apache parsing error with conditional statements and \(empty\) comments [\#577](https://github.com/hercules-team/augeas/issues/577)
- Memory usage issues with large Apache httpd configuration files [\#569](https://github.com/hercules-team/augeas/issues/569)
- nginx: Support semicolons in values [\#566](https://github.com/hercules-team/augeas/issues/566)
- multipath.aug - unable to set "blacklist{ wwid .\*}" [\#564](https://github.com/hercules-team/augeas/issues/564)
- Json.lns how to set a value to true or false [\#559](https://github.com/hercules-team/augeas/issues/559)
- Json.lns fail to handle backslashes [\#557](https://github.com/hercules-team/augeas/issues/557)
- segmentation fault in augmatch with no input file [\#556](https://github.com/hercules-team/augeas/issues/556)
- A suspicious misusage of realloc\(\)? [\#554](https://github.com/hercules-team/augeas/issues/554)
- php.aug: "set" to uncomment existing directives rather than adding new ones [\#550](https://github.com/hercules-team/augeas/issues/550)
- Error in /etc/systemd/system/kubelet.service.d \(put\_read\)   Is a directory [\#548](https://github.com/hercules-team/augeas/issues/548)
- Append '\n' to work around lenses broken by files without ending newline? [\#547](https://github.com/hercules-team/augeas/issues/547)
- augmatch name [\#542](https://github.com/hercules-team/augeas/issues/542)

**Merged pull requests:**

- Redis: accept bind command [\#580](https://github.com/hercules-team/augeas/pull/580) ([lutter](https://github.com/lutter))
- Httpd: accept whitespace comment after tag opening a section [\#579](https://github.com/hercules-team/augeas/pull/579) ([lutter](https://github.com/lutter))
- Replace pure function invocations in path expressions with their result [\#578](https://github.com/hercules-team/augeas/pull/578) ([lutter](https://github.com/lutter))
- Fix few minor memory leaks [\#576](https://github.com/hercules-team/augeas/pull/576) ([ptoscano](https://github.com/ptoscano))
- fa: check result of fopen [\#575](https://github.com/hercules-team/augeas/pull/575) ([ptoscano](https://github.com/ptoscano))
- augtool: fix access to invalid memory [\#574](https://github.com/hercules-team/augeas/pull/574) ([ptoscano](https://github.com/ptoscano))
- Nginx: allow semicolons inside double quoted strings in simple direct… [\#567](https://github.com/hercules-team/augeas/pull/567) ([lutter](https://github.com/lutter))
- augmatch: add a --quiet option and make exit status useful [\#563](https://github.com/hercules-team/augeas/pull/563) ([lutter](https://github.com/lutter))
- Grub: tolerate some invalid entries [\#562](https://github.com/hercules-team/augeas/pull/562) ([lutter](https://github.com/lutter))
- Parse nginx sites-enabled [\#560](https://github.com/hercules-team/augeas/pull/560) ([plumbeo](https://github.com/plumbeo))
- Json: allow escaped slashes in strings [\#558](https://github.com/hercules-team/augeas/pull/558) ([lutter](https://github.com/lutter))
- Fix several memory leaks in augmatch [\#555](https://github.com/hercules-team/augeas/pull/555) ([qiankehan](https://github.com/qiankehan))
- Systemd: explicitly exclude \*.d and \*.wants directories [\#552](https://github.com/hercules-team/augeas/pull/552) ([lutter](https://github.com/lutter))
- Update gnulib [\#551](https://github.com/hercules-team/augeas/pull/551) ([lutter](https://github.com/lutter))
- Chrony: add new options [\#549](https://github.com/hercules-team/augeas/pull/549) ([mlichvar](https://github.com/mlichvar))
- new lens: Strongswan [\#543](https://github.com/hercules-team/augeas/pull/543) ([kunkku](https://github.com/kunkku))
- Cleaner error message on regex / [\#541](https://github.com/hercules-team/augeas/pull/541) ([jcpunk](https://github.com/jcpunk))

## [release-1.10.1](https://github.com/hercules-team/augeas/tree/release-1.10.1) (2018-01-29)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.10.0...release-1.10.1)

## [release-1.10.0](https://github.com/hercules-team/augeas/tree/release-1.10.0) (2018-01-25)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.9.0...release-1.10.0)

**Fixed bugs:**

- util.aug empty comments with CRLF line endings [\#161](https://github.com/hercules-team/augeas/issues/161)

**Closed issues:**

- Create new XML file [\#536](https://github.com/hercules-team/augeas/issues/536)
- Build without docs, examples, manuals, and tests [\#535](https://github.com/hercules-team/augeas/issues/535)
- sshd lense multiple parameters not corrected [\#528](https://github.com/hercules-team/augeas/issues/528)
- augtool hangs while loading with lens set to "dist/X.lns" [\#522](https://github.com/hercules-team/augeas/issues/522)
- output of fadot leads to confusion [\#519](https://github.com/hercules-team/augeas/issues/519)
- bug in libfa [\#518](https://github.com/hercules-team/augeas/issues/518)
- nsswitch.conf may contain comment at end of line [\#517](https://github.com/hercules-team/augeas/issues/517)
- ntp.conf: Add "ntpsigndsocket" [\#516](https://github.com/hercules-team/augeas/issues/516)
- 1.9.0 tarball contains tests/lens-\*.sh files [\#511](https://github.com/hercules-team/augeas/issues/511)
- Syslog lens : include support [\#486](https://github.com/hercules-team/augeas/issues/486)

**Merged pull requests:**

- Fstab: allow leading whitespace in lines with spec [\#544](https://github.com/hercules-team/augeas/pull/544) ([ptoscano](https://github.com/ptoscano))
- Add logind.conf to systemd lense [\#539](https://github.com/hercules-team/augeas/pull/539) ([jcpunk](https://github.com/jcpunk))
- httpd: include  /etc/httpd/conf.modules.d [\#537](https://github.com/hercules-team/augeas/pull/537) ([examon](https://github.com/examon))
- Update Makefile.am [\#533](https://github.com/hercules-team/augeas/pull/533) ([joerg-krause](https://github.com/joerg-krause))
- New command line tool 'augmatch' for grepping through files [\#531](https://github.com/hercules-team/augeas/pull/531) ([lutter](https://github.com/lutter))
- Add 'count' command in augtool; at 'not\(\)' operator in path expressions [\#530](https://github.com/hercules-team/augeas/pull/530) ([lutter](https://github.com/lutter))
- Make sure we don't load a module but then later can't find it [\#529](https://github.com/hercules-team/augeas/pull/529) ([lutter](https://github.com/lutter))
- spelling fixes [\#527](https://github.com/hercules-team/augeas/pull/527) ([ka7](https://github.com/ka7))
- Support YAML sequence documents [\#526](https://github.com/hercules-team/augeas/pull/526) ([jayvdb](https://github.com/jayvdb))
- Correct some spelling mistakes [\#523](https://github.com/hercules-team/augeas/pull/523) ([hillu](https://github.com/hillu))
- Fix issue \#161 [\#520](https://github.com/hercules-team/augeas/pull/520) ([lutter](https://github.com/lutter))
- RANCiD lens [\#514](https://github.com/hercules-team/augeas/pull/514) ([bodgit](https://github.com/bodgit))
- Grubenv [\#291](https://github.com/hercules-team/augeas/pull/291) ([omgold](https://github.com/omgold))

## [release-1.9.0](https://github.com/hercules-team/augeas/tree/release-1.9.0) (2017-10-05)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.8.1...release-1.9.0)

**Fixed bugs:**

- Augeas leaves temporary files when the destination is not writable [\#479](https://github.com/hercules-team/augeas/issues/479)
- sshd lense doesn't seem to handle groups with spaces [\#477](https://github.com/hercules-team/augeas/issues/477)
- Httpd: parsing Apache expressions with quoted strings fails [\#470](https://github.com/hercules-team/augeas/issues/470)
- Span indices not updated [\#467](https://github.com/hercules-team/augeas/issues/467)
- Httpd: missing space between quoted directive arguments [\#435](https://github.com/hercules-team/augeas/issues/435)
- Httpd: can't parse RewriteCond with empty string in conditional [\#429](https://github.com/hercules-team/augeas/issues/429)
- Httpd: fails to parse comment ending in backslash [\#423](https://github.com/hercules-team/augeas/issues/423)
- Output from: `augtool print /augeas//error` [\#190](https://github.com/hercules-team/augeas/issues/190)
- httpd lens cannot parse various mod\_security SecRule statements [\#168](https://github.com/hercules-team/augeas/issues/168)

**Closed issues:**

- How to write out a literal backslash character? [\#507](https://github.com/hercules-team/augeas/issues/507)
- 1.8.0 and 1.8.1 are not marked as Github releases [\#497](https://github.com/hercules-team/augeas/issues/497)
- Backslash-escaping in the lens language is incorrect [\#495](https://github.com/hercules-team/augeas/issues/495)
- Yum lens saving excludes incorrectly [\#487](https://github.com/hercules-team/augeas/issues/487)
- Missing new options for Sudoers.aug [\#482](https://github.com/hercules-team/augeas/issues/482)
- fails to parse latest java8u131 java.security file [\#468](https://github.com/hercules-team/augeas/issues/468)
- Differences in result of span pre and post 1.7 release [\#466](https://github.com/hercules-team/augeas/issues/466)
- cannot insert compress node in logrotate.conf [\#181](https://github.com/hercules-team/augeas/issues/181)

**Merged pull requests:**

- Properties: accept DOS line endings [\#510](https://github.com/hercules-team/augeas/pull/510) ([lutter](https://github.com/lutter))
- Sshd: recognize quoted group names with spaces in them [\#509](https://github.com/hercules-team/augeas/pull/509) ([lutter](https://github.com/lutter))
- \* src/put.c \(create\_del\): do not unescape the string before writing it out [\#508](https://github.com/hercules-team/augeas/pull/508) ([lutter](https://github.com/lutter))
- Improvements of syslog and rsyslog [\#506](https://github.com/hercules-team/augeas/pull/506) ([lutter](https://github.com/lutter))
- Exports: allow paths in quotes [\#505](https://github.com/hercules-team/augeas/pull/505) ([lutter](https://github.com/lutter))
- New lense for OZ [\#504](https://github.com/hercules-team/augeas/pull/504) ([jcpunk](https://github.com/jcpunk))
- Fix several httpd issues [\#503](https://github.com/hercules-team/augeas/pull/503) ([lutter](https://github.com/lutter))
- Add seccomp\_sandbox to vsftpd lens [\#501](https://github.com/hercules-team/augeas/pull/501) ([stroebs](https://github.com/stroebs))
- \* .gitignore: ignore examples/dump [\#500](https://github.com/hercules-team/augeas/pull/500) ([ptoscano](https://github.com/ptoscano))
- A few portability fixes [\#499](https://github.com/hercules-team/augeas/pull/499) ([smortex](https://github.com/smortex))
- Fix memory leak when checking version of augparse [\#498](https://github.com/hercules-team/augeas/pull/498) ([qiankehan](https://github.com/qiankehan))
- \* src/lexer.l: properly handle backslash escaping in strings and regex [\#496](https://github.com/hercules-team/augeas/pull/496) ([lutter](https://github.com/lutter))
- Grub: handle top-level "boot" directive [\#494](https://github.com/hercules-team/augeas/pull/494) ([ptoscano](https://github.com/ptoscano))
- When saving to nonwritable file fails, make sure we remove the tempfile [\#493](https://github.com/hercules-team/augeas/pull/493) ([lutter](https://github.com/lutter))
- Cgconfig: allow fperm & dperm in admin & task [\#491](https://github.com/hercules-team/augeas/pull/491) ([ptoscano](https://github.com/ptoscano))
- Address warnings from GCC7 [\#490](https://github.com/hercules-team/augeas/pull/490) ([lutter](https://github.com/lutter))
- Fix sudoers lens: recognize "match\_group\_by\_gid" [\#488](https://github.com/hercules-team/augeas/pull/488) ([ghost](https://github.com/ghost))
- Sferry/solaris build [\#484](https://github.com/hercules-team/augeas/pull/484) ([shawnferry](https://github.com/shawnferry))
- Add FreeBSD paths [\#483](https://github.com/hercules-team/augeas/pull/483) ([bitnexus](https://github.com/bitnexus))
- WIP: speed up iterating and getting information about lots of nodes [\#481](https://github.com/hercules-team/augeas/pull/481) ([lutter](https://github.com/lutter))
- \* src/pathx.c \(parse\_name\): correctly handle trailing whitespace in n… [\#480](https://github.com/hercules-team/augeas/pull/480) ([lutter](https://github.com/lutter))
- update rsyslog lens to includ files in rsyslog.d dir [\#475](https://github.com/hercules-team/augeas/pull/475) ([dallendoug](https://github.com/dallendoug))
- Parse available sites for nginx [\#471](https://github.com/hercules-team/augeas/pull/471) ([thedrow](https://github.com/thedrow))
- Add new nslcd.conf lenses to parse nss-pam-ldapd configuration file [\#463](https://github.com/hercules-team/augeas/pull/463) ([jplana](https://github.com/jplana))
- Xymon.aug: Add group-sorted to definitions [\#462](https://github.com/hercules-team/augeas/pull/462) ([JSP-GIHS](https://github.com/JSP-GIHS))
- Improve put error messages [\#460](https://github.com/hercules-team/augeas/pull/460) ([lutter](https://github.com/lutter))
- Update gnulib to 7b8cbb9 [\#455](https://github.com/hercules-team/augeas/pull/455) ([domcleal](https://github.com/domcleal))
- Add lens for reading termcap-style databases [\#274](https://github.com/hercules-team/augeas/pull/274) ([bodgit](https://github.com/bodgit))

## [release-1.8.1](https://github.com/hercules-team/augeas/tree/release-1.8.1) (2017-08-17)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.8.0...release-1.8.1)

**Closed issues:**

- aug\_source returns the path to the root node and not the filename [\#473](https://github.com/hercules-team/augeas/issues/473)
- Change filter for rsyslog.aug lens [\#465](https://github.com/hercules-team/augeas/issues/465)
- `make check` segmentation fault in test-api on macOS Sierra [\#461](https://github.com/hercules-team/augeas/issues/461)
- Can't install python-augeas [\#457](https://github.com/hercules-team/augeas/issues/457)

## [release-1.8.0](https://github.com/hercules-team/augeas/tree/release-1.8.0) (2017-03-20)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.7.0...release-1.8.0)

**Fixed bugs:**

- Xml.lns support on External Entities [\#142](https://github.com/hercules-team/augeas/issues/142)

**Closed issues:**

- Inserting label to collection of keys is wrongly escaped [\#451](https://github.com/hercules-team/augeas/issues/451)
- How to parse /etc/postfix/aliases [\#447](https://github.com/hercules-team/augeas/issues/447)
- Allow underscores in postfix\_virtual [\#439](https://github.com/hercules-team/augeas/issues/439)
- augtool: out-of-memory [\#438](https://github.com/hercules-team/augeas/issues/438)
- RHEL7.3 krb5-libs package comes with config containing includedir. [\#430](https://github.com/hercules-team/augeas/issues/430)
- Add an API to get the filename without parsing spans [\#384](https://github.com/hercules-team/augeas/issues/384)
- multipath.conf: default sections misses bunch of keywords [\#284](https://github.com/hercules-team/augeas/issues/284)

**Merged pull requests:**

- Fix for rsyslog rainerscript module loading, with options [\#453](https://github.com/hercules-team/augeas/pull/453) ([ghost](https://github.com/ghost))
- Add 'context' and 'info' commands to augtool; reorganize help a little [\#452](https://github.com/hercules-team/augeas/pull/452) ([lutter](https://github.com/lutter))
- Reduce the total amount of memory used by Augeas [\#449](https://github.com/hercules-team/augeas/pull/449) ([lutter](https://github.com/lutter))
- Add a new aug\_source API call and 'source' command in augtool [\#448](https://github.com/hercules-team/augeas/pull/448) ([lutter](https://github.com/lutter))
- Add lens for radicale config file [\#446](https://github.com/hercules-team/augeas/pull/446) ([jvalleroy](https://github.com/jvalleroy))
- Allow underscores in postfix\_virtual [\#445](https://github.com/hercules-team/augeas/pull/445) ([lingfish](https://github.com/lingfish))
- Update interfaces.aug [\#442](https://github.com/hercules-team/augeas/pull/442) ([joerg-krause](https://github.com/joerg-krause))
- Updated for multipath-0.4.9-99.el7. [\#441](https://github.com/hercules-team/augeas/pull/441) ([XMol](https://github.com/XMol))
- \[krb5.aug\] Support realms that start with numbers [\#437](https://github.com/hercules-team/augeas/pull/437) ([mdwheele](https://github.com/mdwheele))
- Chrony: add new directives and options [\#436](https://github.com/hercules-team/augeas/pull/436) ([mlichvar](https://github.com/mlichvar))
- Yum lenses work on DNF, add them to import list [\#434](https://github.com/hercules-team/augeas/pull/434) ([jcpunk](https://github.com/jcpunk))
- php.aug  - Fix php7 FPM support [\#433](https://github.com/hercules-team/augeas/pull/433) ([ddico](https://github.com/ddico))
- Removed trailing space in token in slapd.lens [\#432](https://github.com/hercules-team/augeas/pull/432) ([iavael](https://github.com/iavael))
- Possible fix to \#430 [\#431](https://github.com/hercules-team/augeas/pull/431) ([smithj4](https://github.com/smithj4))
- Xml: process external entities in DOCTYPE declaration [\#426](https://github.com/hercules-team/augeas/pull/426) ([lutter](https://github.com/lutter))

## [release-1.7.0](https://github.com/hercules-team/augeas/tree/release-1.7.0) (2016-11-08)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.6.0...release-1.7.0)

**Fixed bugs:**

- Dhclient: gethostname\(\) function not supported [\#141](https://github.com/hercules-team/augeas/issues/141)
- Can not parse default /etc/default/rmt [\#105](https://github.com/hercules-team/augeas/issues/105)

**Closed issues:**

- defvar not working in command file [\#414](https://github.com/hercules-team/augeas/issues/414)
- Splunk lens does not work with modern Universal Forwarder [\#407](https://github.com/hercules-team/augeas/issues/407)
- Out of bounds heap read in skel\_instance\_of / put.c [\#398](https://github.com/hercules-team/augeas/issues/398)
- Double free with AUG\_ENABLE\_SPAN in free\_tree\_node [\#397](https://github.com/hercules-team/augeas/issues/397)
- CSV lens fails: Failed to compile comment [\#396](https://github.com/hercules-team/augeas/issues/396)
- regex "multiple any or newline class" does not match multiline comments [\#394](https://github.com/hercules-team/augeas/issues/394)
- /etc/sysconfig/network parsing fails in openSUSE [\#254](https://github.com/hercules-team/augeas/issues/254)

**Merged pull requests:**

- Refine handling of multiple transforms handling the same file [\#425](https://github.com/hercules-team/augeas/pull/425) ([lutter](https://github.com/lutter))
- Properly check composite regular expressions for syntax errors [\#422](https://github.com/hercules-team/augeas/pull/422) ([lutter](https://github.com/lutter))
- Cron\_User: new lens for user crontab files [\#421](https://github.com/hercules-team/augeas/pull/421) ([lutter](https://github.com/lutter))
- Refactorings/stability fixes for recursive parsing [\#410](https://github.com/hercules-team/augeas/pull/410) ([lutter](https://github.com/lutter))
- Various fixes based on coverity errors [\#405](https://github.com/hercules-team/augeas/pull/405) ([lutter](https://github.com/lutter))
- Fix memory corruption when using spans with a recursive lens \(bug \#397\) [\#403](https://github.com/hercules-team/augeas/pull/403) ([lutter](https://github.com/lutter))
- Fix illegal memory access in skel\_instane\_of [\#402](https://github.com/hercules-team/augeas/pull/402) ([lutter](https://github.com/lutter))
- Update gnulib to 6fafd688 [\#400](https://github.com/hercules-team/augeas/pull/400) ([lutter](https://github.com/lutter))
- Reflect Ubuntu 16.04 location of php.ini [\#399](https://github.com/hercules-team/augeas/pull/399) ([daMihe](https://github.com/daMihe))

## [release-1.6.0](https://github.com/hercules-team/augeas/tree/release-1.6.0) (2016-08-05)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.5.0...release-1.6.0)

**Closed issues:**

- Shellvars seems to fail when a '.' is used in the key. [\#383](https://github.com/hercules-team/augeas/issues/383)
- Allow to build without libxml2 [\#382](https://github.com/hercules-team/augeas/issues/382)
- pam lens: arguments should be separated by spaces not tabs? [\#236](https://github.com/hercules-team/augeas/issues/236)

**Merged pull requests:**

- DEREF\_OF\_NULL [\#389](https://github.com/hercules-team/augeas/pull/389) ([g-coder](https://github.com/g-coder))
- fix restrict to allow also -4 and also fix save/store ability [\#386](https://github.com/hercules-team/augeas/pull/386) ([jreidinger](https://github.com/jreidinger))
- Add lens to parse postfix password maps [\#380](https://github.com/hercules-team/augeas/pull/380) ([spacedog](https://github.com/spacedog))
- Recent ntp.conf allows for 'pool' instead of 'server' et al [\#378](https://github.com/hercules-team/augeas/pull/378) ([ghost](https://github.com/ghost))

## [release-1.5.0](https://github.com/hercules-team/augeas/tree/release-1.5.0) (2016-05-11)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.4.0...release-1.5.0)

**Fixed bugs:**

- /etc/apt/apt.conf.d/01autoremove cannot be parsed by aptconf lens [\#341](https://github.com/hercules-team/augeas/issues/341)
- httpd lens fails on missing double quote to end directive argument [\#330](https://github.com/hercules-team/augeas/issues/330)
- Httpd.lns cannot parse file without ending EOL [\#312](https://github.com/hercules-team/augeas/issues/312)
- httpd lens fails on backslashes in directive regular expressions [\#307](https://github.com/hercules-team/augeas/issues/307)
- Sudoers lens doesn't support `!` [\#262](https://github.com/hercules-team/augeas/issues/262)
- Typo on the FAQ page: "ditributed" [\#234](https://github.com/hercules-team/augeas/issues/234)
- Aptsources lens can't handle \[ options \] in /etc/apt/sources.list [\#295](https://github.com/hercules-team/augeas/pull/295) ([SunilMohanAdapa](https://github.com/SunilMohanAdapa))
- shellvars lens can't handle "expression1 && expression2" or "expression1 || expression2" [\#215](https://github.com/hercules-team/augeas/pull/215) ([chelli](https://github.com/chelli))

**Closed issues:**

- Link to Stock Lens Hosts not working [\#373](https://github.com/hercules-team/augeas/issues/373)
- aug\_get may not set value to NULL under certain conditions [\#372](https://github.com/hercules-team/augeas/issues/372)
- logrotate lens does not support dateyesterday [\#367](https://github.com/hercules-team/augeas/issues/367)
- Unable to load single lens [\#360](https://github.com/hercules-team/augeas/issues/360)
- Commenting out a line [\#355](https://github.com/hercules-team/augeas/issues/355)
- mysql lenses [\#353](https://github.com/hercules-team/augeas/issues/353)
- support for negative values in chrony lens [\#347](https://github.com/hercules-team/augeas/issues/347)
- Please use github.com/coreutils/gnulib [\#345](https://github.com/hercules-team/augeas/issues/345)
- Allow to parse files in arbitrary paths [\#337](https://github.com/hercules-team/augeas/issues/337)
- Cannot install 1.4 from the PPA due to build errors [\#336](https://github.com/hercules-team/augeas/issues/336)
- Escaped dots in regexps crashing the httpd lens [\#324](https://github.com/hercules-team/augeas/issues/324)
- double free when deleting nested XML elements using // \(descendant-or-self\) [\#319](https://github.com/hercules-team/augeas/issues/319)
- RPM versions available through repos.. [\#314](https://github.com/hercules-team/augeas/issues/314)
- aug\_save\(\) fails with /augeas//errors pointing to multiple unrelated files [\#308](https://github.com/hercules-team/augeas/issues/308)
- \[RFE\] entries in sshd Match blocks indented [\#286](https://github.com/hercules-team/augeas/issues/286)
- Difficult with current API to insert node and refer to it afterwards [\#285](https://github.com/hercules-team/augeas/issues/285)
- Spacevars lens fails on empty values [\#279](https://github.com/hercules-team/augeas/issues/279)
- Support for termcap-style capability databases [\#271](https://github.com/hercules-team/augeas/issues/271)
- On OSX libedit lacks rl\_crlf and rl\_replace\_line [\#256](https://github.com/hercules-team/augeas/issues/256)
- \[RFE\] Grub 2 Lense [\#247](https://github.com/hercules-team/augeas/issues/247)
- sshd lens may be incorrect [\#205](https://github.com/hercules-team/augeas/issues/205)

**Merged pull requests:**

- Chrony: add new options [\#371](https://github.com/hercules-team/augeas/pull/371) ([mlichvar](https://github.com/mlichvar))
- Release 1.5.0 [\#369](https://github.com/hercules-team/augeas/pull/369) ([lutter](https://github.com/lutter))
- logrotate: support dateyesterday option \(GH \#367\) [\#368](https://github.com/hercules-team/augeas/pull/368) ([chris-reeves](https://github.com/chris-reeves))
- smbusers: add support for ; comments [\#364](https://github.com/hercules-team/augeas/pull/364) ([cbosdo](https://github.com/cbosdo))
- Inputrc fixes [\#359](https://github.com/hercules-team/augeas/pull/359) ([cbosdo](https://github.com/cbosdo))
- Fix memory corruption in format\_union\_atype [\#350](https://github.com/hercules-team/augeas/pull/350) ([lutter](https://github.com/lutter))
- Improve chrony lens [\#348](https://github.com/hercules-team/augeas/pull/348) ([mlichvar](https://github.com/mlichvar))
- AptConf: Allow empty hash-style comments [\#342](https://github.com/hercules-team/augeas/pull/342) ([raphink](https://github.com/raphink))
- shelllvars lens fails for default configuration of aufs [\#339](https://github.com/hercules-team/augeas/pull/339) ([thedrow](https://github.com/thedrow))
- nginx lense can't parse example from linguist [\#335](https://github.com/hercules-team/augeas/pull/335) ([thedrow](https://github.com/thedrow))
- Httpd: parse mismatching case in opening/closing tags [\#329](https://github.com/hercules-team/augeas/pull/329) ([domcleal](https://github.com/domcleal))
- Skip hidden tree nodes in path expressions [\#323](https://github.com/hercules-team/augeas/pull/323) ([lutter](https://github.com/lutter))
- aug\_rm: fix segfault when deleting a tree and one of its ancestors [\#320](https://github.com/hercules-team/augeas/pull/320) ([lutter](https://github.com/lutter))
- Ssh: add support for GlobalKnownHostsFile [\#316](https://github.com/hercules-team/augeas/pull/316) ([raphink](https://github.com/raphink))
- Invalid rules to parse /etc/network/interfaces [\#306](https://github.com/hercules-team/augeas/pull/306) ([b-s-a](https://github.com/b-s-a))
- Print timing information for every augtool command [\#305](https://github.com/hercules-team/augeas/pull/305) ([lutter](https://github.com/lutter))
- Improve performance of some path expressions [\#303](https://github.com/hercules-team/augeas/pull/303) ([lutter](https://github.com/lutter))
- Aptsources lens can't handle cdrom:// URLs properly [\#296](https://github.com/hercules-team/augeas/pull/296) ([SunilMohanAdapa](https://github.com/SunilMohanAdapa))
- Dhclient: avoid put ambiguity for node without value [\#294](https://github.com/hercules-team/augeas/pull/294) ([raphink](https://github.com/raphink))
- ssh lense: adding new keywords... [\#290](https://github.com/hercules-team/augeas/pull/290) ([omgold](https://github.com/omgold))
- multipath.conf lense: added bunch of missing keywords [\#289](https://github.com/hercules-team/augeas/pull/289) ([omgold](https://github.com/omgold))
- reprepro\_uploader lens doesn't support groups [\#283](https://github.com/hercules-team/augeas/pull/283) ([kumy](https://github.com/kumy))
- Rsyslog: Improve property filter parsing. [\#282](https://github.com/hercules-team/augeas/pull/282) ([gasmith-nutanix](https://github.com/gasmith-nutanix))
- openvpn: added all options available in OpenVPN2.3 and all tests [\#278](https://github.com/hercules-team/augeas/pull/278) ([dafugg](https://github.com/dafugg))
- CSV: New lens to parse CSV files [\#275](https://github.com/hercules-team/augeas/pull/275) ([raphink](https://github.com/raphink))
- Xml: Accept empty document [\#255](https://github.com/hercules-team/augeas/pull/255) ([raphink](https://github.com/raphink))
- Chrony: update for chrony 2.0 [\#248](https://github.com/hercules-team/augeas/pull/248) ([mlichvar](https://github.com/mlichvar))
- Shellvars: Allow \(almost\) any command [\#119](https://github.com/hercules-team/augeas/pull/119) ([raphink](https://github.com/raphink))

## [release-1.4.0](https://github.com/hercules-team/augeas/tree/release-1.4.0) (2015-06-02)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.3.0...release-1.4.0)

**Fixed bugs:**

- nginx lens doesn't support recursive blocks [\#179](https://github.com/hercules-team/augeas/issues/179)
- Segmentation fault in aug\_save if file not writeable [\#178](https://github.com/hercules-team/augeas/issues/178)
- Cannot have more than one comment after start of section in Httpd.lns [\#140](https://github.com/hercules-team/augeas/issues/140)

**Closed issues:**

- test-put-symlink-augsave.sh failure [\#242](https://github.com/hercules-team/augeas/issues/242)
- Redis lens doesn't allow setting slaveof no one [\#232](https://github.com/hercules-team/augeas/issues/232)
- grub.lns unable to handle password entries in grub.conf for specific titles [\#229](https://github.com/hercules-team/augeas/issues/229)
- network/network2 API docs empty [\#227](https://github.com/hercules-team/augeas/issues/227)
- Properties lens doesn't like multi-line where first line has no value [\#224](https://github.com/hercules-team/augeas/issues/224)
- Web page about Hosts lens returns a 404 error [\#222](https://github.com/hercules-team/augeas/issues/222)
- logrotate lens does not support dateformat [\#217](https://github.com/hercules-team/augeas/issues/217)
- sshd lense Multiple ListenAddress [\#206](https://github.com/hercules-team/augeas/issues/206)
- does augeas lense test clear update-notifier/updates-available? [\#204](https://github.com/hercules-team/augeas/issues/204)
- Shellvars.lns can't insert comment if the first line a newline [\#202](https://github.com/hercules-team/augeas/issues/202)
- configure? [\#201](https://github.com/hercules-team/augeas/issues/201)
- Redis lense doesn't support empty values [\#195](https://github.com/hercules-team/augeas/issues/195)
- Augeas 1.3.0 Httpd lens broke parsing RedHat's ssl.conf [\#191](https://github.com/hercules-team/augeas/issues/191)
- sudo lens should support + in usernames [\#189](https://github.com/hercules-team/augeas/issues/189)
- Lens for /etc/postfix/sasl/smtpd.conf [\#182](https://github.com/hercules-team/augeas/issues/182)
- iscsid.conf parsing [\#174](https://github.com/hercules-team/augeas/issues/174)
- non-zero exit code on augtool match failure [\#170](https://github.com/hercules-team/augeas/issues/170)
- Provide full set of XPath operations [\#164](https://github.com/hercules-team/augeas/issues/164)
- logrotate lens fails to parse when no-space between file and { [\#123](https://github.com/hercules-team/augeas/issues/123)
- \[RFE\] /etc/tuned/tuned-main.conf can be parsed by shellvars.aug [\#121](https://github.com/hercules-team/augeas/issues/121)
- Undefined refernce to YYID [\#88](https://github.com/hercules-team/augeas/issues/88)
- Add an `aug\_lua` call [\#71](https://github.com/hercules-team/augeas/issues/71)

**Merged pull requests:**

- Add /etc/puppetlabs/mcollective to mcollective lens [\#240](https://github.com/hercules-team/augeas/pull/240) ([MikaelSmith](https://github.com/MikaelSmith))
- Nginx: rework to support a much broader set of valid configurations [\#225](https://github.com/hercules-team/augeas/pull/225) ([lutter](https://github.com/lutter))
- Httpd: error parsing comments [\#220](https://github.com/hercules-team/augeas/pull/220) ([jjlin](https://github.com/jjlin))
- Added support for conf-available apache config file location [\#208](https://github.com/hercules-team/augeas/pull/208) ([annunaki2k2](https://github.com/annunaki2k2))
- exports: add bracket support to the supported machine name formats. [\#199](https://github.com/hercules-team/augeas/pull/199) ([vdesjardins](https://github.com/vdesjardins))
- Puppetfile: New lens to parse the Puppetfile format [\#186](https://github.com/hercules-team/augeas/pull/186) ([raphink](https://github.com/raphink))
- NagiosCfg lens broken for /etc/nagios.cfg on CentOS 6 due to spaces [\#177](https://github.com/hercules-team/augeas/pull/177) ([nguillaumin](https://github.com/nguillaumin))

## [release-1.3.0](https://github.com/hercules-team/augeas/tree/release-1.3.0) (2014-11-07)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.2.0...release-1.3.0)

**Fixed bugs:**

- Can't escape certain special characters in XML lens [\#145](https://github.com/hercules-team/augeas/issues/145)
- Redis.conf empty string value [\#115](https://github.com/hercules-team/augeas/issues/115)

**Closed issues:**

- augeas ruby API rpm for CentOs/Redhat should not require gcc/devel tools to install in ruby [\#166](https://github.com/hercules-team/augeas/issues/166)
- Cannot add options to autofs entry using automounter.aug unless at least one option already exists [\#163](https://github.com/hercules-team/augeas/issues/163)
- Broken/missing documentation for stock lenses [\#157](https://github.com/hercules-team/augeas/issues/157)
- Httpd.aug: mod\_version operators are not supported [\#154](https://github.com/hercules-team/augeas/issues/154)
- Puppet.lns chokes on single-character configurations [\#151](https://github.com/hercules-team/augeas/issues/151)
- xml.lens parse failure DOCTYPE + ":" [\#143](https://github.com/hercules-team/augeas/issues/143)
- nrpe lens does not allow forward slash in command name [\#128](https://github.com/hercules-team/augeas/issues/128)
- Link to Properties lens is broken [\#116](https://github.com/hercules-team/augeas/issues/116)
- Can not parse the configure files which are used to report a bug to bugzilla [\#107](https://github.com/hercules-team/augeas/issues/107)
- Can not parse default /etc/httpd/conf.d/mod\_security.conf  [\#104](https://github.com/hercules-team/augeas/issues/104)
- logrotate cannot save, "Failed to match" [\#103](https://github.com/hercules-team/augeas/issues/103)
- Can't configure /etc/cups/cups-pdf.conf [\#102](https://github.com/hercules-team/augeas/issues/102)
- ld.so.conf.d errors on a vanilla kernel-2.6.x.el6.x86\_64.conf [\#100](https://github.com/hercules-team/augeas/issues/100)
- Missing support for "append" and "prepend" in dhclient.conf [\#95](https://github.com/hercules-team/augeas/issues/95)
- Desktop.aug, cannot parse Name\[sr@latin\] = ... \(Serbian AFAIK\) [\#92](https://github.com/hercules-team/augeas/issues/92)
- Error parsing iptables file [\#83](https://github.com/hercules-team/augeas/issues/83)
- xml parse fails [\#80](https://github.com/hercules-team/augeas/issues/80)
- cp is missing from the man pages [\#78](https://github.com/hercules-team/augeas/issues/78)
- requesting a lense for chrony [\#76](https://github.com/hercules-team/augeas/issues/76)
- sshd Match Address \*,!10.0.0.0/24 strangeness [\#75](https://github.com/hercules-team/augeas/issues/75)
- FreeBSD 10+ && syslog.aug [\#65](https://github.com/hercules-team/augeas/issues/65)
- Master branch doesn't compile [\#56](https://github.com/hercules-team/augeas/issues/56)
- XML prolog with single quotes [\#20](https://github.com/hercules-team/augeas/issues/20)

**Merged pull requests:**

- augtool: on interrupt, cancel current line instead of exiting [\#207](https://github.com/hercules-team/augeas/pull/207) ([jjlin](https://github.com/jjlin))
- Added config.d files to the nginx lens filter [\#171](https://github.com/hercules-team/augeas/pull/171) ([kenaniah](https://github.com/kenaniah))
- Systemd: parse environment variables where only value is quoted [\#156](https://github.com/hercules-team/augeas/pull/156) ([domcleal](https://github.com/domcleal))
- add support for mtu relevant parameters fragment and mssfix [\#133](https://github.com/hercules-team/augeas/pull/133) ([fonk](https://github.com/fonk))
- \* src/augrun.c: remove unused "filename" argument from dump-xml command [\#130](https://github.com/hercules-team/augeas/pull/130) ([domcleal](https://github.com/domcleal))
- Error loading /etc/apache2/envvars with Shellvars lense [\#118](https://github.com/hercules-team/augeas/pull/118) ([mcanevet](https://github.com/mcanevet))
- add missing } in file path for nagios/icinga objects [\#109](https://github.com/hercules-team/augeas/pull/109) ([saimonn](https://github.com/saimonn))
- UpdateDB: New lens to parse /etc/updatedb.conf [\#98](https://github.com/hercules-team/augeas/pull/98) ([raphink](https://github.com/raphink))

## [release-1.1.0](https://github.com/hercules-team/augeas/tree/release-1.1.0) (2013-06-14)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-1.0.0...release-1.1.0)

  - General changes/additions
    * Handle files with special characters in their name, bug #343
    * Fix type error in composition ('f; g') of functions, bug #328
    * Improve detection of version script; make build work on Illumos with
      GBU ld (Igor Pashev)
    * augparse: add --trace option to print filenames of all modules being
      loaded
    * Various lens documentation improvements (Jasper Lievisse Adriaanse)
  - Lens changes/additions
    * ActiveMQ_*: new lens for ActiveMQ/JBoss A-MQ (Brian Harrington)
    * AptCacherNGSecurity: new lens for /etc/apt-cacher-ng/security.conf
      (Erik Anderson)
    * Automaster: accept spaces between options
    * BBHosts: support more flags and downtime feature (Mathieu Alorent)
    * Bootconf: new lens for OpenBSD's /etc/boot.conf (Jasper Lievisse Adriaanse)
    * Desktop: Support dos eol
    * Dhclient: read /etc/dhclient.conf used in OpenBSD (Jasper Lievisse Adriaanse)
    * Dovecot: New lens for dovecot configurations (Serge Smetana)
    * Fai_Diskconfig: Optimize some regexps
    * Fonts: exclude all README files (Jasper Lievisse Adriaanse)
    * Inetd: support IPv6 addresses, bug #320
    * IniFile: Add lns_loose and lns_loose_multiline definitions
               Support smart quotes
      Warning: Smart quotes support means users should not add
               escaped double quotes themselves. Tests need to be fixed
               also.
               Use standard Util.comment_generic and Util.empty_generic
      Warning: Existing lens tests must be adapted to use standard
               comments and empty lines
               Allow spaces in entry_multiline* values
               Add entry_generic and entry_multiline_generic
               Add empty_generic and empty_noindent
               Let multiline values begin with a single newline
               Support dos eol
      Warning: Support for dos eol means existing lenses usually
               need to be adapted to exclude \r as well as \n.
    * IPRoute2: Support for iproute2 files (Davide Guerri)
    * JaaS: lens for the Java Authentication and Authorization Service
            (Simon Vocella)
    * JettyRealm: new lens for jetty-realm.properties (Brian Harrington)
    * JMXAccess, JMXPassword: new lenses for ActiveMQ's JMX files
      (Brian Harrington)
    * Krb5: Use standard comments and empty lines
            Support dos eol
            Improve performance
            Accept pkinit_anchors (Andrew Anderson)
    * Lightdm: Use standard comments and empty lines
    * LVM: New lens for LVM metadata (Gabriel)
    * Mdadm_conf: optimize some regexps
    * MongoDBServer: new lens (Brian Harrington)
    * Monit: also load /etc/monitrc (Jasper Lievisse Adriaanse)
    * MySQL: Use standard comments and empty lines
             Support dos eol
    * NagiosCfg: handle Icinga and resources.cfg (Jasper Lievisse Adriaanse)
    * Nrpe: accept any config option rather than predefined list (Gonzalo
            Servat); optimize some regexps
    * Ntpd: new lense for OpenNTPD config (Jasper Lievisse Adriaanse)
    * Odbc: Use standard comments and empty lines
    * Openshift_*: new lenses for Openshift support (Brian Harrington)
    * Quote: allow multiple spaces in quote_spaces; improve docs
    * Passwd: allow period in user names in spec, bug #337; allow overrides
              in nisentry
    * PHP: Support smart quotes
           Use standard comments and empty lines
           Load /etc/php*/fpm/pool.d/*.conf (Enrico Stahn)
    * Postfix_master: allow [] in words, bug #345
    * Resolv: support 'lookup' and 'family' key words, bug #320
              (Jasper Lievisse Adriaanse))
    * Rsyslog: support :omusrmsg: list of users in actions
    * RX: add CR to RX.space_in
    * Samba: Use standard comments and empty lines
             Support dos eol
    * Schroot: Support smart quotes
    * Services: support port ranges (Branan Purvine-Riley)
    * Shellvars: optimize some regexps; reinstate /etc/sysconfig/network,
      fixes bug #330, RHBZ#904222, RHBZ#920609; parse /etc/rc.conf.local
      from OpenBSD
    * Sip_Conf: New lens for sip.conf configurations (Rob Tucker)
    * Splunk: new lens (Tim Brigham)
    * Subversion: Support smart quotes
                  Use standard comments and empty lines
                  Use IniFile.entry_multiline_generic
                  Use IniFile.empty_noindent
                  Support dos eol
    * Sudoers: allow user aliases in specs
    * Sysctl: exclude README file
    * Systemd: Support smart quotes; allow backslashes in values
    * Xinetd: handle missing values in list, bug #307
    * Xorg: allow 'Screen' in Device section, bug #344
    * Yum: Support dos eol, optimize some regexps


## [release-1.0.0](https://github.com/hercules-team/augeas/tree/release-1.0.0) (2012-12-21)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.10.0...release-1.0.0)

  - General changes/additions
    * fix missing requirement on libxml2 in pkg-config
    * do not replace pathin with '/*' unless the length is 0
      or pathin is '/', bug #239
    * create context path if it doesn't exist
    * add missing argument to escape() to fix build on solaris, bug #242
    * fix fatest linking with libfa
    * don't use variables uninitialized upon error (Jim Meyering)
    * bootstrap: add strchrnul gnulib module (for Solaris)
    * remove Linux-isms so tests can run on Solaris
    * re-open rl_outstream/stdout only when stdout isn't a tty
      (fixes -e -i); use /dev/tty instead of /dev/stdout when re-opening
      to prevent permission errors, bug #241
    * take root into account for excludes, bug #252
    * fix different errors for parse and put failure
    * fix various memory leaks
    * add leak test
    * allocate exception instead of static const value
    * improve aug_srun quoting to permit concatenation and better detect
      bad quoting
    * rename echo to echo_commands to fix differing types reported
      with Solaris linker (Tim Mooney), bug #262
    * fix excl filters that only specify a filename or wildcard
    * make sure reloading discards changes after save with mode 'newfile'
    * remove loop that added a second iteration around children of /files,
      causing multiple saves in newfile and noop modes when editing under
      /files/boot, bug #264
    * support \t and \n in aug_srun tokens, bug #265
    * compile_exp: don't return an uninitialized pointer upon failure
      (Jim Meyering)
    * include 'extern "C"' wrapper for C++, bug #272 (Igor Pashev)
    * src/try: don't overwrite gdbcmds.txt if it exists
    * fix behavior of set with empty strings
    * allow running individual tests with test-run
    * test-augtool.sh: escape all possible regular expressions before
      they are sent to sed (Micah Anderson)
    * add new print_tree primitive
    * fix bad memory access in regexp.c
    * case-insensitive regexps: fix a problem with number of groups
    * prevent symlink attacks via .augnew during saving,
      RedHat bug #772257, CVE-2012-0786
    * prevent cross-mountpoint attacks via .augsave during saving,
      RedHat bug #772261, CVE-2012-0787
    * add bundled (gnulib) provides in augeas.spec.in, RedHat bug #821745
    * make Travis CI builds
    * src/transform.c (xread_file): catch failed fopen, e.g. EACCES
    * src/augrun.c (cmd_retrieve_help): tidy line wrapping
    * make get_square case insensitive on the ending key
    * escape double quotes when dumping regexp
    * use constants for "lens", "incl" and "excl"
    * src/transform.c (filter_generate): remove duplicate variable assignment
    * src/jmt.c (parse_add_item): ensure return is defined on goto error
    * src/transform.c (transform_save): chmod after creating new files to
      permissions implied by the umask
    * ignore eclipse settings directory
    * fix memory leak in dbg_visit
    * build AST while visiting the parse v2
    * rewrite square lens to be more generic, allowing e.g. square quoting
    * tests/modules/fail_shadow_union.aug: fix unintended test failure
    * src/syntax.c (compile_test): print which test failed when missing
      exception
    * libfa (fa_enumerate): new function
    * use precise ctype of a square lens if it is indeed regula
    * square: properly handle first lens matching empty string
    * square lens: correctly process skeletons during put
    * src/pathx.c: disallow ',' in names in path expressions
    * src/pathx.c: match functions by name and arity
    * src/pathx.c: pass the number of actual arguments to the func
      implementation
    * correctly parse escaped string literals in vim syntax file (Domen Ko¿ar)
  - API changes/additions
    * add aug_text_store to write string to tree
    * add aug_text_retrieve to turn tree into text
    * add aug_rename to rename node labels without moving them in the tree
    * add aug_transform to allow specifying transforms
    * add aug_label to retrieve the label from a path
  - Augtool/aug_srun changes/additions
    * add "touch" command to create node if it doesn't exist, bug #276
    * make <VALUE> argument to "set" and "setm" optional, bug #276
    * add "text_store" and "text_retrieve" commands
    * add "rename" command
    * add "transform" command and "-t|--transform" option
    * add "label" command
    * arrange commands in groups for better help
    * man/augtool.pod: update mentions of default load path
    * fix exit code when using autosave
    * output errors when sending a command as argument
    * honor --echo when sending a command as argument
  - XPath changes/additions
    * add support for an 'i' flag in regexp builtin function
  - Lens changes/additions
    * Aliases: commands can be fully enclosed in quotes, bug #229
    * Anacron: new lens for /etc/anacrontab
    * Apt_Update_Manager: new lens for /etc/update-manager
    * AptPreferences: #comments are accepted within entries
    * AuthorizedKeys: new lens for SSH's authorized_keys
    * AutoMaster: new lens for auto.master files
    * AutoMounter: new lens for automounter maps (/etc/auto.*)
    * Avahi: new lens for /etc/avahi/avahi-daemon.conf (Athir Nuaimi)
    * Build: add blocks
    * Cachefilesd: new lens for /etc/cachefilesd.conf (Pat Riehecky)
    * Carbon: new lens for /etc/carbon files (Marc Fournier)
    * Cgconfig: add space between group and id (Filip Andres)
    * Channels: new lens for channels.conf
    * Collectd: new lens for /etc/collectd.conf
    * Cron: exclude cron allow/deny files;
            optimize typechecking;
            records can be prefixed by '-' (Michal Filka)
    * CronAllow: new lens for cron/at allow/deny files
    * Cups: new lens for Cups files
    * Cyrus_Imapd: new lens for /etc/imapd.conf, bug #296 (Jeroen van Meeuwen)
    * Debctrl: fixed package paragraph keywords, allow variables
               for version numbers in dependency lists,
               allow DM-Upload-Allowed keyword, Debian bug #650887;
               allow control extensions for Python packages, bug #267
    * Dhcpd: fix primary statement arguments, bug #293;
             use the Quote module to manage quoted values;
             force double quotes for filename attribute, bug #311
    * Dput: use Sys.getenv("HOME")
    * Erlang: new generic lens to build Erlang config lenses
    * Fonts: new lens for /etc/fonts files
    * Fstab: handle options with empty values ("password=");
             make options field optional;
             allow end-of-line comment
    * Fuse: new lens for fuse.conf
    * Gdm: include /etc/gdm/custom.conf
    * Grub: parse "password --encrypted" properly, bug #250;
            optimize typechecking;
            add /boot/grub/grub.conf to transform (Josh Kayse)
    * GtkBookmarks: new lens for $HOME/.gtk-bookmarks
    * Hosts_Access: add netmask;
                    permit more client list formats
                    (whitespace separated lists, @netgroups,
                    IPv6 hosts, inc. zone indices,
                    paths to lists of clients, wildcards,
                    hosts_options), bug #256
    * Htpasswd: new lens for htpasswd/rsyncd.secret files (Marc Fournier)
    * Httpd: support DOS eol
    * IniFile: allow # and ; in quoted values, bug #243;
               add entry_list and entry_list_nocomment
    * Inputrc: new lens for /etc/inputrc
    * Iptables: test that blank lines are accepted (Terence Haddock)
    * Json: allow JSON number literals to be followed by whitespace;
            correctly parse empty object and arrays (Lubomir Rintel)
    * Keepalived: various improvements, optimize typechecking
    * Krb5: handle host{} sections in v4_name_convert;
            support ticket_lifetime;
            handle multiple arguments to *_enctypes (Pat Riehecky);
            better whitespace and semicolon comment support
    * Ldif: new lens to read LDIF files per RFC2849
    * Ldso: new lens for ld.so.conf files
    * Lightdm: new lens for /etc/lightdm/*.conf, bug #302 (David Salmen)
    * Logrotate: rewrite with Build, Rx, and Sep;
                 add su logrotate.conf option (Luc Didry);
                 accept integers prefixed by a sign (Michal Filka)
    * Logwatch: new lens for /etc/logwatch/conf/logwatch.conf (Francois Lebel)
    * Mcollective: new lens for Mcollective files (Marc Fournier)
    * Memcached: new lens for /etc/memcached.conf (Marc Fournier)
    * Mdadm_conf: include /etc/mdadm/mdadm.conf
    * Mke2fs: add support for default_mntopts, enable_periodic_fsck,
              and auto_64-bit_support
    * Modprobe: support softdep command, Debian bug #641813;
                allow spaces around '=' in option, RedHat bug #826752;
                support multiline split commands, Ubuntu bug #1054306;
                revert inner lens name change, fixes Modules_conf
    * Modules: define own entry regexp as referenced Modprobe inner lens
               doesn't match file format
    * Multipath: allow devices to override defaults, bug #278 (Jacob M. McCann)
    * NagiosCfg: support syntax for commands.cfg and resource.cfg
    * Netmask: new lens for /etc/inet/netmasks on Solaris
    * NetworkManager: new lens for NetworkManager files
    * Networks: handle multiple missing network octets,
                fix sequencing of aliases
    * Nginx: new lens for /etc/nginx/nginx.conf (Ian Berry)
    * Nsswitch: add passwd_compat, group_compat and shadow_compat
                GNU extensions (Travis Groth);
                remove long list of databases, match by regexp
    * Ntp: allow deprecated 'authenticate' setting;
           add tos directive, bug #297 (Jacob M. McCann)
    * OpenVPN: use the Quote module to manage quoted values
    * Pam: allow uppercase chars in 'types', remove /etc/pam.conf from filter;
           ignore allow.pamlist;
           exclude /etc/pam.d/README, bug #255
    * PamConf: new lens for /etc/pam.conf
    * Passwd: allow asterisk in password field, bug #255
    * Pg_Hba: support multiple options, bug #313;
              add a path to pg_hba.aug, bug #281 (Marc Fournier)
    * Php: support include() statements
    * Phpvars: map arrays with @arraykey subnodes to make working paths;
               support classes and public/var values, bug #299 (aheahe)
    * Postfix_Transport: new lens for Postfix transport files;
               allow host:port and [host]:port syntaxes, bug #303
    * Postfix_Virtual: new lens for Postfix virtual files
    * Postgresql: new lens for postgresql.conf;
                  properly support quotes, bug #317
    * Properties: improve handling of whitespace, empty props, and underscores
                  in keys (Brett Porter, Carlos Sanchez)
    * Protocols: new lens for /etc/protocols
    * Puppet: add /usr/local/etc/puppet paths (Tim Bishop)
    * Puppet_Auth: new lens for /etc/puppet/auth.conf
    * PuppetFileserver: add /usr/local/etc/puppet paths (Tim Bishop)
    * PythonPaste: new lens for Python Paste configs (Dan Prince)
    * Qpid: new lens to read Apache Qpid daemon/client configs (Andrew Replogle)
    * Quote: new generic lens to manage quoted values using square lenses
    * Rabbitmq: new lens for /etc/rabbitmq/rabbitmq.config
    * Redis: new lens for /etc/redis/redis.conf (Marc Fournier)
    * Resolv: add in single-request-reopen (Erinn Looney-Triggs)
    * Rsyslog: new lens for rsyslog files
    * Rx: add continous lines (cl, cl_or_space, cl_or_opt_space)
    * Sep: add space_equal;
           add continous lines (cl_or_space, cl_or_opt_space)
    * Shellvars: support @return;
                 allow multiple elif statements;
                 parse functions;
                 add more includes;
                 autoload some SuSe and RHN specific files (Duncan Mac-Vicar P);
                 add BSD's /etc/rc.conf, bug #255;
                 remove non-shell files, up2date now has a lens,
                 move updatedb.conf to Simplevars;
                 include /etc/{default,sysconfig}/* and /etc/selinux/config;
                 add systemd's /etc/os-release file;
                 exclude bootloader from shellvars (Duncan Mac-Vicar P);
                 handle bash's implicit concatenation of quoted strings
                 (Michal Filka);
                 exclude /etc/default/whoopsie;
                 fix ambiguity by making semi-colons illegal in bquot
                 and arrays;
                 add lns_norec to check for ambiguities;
                 allow newlines in quoted values;
                 allow semi-colons in bquot and dollar_assign;
                 make end-of-line comments begin with a space;
                 allow double backquoted values;
                 support matching keys in var_action, bug #290;
                 fix empty lines after comments;
                 add shift and exit builtins, with optional args;
                 allow double quotes around variables in case statements;
                 fix empty comments;
                 add locale.conf, vconsole.conf systemd configs,
                 RedHat bug #881841
    * Shells: permit same-line comments
    * Simplelines: new lens for simple lines files
    * Simplevars: new lens for simple key/value, non shellvars files
    * Smbusers: new lens for Samba's smbusers
    * Sssd: new lens for sssd.conf (Erinn Looney-Triggs)
    * Ssh: use Sys.getenv('HOME') in filter instead of ~ since it's not
           expanded (Luc Didry)
    * Sshd: permit hyphens in subsystem names
    * Subversion: new lens for /etc/subversion files
    * Sudoers: optimize typechecking;
               allow = in commands (but force ! or / as first character
               if not an alias);
               allow commands without full path if they begin with a lowcase
               letter;
               allow "!" as a type of Defaults entry, Debian bug #650079;
               allow quoted strings in Defaults parameters, bug #263
    * Sysconfig: handle end of line comments and semicolons; strip quotes,
                 RedHat bug #761246
    * Sysctl: include /etc/sysctl.d files
    * Syslog: allow capital letters in tokens
    * Systemd: new lens to parse systemd unit files
    * Thttpd: new lens for /etc/thttpd/thttpd.conf (Marc Fournier)
    * Up2date: new lens for /etc/sysconfig/rhn/up2date
    * Util: add comment_noindent; add delim; add doseol;
            support DOS eols in various places;
            add *.bak and *.old to stdexcl, to match files in /etc/sysconfig
    * Vfstab: new lens for /etc/vfstab config on Solaris
    * Vmware_Config: new lens for /etc/vmware/config
    * Vsftpd: add require_ssl_reuse option (Danny Yates)
    * Xinetd: rewrite with Build, Sep, and Rx;
              make attribute names case-insensitive (Michal Filka)
    * Xml: support single _and_ double quoted attribute values,
           RedHat bug #799885, bug #258
    * Xymon: new lens for Xymon config files, bug #266 (Jason Kincl)
    * Yum: rebase on IniFile, support for comments, bug #217


## [release-0.10.0](https://github.com/hercules-team/augeas/tree/release-0.10.0) (2011-12-02)

  - support relative paths by taking them relative to the value
    of /augeas/context in all API functions where paths are used
  - add aug_to_xml to API: transform tree(s) into XML, exposed as dump-xml
    in aug_srun and augtool. Introduces dependency on libxml2
  - fix regular expression escaping. Previously, /[\/]/ match either a
    backslash or a slash. Now it only matches a slash
  - path expressions: add function 'int' to convert a node value (string)
    to an integer
  - path expressions: make sure the regexp produced by empty nodesets from
    regexp() and glob() matches nothing, rather than the empty word
  - fix --autosave when running single command from command line, BZ 743023
  - aug_srun: support 'insert' and 'move' as aliases for 'ins' and 'mv'
  - aug_srun: allow escaping of spaces, quotes and brackets with \
  - aug_init: accept AUG_NO_ERR_CLOSE flag; return augeas handle even when
    intialization fails so that caller gets some details about why
    initialization failed
  - aug_srun: tolerate trailing white space in commands
  - much improved, expanded documentation of many lenses
  - always interpret lens filter paths as absolute, bug #238
  - fix bug in libfa that would incorrectly calculate the difference of a
    case sensistive and case insensitive regexp (/[a-zA-Z]+/ - /word/i
    would match 'worD')
  - new builtin 'regexp_match' for .aug files to make testing regexp
    matching easier during development
  - fix 'span' command, bug #220
  - Lens changes/additions
    * Access: parse user@host and (group) in users field; field separator
      need not be surrounded by spaces
    * Aliases: allow spaces before colons
    * Aptconf: new lens for /etc/apt/apt.conf
    * Aptpreferences: support origin entries
    * Backuppchosts: new lens for /etc/backuppc/hosts, bug 233 (Adam Helms)
    * Bbhosts: various fixes
    * Cgconfig: id allowed too many characters
    * Cron: variables aren't set like shellvars, semicolons are allowed in
      email addresses; fix parsing of numeric fields, previously upper case
      chars were allowed; support ranges in time specs
    * Desktop: new lens for .desktop files
    * Dhcpd: slashes must be double-quoted; add Red Hat's dhcpd.conf
      locations
    * Exports: allow empty options
    * Fai_diskconfig: new lens for FAI disk_config files
    * Fstab: allow ',' in file names, BZ 751342
    * Host_access: new lens for /etc/hosts.{allow,deny}
    * Host_conf: new lens for /etc/host.conf
    * Hostname: new lens for /etc/hostname
    * Hosts: also load /etc/mailname by default
    * Iptables: allow digits in ipt_match keys, bug #224
    * Json: fix whitespace handling, removing some cf ambiguities
    * Kdump: new lens for /etc/kdump.conf (Roman Rakus)
    * Keepalived: support many more flags, fields and blocks
    * Krb5: support [pam] section, bug #225
    * Logrotate: be more tolerant of whitespace in odd places
    * Mdadm_conf: new lens for /etc/mdadm.conf
    * Modprobe: Parse commands in install/remove stanzas (this introduces a
      backwards incompatibility); Drop support for include as it is not
      documented in manpages and no unit tests are shipped.
    * Modules: new lens for /etc/modules
    * Multipath: add support for seveal options in defaults section, bug #207
    * Mysql: includedir statements are not part of sections; support
      \!include; allow indentation of entries and flags
    * Networks: new lens for /etc/networks
    * Nrpe: allow '=' in commands, bug #218 (Marc Fournier)
    * Php: allow indented entries
    * Phpvars: allow double quotes in variable names; accept case
      insensitive PHP tags; accept 'include_once'; allow empty lines at
      EOF; support define() and bash-style and end-of-line comments
    * Postfix_master: allow a lot more chars in words/commands, including
      commas
    * PuppetFileserver: support same-line comments and trailing whitespace,
      bug #214
    * Reprepo_uploaders: new lens for reprepro's uploaders files
    * Resolv: permit end-of-line comments
    * Schroot: new lens for /etc/schroot/schroot.conf
    * Shellvars: greatly expand shell syntax understood; support various
      syntactic constructs like if/then/elif/else, for, while, until, case,
      and select; load /etc/blkid.conf by default
    * Spacevars: add toplevel lens 'lns' for consistency
    * Ssh: new lens for ssh_config (Jiri Suchomel)
    * Stunnel: new lens for /etc/stunnel/stunnel.conf (Oliver Beattie)
    * Sudoers: support more parameter flags/options, bug #143
    * Xendconfsxp: lens for Xen configuration (Tom Limoncelli)
    * Xinetd: allow spaces after '{'

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.9.0...release-0.10.0)

## [release-0.9.0](https://github.com/hercules-team/augeas/tree/release-0.9.0) (2011-07-26)

  - augtool: keep history in ~/.augeas/history
  - add aug_srun API function; this makes it possible to run a sequence of
    commands through the API
  - aug_mv: report error AUG_EMVDESC on attempts to move a node into one of
    its descendants
  - path expressions: allow whitespace inside names, making '/files/etc/foo
    bar/baz' a legal path, but parse [expr1 or expr2] and [expr1 and expr2]
    as the logical and/or of expr1 and expr2
  - path expressions: interpret escape sequences in regexps; since '.' does
    not match newlines, it has to be possible to write '.|\n' to match any
    character
  - path expressions: allow concatenating strings and regexps; add
    comparison operator '!~'; add function 'glob'; allow passing a nodeset
    to function 'regexp'
  - store the names of the functions available in path expressions under
    /augeas/version
  - fix several smaller memory leaks
  - Lens changes/additions
    * Aliases: allow spaces and commas in aliases (Mathieu Arnold)
    * Grub: allow "bootfs" Solaris/ZFS extension for dataset name, bug #201
      (Dominic Cleal); allow kernel path starting with a BIOS device,
      bug #199
    * Inifile: allow multiline values
    * Php: include files from Zend community edition, bug #210
    * Properties: new lens for Java properties files, bug #194 (Craig Dunn)
    * Spacevars: autoload two ldap files, bug #202 (John Morrissey)
    * Sudoers: support users:groups format in a Runas_Spec line, bug #211;
      add CSW paths (Dominic Cleal)
    * Util: allow comment_or_eol to match whitespace-only comments,
      bug #205 (Dominic Cleal)
    * Xorg: accept InputClass section; autoload from /etc/X11/xorg.conf.d,
      bug #197

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.8.1...release-0.9.0)

## [release-0.8.1](https://github.com/hercules-team/augeas/tree/release-0.8.1) (2011-04-16)

  - augtool: respect autosave flag in oneshot mode, bug #193; fix segfault
    caused by unmatched bracket in path expression, bug #186
  - eliminate a global variable in the lexer, fixes BZ 690286
  - replace an erroneous assert(0) with a proper error message when none of
    the alternatives in a union match during saving, bug #183
  - improve AIX support
  - Lens changes/additions
    * Access: support the format @netgroup@@nisdomain, bug #190
    * Fstab: fix parsing of SELinux labels in the fscontext option (Matt Booth)
    * Grub: support 'device' directive for UEFI boot, bug #189; support
      'configfile' and 'background' (Onur Küçük)
    * Httpd: handle continuation lines (Bill Pemberton); autoload
      httpd.conf on Fedora/RHEL, BZ 688149; fix support for single-quoted
      strings
    * Iptables: support --tcp-flags, bug #157; allow blank and comment
      lines anywhere
    * Mysql: include /etc/my.cnf used on Fedora/RHEL, BZ 688053
    * NagiosCfg: parse setting multiple values on one line (Sebastien Aperghis)
    * NagiosObjects: process /etc/nagios3/objects/*.cfg (Sebastien Aperghis)
    * Nsswitch: support 'sudoers' as a database, bug #187
    * Shellvars: autoload /etc/rc.conf used in FreeBSD (Rich Jones)
    * Sudoers: support '#include' and '#includedir', bug #188
    * Yum: exclude /etc/yum/pluginconf.d/versionlock.list (Bill Pemberton)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.8.0...release-0.8.1)

## [release-0.8.0](https://github.com/hercules-team/augeas/tree/release-0.8.0) (2011-02-24)

  - add new 'square' lens combinator
  - add new aug_span API function
  - augtool: short options for --nostdinc, --noload, and --noautoload
  - augtool: read commands from tty after executing file with --interactive
  - augtool: add --autosave option
  - augtool: add --span option to load nodes' span
  - augtool: add span command to get the node's span according to the input
    file
  - augtool: really be quiet when we shouldn't be echoing
  - fix segfault in get.c with L_MAYBE lens; bug #180
  - fix segfault when a path expression called regexp() with an invalid
    regexp; bug #168
  - improved vim syntax file
  - replace augtest by test-augtool.sh to obviate the need for Ruby to run
    tests
  - use sys_wait module from gnulib; bug #164
  - Lens changes/additions
    * Access: new lens for /etc/security/access.conf (Lorenzo Dalrio)
    * Crypttab: new lens for /etc/crypttab (Frederic Lespez)
    * Dhcpd: new lens
    * Exports: accept hostnames with dashes; bug #169 (Sergio Ballestrero)
    * Grub: add various Solaris extensions (Dominic Cleal); support "map"
      entries, bug #148
    * Httpd: new lens for Apache config
    * Inifile: new lens indented_title_label
    * Interfaces: allow indentation for "iface" entries; bug #182
    * Mysql: change default comment delimiter from ';' to '#'; bug #181
    * Nsswitch: accept various add'l databases; bug #171
    * PuppetFileserver: new lens for Puppet's fileserver.conf (Frederic Lespez)
    * REsolv: allow comments starting with ';'; bug #173 (erinn)
    * Shellvars: autoload various snmpd config files; bug #170 (erinn)
    * Solaris_system: new lens for /etc/system on Solaris (Dominic Cleal)
    * Util (comment_c_style, empty_generic, empty_c_style): new lenses
    * Xml: generic lens to process XML files
    * Xorg: make "position" in "screen" optional; allow "Extensions"
      section; bug #175 (omzkk)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.7.4...release-0.8.0)

## [release-0.7.4](https://github.com/hercules-team/augeas/tree/release-0.7.4) (2010-11-19)

  - augtool: new clearm command to parallel setm
  - augtool: add --file option
  - Fix SEGV under gcc 4.5, caused by difficulties of the gcc optimizer
    handling bitfields (bug #149; rhbz #651992)
  - Preserve parse errors under /augeas//error: commit 5ee81630, released
    in 0.7.3, introduced a regression that would cause the loss of parse
    errors; bug #138
  - Avoid losing already parsed nodes under certain circumstances; bug #144
  - Properly record the new mtime of a saved file; previously the mtime in
    the tree was reset to 0 when a file was saved, causing unnecessary file
    reloads
  - fix a SEGV when using L_MAYBE in recursive lens; bug #136
  - Incompatible lens changes
    * Fstab: parse option values
    * Squid: various improvements, see bug #46;
    * Xinetd: map service names differently
  - Lens changes/additions
    * Aptsources: map comments properly, allow indented lines; bug #151
    * Grub: add indomU setting for Debian. Allow '=' as separator in title;
      bug #150
    * Fstab: also process /etc/mtab
    * Inetd: support rpc services
    * Iptables: allow underscore in chain names
    * Keepalived: new lens for /etc/keepalived/keepalived.conf
    * Krb5: allow digits in realm names; bug #139
    * Login_defs: new lens for /etc/login.defs (Erinn Looney-Triggs)
    * Mke2fs: new lens for /etc/mke2fs.conf
    * Nrpe: new lens for Nagios nrpe (Marc Fournier)
    * Nsswitch: new lens for /etc/nsswitch.conf
    * Odbc: new lens for /etc/odbc.ini (Marc Fournier)
    * Pg_hba: New lens; bug #140 (Aurelien Bompard). Add system path on
      Debian; bug #154 (Marc Fournier)
    * Postfix_master: parse arguments in double quotes; bug #69
    * Resolv: new lens for /etc/resolv.conf
    * Shells: new lens for /etc/shells
    * Shellvars: parse ulimit builtin
    * Sudoers: load file from /usr/local/etc (Mathieu Arnold) Allow
      'visiblepw' parameter flag; bug #143. Read files from /etc/sudoers.d
    * Syslog: new lens for /etc/syslog.conf (Mathieu Arnold)
    * Util: exclude dpkg backup files; bug #153 (Marc Fournier)
    * Yum: accept continuation lines for gpgkey; bug #132

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.7.3...release-0.7.4)

## [release-0.7.3](https://github.com/hercules-team/augeas/tree/release-0.7.3) (2010-08-07)

  - aug_load: only reparse files that have actually changed; greatly speeds
    up reloading
  - record all variables in /augeas/variables, regardless of whether they
    were defined with aug_defvar or aug_defnode; make sure
    /augeas/variables always exists
  - redefine all variables (by reevaluating their corresponding
    expressions) after a aug_load. This makes variables 'sticky' across
    loads
  - fix behavior of aug_defnode to not fail when the expression evaluates
    to a nonempty node set
  - make gnulib a git submodule so that we record the gnulib commit off
    which we are based
  - allow 'let rec' with non-recursive RHS
  - fix memory corruption when reloading a tree into which a variable
    defined by defnode points (BZ 613967)
  - plug a few small memory leaks, and some segfaults
  - Lens changes/additions
    * Device_map: new lens for grub's device.map (Matt Booth)
    * Limits: also look for files in /etc/security/limits.d
    * Mysql: new lens (Tim Stoop)
    * Shellvars: read /etc/sysconfig/suseconfig (Frederik Wagner)
    * Sudoers: allow escaped spaces in user/group names (Raphael Pinson)
    * Sysconfig: lens for the shell subdialect used in /etc/sysconfig; lens
      strips quotes automatically

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.7.2...release-0.7.3)

## [release-0.7.2](https://github.com/hercules-team/augeas/tree/release-0.7.2) (2010-06-22)

  - new API call aug_setm to set/create multiple nodes simultaneously
  - record expression used in a defvar underneath /augeas/variables
  - Lens changes/additions
    * Group: add test for disabled account (Raphael Pinson)
    * Grub: handle comments within a boot stanza
    * Iptables: also look for /etc/iptables-save (Nicolas Valcarcel)
    * Modules_conf: new lens for /etc/modules.conf (Matt Booth)
    * Securetty: added handling of emtpy lines/comments (Frederik Wagner)
    * Shellvars: added SuSE sysconfig puppet files (Frederik Wagner),
      process /etc/environment (seph)
    * Shellvars_list: Shellvars-like lens that treats strings of
      space-separated words as lists (Frederik Wagner)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.7.1...release-0.7.2)

## [release-0.7.1](https://github.com/hercules-team/augeas/tree/release-0.7.1) (2010-04-21)

  - new primitive lens 'value' to set value of a node to a constant,
    similar to 'label' for the key (see http://augeas.net/docs/lenses.html)
  - new builtins for printing and getting the types of a lens (see
    http://augeas.net/docs/builtins.html)
  - add unit type to lens language; allow '_' as an identifier in let's to
    force evaluation for side effect only
  - Various fixes for Solaris. Augeas now builds cleanly on Solaris 5.10,
    and most of the tests pass. The three tests that fail all fail because
    the test scripts have Linux idiosyncrasies. This needs to be addressed
    in a future release. Much thanks to Dagobert Michelsen and the OpenCSW
    project (http://www.opencsw.org/) for providing me with access to their
    build farm.
  - fix crash when recursive lens was used in a nonrecursive lens (bug #100)
  - context free parser/recursive lenses: handle 'l?' properly (bug #119);
    distinguish between successful parse and parse with an error at end of
    input; do caller filtering to avoid spurious ambiguous parses with
    grammars containing epsilon productions
  - aug_get: return -1 when multiple nodes match (bug #121)
  - much better error message when iteration stops prematurely during
    put/create than the dreaded 'Short iteration'
  - augtool: ignore empty lines from stdin; report error when get fails
  - fix memory leak in file_info (transform.c); this was leaking a file
    name every time we loaded a file (Laine Stump)
  - nicer error message when typechecker spots ambiguity in atype
  - libfa: handle '(a|)' and 'r{min,}' properly
  - locale independence: handle a literal '|' properly on systems that lack
    use_locale
  - bootstrap: pull in isblank explicitly (needed on Solaris)
  - src/lens.c (lns_check_rec): fix refcounting mistake on error path (bug #120)
  - fix SEGV when loading empty files
  - improvements in handling some OOM's
  - Lens changes/additions
    * Approx: lens and test for the approx proxy server (Tim Stoop)
    * Cgconfig: lens and tests for libcgroup config (Ivana Hutarova Varekova)
    * Cgrules: new lens and test (Ivana Hutarova Varekova)
    * Cobblermodules: lens + tests for cobbler's modules.conf (Shannon Hughes)
    * Debctrl: new lens and test (Dominique Dumont)
    * Dput: add 'allow_dcut' parameter (bug #105) (Raphael Pinson)
    * Dhclient: add rfc code parsing (bug #107) (Raphael Pinson)
    * Group: handle disabled passwords
    * Grub: support empty kernel parameters, Suse incl.s (Frederik Wagner)
    * Inittab: allow ':' in the process field (bug #109)
    * Logrotate: tolerate whitespace at the end of a line (bug #101); files
      can be separated by newlines (bug #104) (Raphael Pinson)
    * Modprobe: Suse includes (Frederik Wagner)
    * Nagisocfg: lens and test for /etc/nagios3/nagios.cfg (Tim Stoop)
    * Ntp: add 'tinker' directive (bug #103)
    * Passwd: parse NIS entries on Solaris
    * Securetty: new lens and test for /etc/securetty (Simon Josi)
    * Shellvars: handle a bare 'export VAR'; Suse includes (Frederik
      Wagner); allow spaces after/before opening/closing parens for array
    * Sshd: allow optional arguments in subsystem commands (Matt Palmer)
    * Sudoers: allow del_negate even if no negate_node is found (bug #106)
               (Raphael Pinson); accept 'secure_path' (BZ 566134) (Stuart
               Sears)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.7.0...release-0.7.1)

## [release-0.7.0](https://github.com/hercules-team/augeas/tree/release-0.7.0) (2010-01-15)

  - Support for context-free lenses via the 'let rec' keyword. The syntax
    is experimental, though the feature is here to stay. See
    lenses/json.aug for an example of what's possible with that.
  - Support for case-insensitive regular expressions. Simply append 'i' to
    a regexp literal to make it case-insensitive, e.g. /hello/i will match
    all variations of hello, regardless of case.
  - Major revamp of augtool. In particular, path expressions don't need to
    be quoted anymore. The online help has been greatly improved.
  - Check during load/save that each file is only matched by one transform
    under /augeas/load. If there are multiple transforms for a file, the
    file is skipped.
  - New error codes AUG_ENOLENS and AUG_EMXFM
  - Do not choke on non-existing lens during save
  - Change the metadata for files under /augeas/files slightly: the node
    /augeas/files/$PATH/lens now has the name of the lens used to load the
    file; the source location of that lens has moved to
    /augeas/files/$PATH/lens/info
  - New public functions fa_nocase, fa_is_nocase, and fa_expand_nocase in
    libfa
  - Various smaller bug fixes, performance improvements and improved error
    messages
  - Lens changes/additions
    * Cobblersettings: new lens and test (Bryan Kearney)
    * Iptables: allow quoted strings as arguments; handle both negation
      syntaxes
    * Json: lens and tests for generic Json files
    * Lokkit: allow '-' in arguments
    * Samba: accept entry keys with ':' (Partha Aji)
    * Shellvars: allow arrays that span multiple lines
    * Xinetd (name): fix bad '-' in character class

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.6.0...release-0.7.0)

## [release-0.6.0](https://github.com/hercules-team/augeas/tree/release-0.6.0) (2009-11-30)

  - Add error reporting API (aug_error and related calls); use to report
    error details in a variety of places
  - Path expressions: add regexp matching; add operator '|' to form union
    of nodesets (ticket #89)
  - Tolerate non-C locales from the environment (ticket #35); it is no
    longer necessary to set the locale to C from the outside
  - use stpcpy/stpncpy from gnulib (needed for building on Solaris)
  - Properly check regexp literals for syntax errors (ticket #93)
  - Distribute and install vim syntax files (ticket #97)
  - many more bugfixes
  - Lens changes/additions
    * Apt_preferences: support version pin; filter out empty lines (Matt
      Palmer)
    * Cron: variables can contain '_' etc. (ticket #94)
    * Ethers: new lens for /etc/ethers (Satoru SATOH)
    * Fstab: allow '#' in spec (ticket #95)
    * Group: allow empty password field (ticket #95)
    * Inittab: parse end-of-line comments into a #comment
    * Krb5: support kdc section; add v4_name_convert subsection to
      libdefaults (ticket #95)
    * Lokkit: add mising eol to forward_port; make argument for --trust
      more permissive
    * Pam: allow '-' before type
    * Postfix_access: new lens for /etc/postfix/access (Partha Aji)
    * Rx: allow '!' in device_name
    * Sudoers: allow certain backslash-quoted characters in a command (Matt
      Palmer)
    * Wine: new lens to read Windows registry files

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.5.3...release-0.6.0)

## [release-0.5.3](https://github.com/hercules-team/augeas/tree/release-0.5.3) (2009-09-14)

  - Match trees on label + value, not just label; see
    tests/modules/pass_strip_quotes.aug for how that enables stripping
    quotes
  - Do not trip over symlinks to files on a different device during save;
    fixes problems with writing to /etc/grub.conf on Fedora/RHEL
  - API (defnode): always add the newly created node into the resulting
    nodeset
  - Add preceding-sibling and following-sibling axes to path expressions
  - augtool, augparse: add --version option (bug #88)
  - Change file info recorded under /augeas/files/FILE/*: remove lens/id
    and move lens/info to lens
  - Properly record new files under /augeas/files (bug #78)
  - aug_load: clean up variables to avoid dangling references (bug #79)
  - Make Augeas work on AIX
  - Ignore anything but regular files when globbing
  - Add 'clear' function to language for use in unit tests
  - typechecker: print example trees in tree format
  - libfa: properly support regexps with embedded NUL's
  - Lens changes/additions
    * Xorg: revamped, fixes various parse failures (Matt Booth)
    * Inetd: new lens and test (Matt Palmer)
    * Multipath: new lens and test
    * Slapd: also read /etc/openldap.slapd.conf (bug #85)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.5.2...release-0.5.3)

## [release-0.5.2](https://github.com/hercules-team/augeas/tree/release-0.5.2) (2009-07-14)

  - Make Augeas work on Mac OS/X (bug #66) (Anders Bjoerklund)
  - reduce symbols exported from libfa with linker script
  - add --echo option to augtool
  - require Automake 1.11 (Jim Meyering)
  - avoid spurious save attempts for freshly read files
  - Lens changes/additions
    * Inittab: schema change: use 'id' field as name of subtree for a line,
      instead of a generated number. Map comments as '#comment' (Matt Palmer)
    * Logrotate: make owner/group in create statement optional, allow
      filenames to be indented
    * Ntp: allow additional options for server etc. (bug #72)
    * Shellvars: allow backticks as quote characters (bug #74)
    * Yum: also read files in /etc/yum/pluginconf.d (Marc Fournier)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.5.1...release-0.5.2)

## [release-0.5.1](https://github.com/hercules-team/augeas/tree/release-0.5.1) (2009-06-10)

  - augeas.h: flag AUG_NO_MODL_AUTOLOAD suppresses initial loading
              of modules; exposed as --noautoload in augtool
  - augtool: don't prompt when input is not from tty (Raphael Pinson)
  - augparse: add --notypecheck option
  - path expressions: allow things like '/foo and /bar[3]' in predicates
  - Lens changes/additions
    * Aliases: map comments as #comment (Raphael Pinson)
    * Build, Rx, Sep: new utility modules (Raphael Pinson)
    * Cron: new lens (Raphael Pinson)
    * Dnsmasq: process files in /etc/dnsmasq.d/* (ticket #65)
    * Grub: parse kernel and module args into separate nodes; parse
            arguments for 'serial', 'terminal', and 'chainloader'; allow
            optional argument for 'savedefault'
    * Interfaces: make compliant with actual Debian spec (Matt Palmer)
    * Iptables: relax regexp for chain names; allow comment lines mixed
                in with chains and rules (ticket #51)
    * Logrotate: allow '=' as separator (ticket #61); make newline at end
                 of scriptlet optional
    * Modprobe: handle comments at end of line
    * Ntp: parse fudge record (Raphael Pinson); parse all directives in
           default Fedora ntp.conf; process 'broadcastdelay', 'leapfile',
           and enable/disable flags (ticket #62)
    * Pbuilder: new lens for Debian's personal builder (Raphael Pinson)
    * Php: add default path on Fedora/RHEL (Marc Fournier)
    * Squid: handle indented entries (Raphael Pinson)
    * Shellvars: map 'export' and 'unset'; map comments as #comment
                 (Raphael Pinson)
    * Sudoers: allow backslashes inside values (ticket #60) (Raphael Pinson)
    * Vsftpd: map comments as #comment; handle empty lines; find
              vsftpd.conf on Fedora/RHEL
    * Xinetd: map comments as #comment (Raphael Pinson)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.5.0...release-0.5.1)

## [release-0.5.0](https://github.com/hercules-team/augeas/tree/release-0.5.0) (2009-03-28)

  - Clean up interface for libfa; the interface is now considered stable
  - New aug_load API call; allows controlling which files to load by
    modifying /augeas/load and then calling aug_load; on startup, the
    transforms marked with autoload are reported under /augeas/load
  - New flag AUG_NO_LOAD for aug_init to keep it from loading files on
    startup; add --noload option to augtool
  - New API calls aug_defvar and aug_defnode to define variables for
    path expressions; exposed as 'defvar' and 'defnode' in augtool
  - Lenses distributed with Augeas are now installed in
    /usr/share/augeas/lenses/dist, which is searched after
    /usr/share/augeas/lenses, so that lenses installed by other packages
    take precedence
  - New program examples/fadot to draw various finite automata (Francis
    Giraldeau)
  - Report line number and character offset in the tree when parsing a
    file with a lens fails
  - Fix error in propagation of dirty flag, which could lead to only
    parts of a tree being saved when multiple files were modified
  - Flush files to disk before moving them
  - Fix a number of memory corruptions in the XPath evaluator
  - Several performance improvements in libfa
  - Lens changes/additions
    * Grub: process embedded comments for update-grub (Raphael Pinson)
    * Iptables: new lens for /etc/sysconfig/iptables
    * Krb5: new lens for /etc/krb5.conf
    * Limits: map dpmain as value of 'domain' node, not as label
              (Raphael Pinson)
    * Lokkit: new lens for /etc/sysconfig/system-config-firewall
    * Modprobe: new lens for /etc/modprobe.d/*
    * Sudoers: more finegrained parsing (ticket #48) (Raphael Pinson)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.4.2...release-0.5.0)

## [release-0.4.2](https://github.com/hercules-team/augeas/tree/release-0.4.2) (2009-03-09)

  - Do not delete files that had an error upon parsing
  - For Fedora/EPEL RPM's, BuildRequire libselinux-devel (bug #26)
  - Bug fixes in path expressions
    * for numbers, the meaning of '<' and '<=' was reversed
  - Always create an entry /files in aug_init
  - New builtin 'Sys' module with functions 'getenv' and 'read_file',
    the latter reads a the contents of a file into a string
  - Lens changes/additions
    * Postfix_main: handle continuation lines
    * Bbhosts, Hosts, Logrotate, Sudoers: label comment nodes as '#comment'
    * Sshd: map comments as '#comment' nodes
    * Squid: add all keywords from squid 2.7 and 3 (Francois Deppierraz)
    * Logrotate: process unit suffixes for 'size' and 'minsize'

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.4.1...release-0.4.2)

## [release-0.4.1](https://github.com/hercules-team/augeas/tree/release-0.4.1) (2009-03-03)

  - Remove files when their entire subtree under /files is deleted
  - Various bug fixes and syntax enhancements for path expressions
    (see tests/xpath.tests for details)
  - Evaluate path expressions with multiple predicates correctly
  - Fix incorrect setting of /augeas/events/saved
  - Major cleanup of matching during get; drastically improves
    performance for very large (on the order of 10k lines) config files
  - Small performance improvement in the typechecker
  - Reject invalid character sets like [x-u] during typecheck
  - Build with compile warnings set to 'maximum' instead of 'error', so
    that builds on platforms with broken headers will work out of the box
  - Lens changes/additions
    * Util.stdexcl now excludes .augsave and .augnew files
    * Logrotate: allow 'yearly' schedule, spaces around braces
    * Ntp: fix so that it processes ntp.conf on Fedora 10
    * Services: lens for /etc/services (Raphael Pinson)
    * Xorg: new lens and tests (Raphael Pinson)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.4.0...release-0.4.1)

## [release-0.4.0](https://github.com/hercules-team/augeas/tree/release-0.4.0) (2009-02-07)

  - Much improved and expanded support for path expressions in the public
    API. See doc/xpath.txt and tests/xpath.tests for details.
  - Solaris support: builds at least on OpenSolaris 2008.11
  - Lens changes/additions
    * Grub: support color and savedefault
    * DarkIce: new lens for http://darkice.tyrell.hu/ (Free Ekanayaka)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.6...release-0.4.0)

## [release-0.3.6](https://github.com/hercules-team/augeas/tree/release-0.3.6) (2009-01-27)

  - report version in /augeas/version, report legal save modes in
    /augeas/version/save/mode for feature tests/version checking
  - dynamically change behavior of aug_save; add noop save mode
    (Bryan Kearney)
  - plug memory leak, more portable SELinux test (Jim Meyering)
  - fix bz #478619 - do not use abspath (Arnaud Gomes-do-Vale)
  - fix segfault when branch in a union does not have a ktype
  - Lens changes/additions
    * Dpkg: new lens for Debian's dpkg.cfg (Robin Lee Powell)
    * Limits: new lens for /etc/security/limits.conf (Free Ekanayaka)
    * Soma: new lens for http://www.somasuite.org/ config
      (Free Ekanayaka)
    * Php, Gdm: fix minor regexp error (Marc Fournier)
      expand filter for Php config files (Robin Lee Powell)
    * Phpvars: whitspace fixes (Free Ekanayaka)
    * Puppet: accept indented puppet.conf (ticket #25)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.5...release-0.3.6)

## [release-0.3.5](https://github.com/hercules-team/augeas/tree/release-0.3.5) (2008-12-24)

  - add an option to rewrite files by overwriting their contents instead of
    putting the new file in place atomically with rename(2); file contents
    are only copied after rename fails with EXDEV or EBUSY, and only if the
    node /augeas/save/copy_if_rename_fails (fix #32)
  - saving of backup (.augsave) files now works even if the original and
    backup files are on different devices
  - major refactoring of how path expressions are handled internally. Fixes
    a number of bugs and oddities (e.g. tickets #7 and #23)
  - fix a bug in fa_as_regexp: a '.' wasn't escaped, ultimately leading to
    spurious errors from the typechecker
  - Lens changes/additions
    * Group: process /etc/group (Free Ekanayaka)
    * Passwd: process /etc/passwd (Free Ekanayaka)
    * Phpvars: process files that set PHP variables, in particular
      /etc/squirrelmail/config.php (Free Ekanayaka)
    * Rsyncd: process /etc/rsyncd.conf (Marc Fournier)
    * Shellvars: process /etc/arno-iptables-firewall/debconf.cfg and
      /etc/cron-apt/config (Free Ekanayaka), load /etc/sysconfig/sendmail
    * Postfix: process postfix's main.cf and master.cf (Free Ekanayaka)
    * Squid: new lens for squid.conf (Free Ekanayaka)
    * Webmin: new lens (Free Ekanayaka)
    * Xinetd: make sure equal sign is surrounded by spaces (#30)
    * Sshd: change the structure of Condition subtrees (Dominique Dumont)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.4...release-0.3.5)

## [release-0.3.4](https://github.com/hercules-team/augeas/tree/release-0.3.4) (2008-11-06)

  - fix saving of backup files; in 0.3.3, when AUG_SAVE_BACKUP was passed
    to aug_init, aug_save would always fail

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.3...release-0.3.4)

## [release-0.3.3](https://github.com/hercules-team/augeas/tree/release-0.3.3) (2008-10-24)

  - restore the behavior of aug_save; in 0.3.2, aug_save broke API by
    returning the number of files changed on success instead of 0

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.2...release-0.3.3)

## [release-0.3.2](https://github.com/hercules-team/augeas/tree/release-0.3.2) (2008-10-22)

  - saving now reports which files were actually changed in
    /augeas/events/saved; aug_save also returns the number of files
    that were changed
  - preserve file owner, permissions and SELinux context when changing a file.
  - make saving idempotent, i.e. when a change to the tree does not result
    in changes to the actual file's content, do not touch the original file
  - report an error if there are nodes in the tree with a label that
    is not allowed by the lens
  - quietly append a newline to files that do not have one
  - generate lens documentation using NaturalDocs and publish those
    on the Auegas website (Raphael Pinson)
  - Lens changes/additions
    * Grub: support the 'password' directive (Joel Nimety)
    * Grub: support 'serial' and 'terminal' directives (Sean E. Millichamp)
    * Samba: change default indentation and separators (Free Ekanayaka)
    * Logrotate: process tabooext, add dateext flag (Sean E. Millichamp)
    * Sshd: Cleaner handling of 'Match' blocks (Dominique Dumont)
    * Monit: new lens (Free Ekanayaka)
    * Ldap: merge with Spacevars (Free Ekanayaka)
    * Shellvars: support /etc/default (Free Ekanayaka)
    * Shellvars: handle space at the end of a line

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.1...release-0.3.2)

## [release-0.3.1](https://github.com/hercules-team/augeas/tree/release-0.3.1) (2008-09-04)

  - Major performance improvement when processing huge files, reducing some
    O(n^2) behavior to O(n) behavior. It's now entirely feasible to
    manipulate for example /etc/hosts files with 65k lines
  - Handle character escapes '\x' in regular expressions in compliance with
    Posix ERE
  - aug_mv: fix bug when moving at the root level
  - Fix endless loop when using a mixed-case module name like MyMod.lns
  - Typecheck del lens: for 'del RE STR', STR must match RE
  - Properly typecheck the '?' operator, especially the atype; also allow
    '?' to be applied to lenses that contain only 'store', and do not
    produce tree nodes.
  - Many new/improved lenses
    * many lenses now map comments as '#comment' nodes instead of just
      deleting them
    * Sudoers: added (Raphael Pinson)
    * Hosts: map comments into tree, handle whitespace and comments
             at the end of a line (Kjetil Homme)
    * Xinetd: allow indented comments and spaces around "}" (Raphael Pinson)
    * Pam: allow comments at the end of lines and leading spaces
           (Raphael Pinson)
    * Fstab: map comments and support empty lines (Raphael Pinson)
    * Inifile: major revamp (Raphael Pinson)
    * Puppet: new lens for /etc/puppet.conf (Raphael Pinson)
    * Shellvars: handle quoted strings and arrays (Nahum Shalman)
    * Php: map entries outside of sections to a '.anon' section
           (Raphael Pinson)
    * Ldap: new lens for /etc/ldap.conf (Free Ekanayaka)
    * Dput: add allowed_distributions entry (Free Ekanayaka)
    * OpenVPN: new lens for /etc/openvpn/{client,server}.conf (Raphael Pinson)
    * Dhclient: new lens for /etc/dhcp3/dhclient.conf (Free Ekanayaka)
    * Samba: new lens for /etc/samba/smb.conf (Free Ekanayaka)
    * Slapd: new lens for /etc/ldap/slapd.conf (Free Ekanayaka)
    * Dnsmasq: new lens for /etc/dnsmasq.conf (Free Ekanayaka)
    * Sysctl: new lens for /etc/sysctl.conf (Sean Millichamp)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.3.0...release-0.3.1)

## [release-0.3.0](https://github.com/hercules-team/augeas/tree/release-0.3.0) (2008-08-08)

  - Add aug_mv call to public API
  - Do not clobber symlinks, instead write new files to target of symlink
  - Fail 'put' when tree has invalid entries
  - Set exit status of augtool
  - Avoid picking special characters, in particular '\0', in examples (libfa)
  - Store system errors, using strerror, in the tree during writing of files
  - New lenses
    * Generic inifile module (Raphael Pinson)
    * logrotate (Raphael Pinson)
    * /etc/ntp.conf (Raphael Pinson)
    * /etc/apt/preferences (Raphael Pinson)
    * bbhosts for Big Brother [http://www.bb4.org/] (Raphael Pinson)
    * php.ini (Raphael Pinson)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.2.2...release-0.3.0)

## [release-0.2.2](https://github.com/hercules-team/augeas/tree/release-0.2.2) (2008-07-18)

  - Fix segfault in store.put on NULL values
  - Properly move default lens dir with DATADIR (Jim Meyering)
  - Fix 'short iteration' error on get/parse of empty string; this bug
    made it impossible to save into a new file
  - Add 'insa' and 'insb' primitives to allow insertion from
    put unit tests
  - aug_insert: handle insertion before first child properly
  - New lenses
    * /etc/exports: NFS exports
    * /etc/dput.cf: Debian's dput (Raphael Pinson)
    * /etc/aliases: don't require whitespace after comma (Greg Swift)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.2.1...release-0.2.2)

## [release-0.2.1](https://github.com/hercules-team/augeas/tree/release-0.2.1) (2008-07-01)

  - Address some compilation issues found on Ubuntu/Debian unstable
  - Fix segfault when aug_init/close are called multiple times
  - Man page for augparse
  - New lenses
    * /etc/sysconfig/selinux
    * Bugfixes for grub.conf

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.2.0...release-0.2.1)

## [release-0.2.0](https://github.com/hercules-team/augeas/tree/release-0.2.0) (2008-06-05)

  - Augeas is now much more portable
    * Pull in gnulib on non-glibc systems
    * Augeas now builds and runs on FreeBSD (possibly others, too)
  - Various fixes for memory corruption and the like
    (Jim Meyering, James Antill)
  - New lenses
    * vsftpd.conf
    * various bugfixes in existing lenses

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.1.1...release-0.2.0)

## [release-0.1.1](https://github.com/hercules-team/augeas/tree/release-0.1.1) (2008-05-16)

  - Add subtraction of regexps to the language, for example
      let re = /[a-z]+/ - /(Allow|Deny)Users/
  - Report errors during get/put in the tree; added subnodes to
    /augeas/files/PATH/error for that purpose
  - Many many bugfixes:
    * plugged all known memory leaks
    * fixed typecheck for lens union (l1 | l2) which was plain wrong
    * reduce overall memory usage by releasing unused compiled regexps
    * further performance improvements in libfa
    * check that values match the regexps in STORE when saving
  - libfa can now convert an automaton back to a regular expression
    (FA_AS_REGEXP)
  - New lenses
    * /etc/fstab
    * /etc/xinetd.conf and /etc/xinetd.d/*

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.1.0...release-0.1.1)

## [release-0.1.0](https://github.com/hercules-team/augeas/tree/release-0.1.0) (2008-05-01)

  - Various changes to public API:
    * Remove aug_exists from public API, and merge functionality into aug_get
    * Do not hide pointer behind typedef; instead Augeas 'handle' type is now
      struct augeas, typedef'd to augeas (Jim Meyering)
    * Const-correctness of public API, return error indication
      from aug_print (Jim Meyering)
    * Make buildable on Debian Etch (remove -fstack-protector from compiler
      switches)
  - Public API is now stable, and existing calls will be supported without
    further changes
  - New schema:
    * /etc/sysconfig/network-scripts/ifcfg-* (Alan Pevec)
    * Assorted other files from /etc/sysconfig (the ones that just set
      shell variables)
    * /etc/apt/sources.list and /etc/apt/sources.list.d/* (Dean Wilson)
  - Man page for augtool (Dean Wilson)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.8...release-0.1.0)

## [release-0.0.8](https://github.com/hercules-team/augeas/tree/release-0.0.8) (2008-04-17)

  - Complete rewrite of the language for schema descriptions

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.7...release-0.0.8)

## [release-0.0.7](https://github.com/hercules-team/augeas/tree/release-0.0.7) (2008-03-14)

  - Typecheck lenses; in particular, discover and complain about ambiguous
    concatenation and iteration
  - Enable typechecking for augparse by default, and for augtool via the
    '-c' flag
  - Fixed lens definitions in spec/ to pass typechecking. They contained
    quite a few stupid and subtle problems
  - Greatly improved libfa performance to make typechecking reasonably
    fast. Typechecking cmfm.aug went from more than two hours to under two
    seconds

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.6...release-0.0.7)

## [release-0.0.6](https://github.com/hercules-team/augeas/tree/release-0.0.6) (2008-03-06)

  - Make it possible to overwrite files when saving with and without
    backups
  - Take the filesystem root as an optional argument to aug_init
  - Expose these two things as command line options in augtool

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.5...release-0.0.6)

## [release-0.0.5](https://github.com/hercules-team/augeas/tree/release-0.0.5) (2008-03-05)

  - Changed public API to contain explicit reference to augeas_t
    structure. This makes it easier to write threadsafe code using Augeas
  - Added libfa, finite automata library, though it's not yet used by
    Augeas

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.4...release-0.0.5)

## [release-0.0.4](https://github.com/hercules-team/augeas/tree/release-0.0.4) (2008-02-26)

  - package as RPM and make sure Augeas can be build on Fedora/RHEL

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.3...release-0.0.4)

## [release-0.0.3](https://github.com/hercules-team/augeas/tree/release-0.0.3) (2008-02-26)

  - further rework; file processing now resembles Boomerang lenses much
    more closely
  - major revamp of the internal tree representation (ordered tree where
    multiple children can have the same label, including NULL labels)
  - move away from LL(1) parsing in favor of regular languages, since they
    enable much better ahead-of-time checks (which are not implemented yet)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.2...release-0.0.3)

## [release-0.0.2](https://github.com/hercules-team/augeas/tree/release-0.0.2) (2008-02-25)

  - completely reworked
  - processing of files is now based on a textual description of the
    structure of the files (basically a LL(1) grammar)

[Full Changelog](https://github.com/hercules-team/augeas/compare/release-0.0.1...release-0.0.2)

## [release-0.0.1](https://github.com/hercules-team/augeas/tree/release-0.0.1) (2007-12-02)

  - First release.
  - Public API and basic tree data structure.
  - Record scanning works.
  - Providers for pam.d, inittab and /etc/hosts
  - Simple tests and test driver

[Full Changelog](https://github.com/hercules-team/augeas/compare/f1e73cec2b4138d85be320662df9ef015bf445c9...release-0.0.1)



\* *This Changelog was automatically generated by [github_changelog_generator](https://github.com/github-changelog-generator/github-changelog-generator)*
