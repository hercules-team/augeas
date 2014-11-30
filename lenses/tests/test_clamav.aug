module Test_ClamAV =

let clamd_conf ="##
## Example config file for the Clam AV daemon
## Please read the clamd.conf(5) manual before editing this file.
##

# Comment or remove the line below.
Example
# LogFile must be writable for the user running daemon.
LogFile /var/log/clamav/clamd.log
LogFileUnlock yes
LogFileMaxSize 0
LogTime yes
LogClean yes
LogSyslog yes
LogFacility LOG_MAIL
LogVerbose yes
LogRotate yes
ExtendedDetectionInfo yes
PidFile /var/run/clamav/clamd.pid
TemporaryDirectory /var/tmp
DatabaseDirectory /var/lib/clamav
OfficialDatabaseOnly no
LocalSocket /var/run/clamav/clamd.sock
LocalSocketGroup virusgroup
LocalSocketMode 660
FixStaleSocket yes
TCPSocket 3310
TCPAddr 127.0.0.1
MaxConnectionQueueLength 30
StreamMaxLength 10M
StreamMinPort 30000
StreamMaxPort 32000
MaxThreads 50
ReadTimeout 300
CommandReadTimeout 5
SendBufTimeout 200
MaxQueue 200
IdleTimeout 60
ExcludePath ^/proc/
ExcludePath ^/sys/
MaxDirectoryRecursion 20
FollowDirectorySymlinks yes
FollowFileSymlinks yes
CrossFilesystems yes
SelfCheck 600
VirusEvent /usr/local/bin/send_sms 123456789 \"VIRUS ALERT: %v\"
User clam
AllowSupplementaryGroups yes
ExitOnOOM yes
Foreground yes
Debug yes
LeaveTemporaryFiles yes
AllowAllMatchScan no
DetectPUA yes
ExcludePUA NetTool
ExcludePUA PWTool
IncludePUA Spy
IncludePUA Scanner
IncludePUA RAT
AlgorithmicDetection yes
ForceToDisk yes
DisableCache yes
ScanPE yes
DisableCertCheck yes
ScanELF yes
DetectBrokenExecutables yes
ScanOLE2 yes
OLE2BlockMacros no
ScanPDF yes
ScanSWF yes
ScanMail yes
ScanPartialMessages yes
PhishingSignatures yes
PhishingScanURLs yes
PhishingAlwaysBlockSSLMismatch no
PhishingAlwaysBlockCloak no
PartitionIntersection no
HeuristicScanPrecedence yes
StructuredDataDetection yes
StructuredMinCreditCardCount 5
StructuredMinSSNCount 5
StructuredSSNFormatNormal yes
StructuredSSNFormatStripped yes
ScanHTML yes
ScanArchive yes
ArchiveBlockEncrypted no
MaxScanSize 150M
MaxFileSize 30M
MaxRecursion 10
MaxFiles 15000
MaxEmbeddedPE 10M
MaxHTMLNormalize 10M
"

let freshclam_conf ="##
## Example config file for freshclam
## Please read the freshclam.conf(5) manual before editing this file.
##


# Comment or remove the line below.
Example

DatabaseDirectory /var/lib/clamav
UpdateLogFile /var/log/clamav/freshclam.log
LogFileMaxSize 2M
LogTime yes
LogVerbose yes
LogSyslog yes
LogFacility LOG_MAIL
LogRotate yes
PidFile /var/run/freshclam.pid
DatabaseOwner clam
AllowSupplementaryGroups yes
DNSDatabaseInfo current.cvd.clamav.net
DatabaseMirror db.XY.clamav.net
DatabaseMirror database.clamav.net
MaxAttempts 5
ScriptedUpdates yes
CompressLocalDatabase no
DatabaseCustomURL http://myserver.com/mysigs.ndb
DatabaseCustomURL file:///mnt/nfs/local.hdb
PrivateMirror mirror1.mynetwork.com
PrivateMirror mirror2.mynetwork.com
Checks 24
HTTPProxyServer myproxy.com
HTTPProxyPort 1234
HTTPProxyUsername myusername
HTTPProxyPassword mypass
HTTPUserAgent SomeUserAgentIdString
LocalIPAddress aaa.bbb.ccc.ddd
NotifyClamd /etc/clamd.conf
OnUpdateExecute command
OnErrorExecute command
OnOutdatedExecute command
Foreground yes
Debug yes
ConnectTimeout 60
ReceiveTimeout 60
TestDatabases yes
SubmitDetectionStats /etc/clamd.conf
DetectionStatsCountry za
DetectionStatsCountry zw
SafeBrowsing yes
Bytecode yes
ExtraDatabase dbname1
ExtraDatabase dbname2
"

test ClamAV.lns get clamd_conf =
    { "#comment" = "#" }
    { "#comment" = "# Example config file for the Clam AV daemon" }
    { "#comment" = "# Please read the clamd.conf(5) manual before editing this file." }
    { "#comment" = "#" }
    {  }
    { "#comment" = "Comment or remove the line below." }
    { "Example" }
    { "#comment" = "LogFile must be writable for the user running daemon." }
    { "LogFile" = "/var/log/clamav/clamd.log" }
    { "LogFileUnlock" = "yes" }
    { "LogFileMaxSize" = "0" }
    { "LogTime" = "yes" }
    { "LogClean" = "yes" }
    { "LogSyslog" = "yes" }
    { "LogFacility" = "LOG_MAIL" }
    { "LogVerbose" = "yes" }
    { "LogRotate" = "yes" }
    { "ExtendedDetectionInfo" = "yes" }
    { "PidFile" = "/var/run/clamav/clamd.pid" }
    { "TemporaryDirectory" = "/var/tmp" }
    { "DatabaseDirectory" = "/var/lib/clamav" }
    { "OfficialDatabaseOnly" = "no" }
    { "LocalSocket" = "/var/run/clamav/clamd.sock" }
    { "LocalSocketGroup" = "virusgroup" }
    { "LocalSocketMode" = "660" }
    { "FixStaleSocket" = "yes" }
    { "TCPSocket" = "3310" }
    { "TCPAddr" = "127.0.0.1" }
    { "MaxConnectionQueueLength" = "30" }
    { "StreamMaxLength" = "10M" }
    { "StreamMinPort" = "30000" }
    { "StreamMaxPort" = "32000" }
    { "MaxThreads" = "50" }
    { "ReadTimeout" = "300" }
    { "CommandReadTimeout" = "5" }
    { "SendBufTimeout" = "200" }
    { "MaxQueue" = "200" }
    { "IdleTimeout" = "60" }
    { "ExcludePath" = "^/proc/" }
    { "ExcludePath" = "^/sys/" }
    { "MaxDirectoryRecursion" = "20" }
    { "FollowDirectorySymlinks" = "yes" }
    { "FollowFileSymlinks" = "yes" }
    { "CrossFilesystems" = "yes" }
    { "SelfCheck" = "600" }
    { "VirusEvent" = "/usr/local/bin/send_sms 123456789 \"VIRUS ALERT: %v\"" }
    { "User" = "clam" }
    { "AllowSupplementaryGroups" = "yes" }
    { "ExitOnOOM" = "yes" }
    { "Foreground" = "yes" }
    { "Debug" = "yes" }
    { "LeaveTemporaryFiles" = "yes" }
    { "AllowAllMatchScan" = "no" }
    { "DetectPUA" = "yes" }
    { "ExcludePUA" = "NetTool" }
    { "ExcludePUA" = "PWTool" }
    { "IncludePUA" = "Spy" }
    { "IncludePUA" = "Scanner" }
    { "IncludePUA" = "RAT" }
    { "AlgorithmicDetection" = "yes" }
    { "ForceToDisk" = "yes" }
    { "DisableCache" = "yes" }
    { "ScanPE" = "yes" }
    { "DisableCertCheck" = "yes" }
    { "ScanELF" = "yes" }
    { "DetectBrokenExecutables" = "yes" }
    { "ScanOLE2" = "yes" }
    { "OLE2BlockMacros" = "no" }
    { "ScanPDF" = "yes" }
    { "ScanSWF" = "yes" }
    { "ScanMail" = "yes" }
    { "ScanPartialMessages" = "yes" }
    { "PhishingSignatures" = "yes" }
    { "PhishingScanURLs" = "yes" }
    { "PhishingAlwaysBlockSSLMismatch" = "no" }
    { "PhishingAlwaysBlockCloak" = "no" }
    { "PartitionIntersection" = "no" }
    { "HeuristicScanPrecedence" = "yes" }
    { "StructuredDataDetection" = "yes" }
    { "StructuredMinCreditCardCount" = "5" }
    { "StructuredMinSSNCount" = "5" }
    { "StructuredSSNFormatNormal" = "yes" }
    { "StructuredSSNFormatStripped" = "yes" }
    { "ScanHTML" = "yes" }
    { "ScanArchive" = "yes" }
    { "ArchiveBlockEncrypted" = "no" }
    { "MaxScanSize" = "150M" }
    { "MaxFileSize" = "30M" }
    { "MaxRecursion" = "10" }
    { "MaxFiles" = "15000" }
    { "MaxEmbeddedPE" = "10M" }
    { "MaxHTMLNormalize" = "10M" }

test ClamAV.lns get freshclam_conf =
    { "#comment" = "#" }
    { "#comment" = "# Example config file for freshclam" }
    { "#comment" = "# Please read the freshclam.conf(5) manual before editing this file." }
    { "#comment" = "#" }
    {  }
    {  }
    { "#comment" = "Comment or remove the line below." }
    { "Example" }
    {  }
    { "DatabaseDirectory" = "/var/lib/clamav" }
    { "UpdateLogFile" = "/var/log/clamav/freshclam.log" }
    { "LogFileMaxSize" = "2M" }
    { "LogTime" = "yes" }
    { "LogVerbose" = "yes" }
    { "LogSyslog" = "yes" }
    { "LogFacility" = "LOG_MAIL" }
    { "LogRotate" = "yes" }
    { "PidFile" = "/var/run/freshclam.pid" }
    { "DatabaseOwner" = "clam" }
    { "AllowSupplementaryGroups" = "yes" }
    { "DNSDatabaseInfo" = "current.cvd.clamav.net" }
    { "DatabaseMirror" = "db.XY.clamav.net" }
    { "DatabaseMirror" = "database.clamav.net" }
    { "MaxAttempts" = "5" }
    { "ScriptedUpdates" = "yes" }
    { "CompressLocalDatabase" = "no" }
    { "DatabaseCustomURL" = "http://myserver.com/mysigs.ndb" }
    { "DatabaseCustomURL" = "file:///mnt/nfs/local.hdb" }
    { "PrivateMirror" = "mirror1.mynetwork.com" }
    { "PrivateMirror" = "mirror2.mynetwork.com" }
    { "Checks" = "24" }
    { "HTTPProxyServer" = "myproxy.com" }
    { "HTTPProxyPort" = "1234" }
    { "HTTPProxyUsername" = "myusername" }
    { "HTTPProxyPassword" = "mypass" }
    { "HTTPUserAgent" = "SomeUserAgentIdString" }
    { "LocalIPAddress" = "aaa.bbb.ccc.ddd" }
    { "NotifyClamd" = "/etc/clamd.conf" }
    { "OnUpdateExecute" = "command" }
    { "OnErrorExecute" = "command" }
    { "OnOutdatedExecute" = "command" }
    { "Foreground" = "yes" }
    { "Debug" = "yes" }
    { "ConnectTimeout" = "60" }
    { "ReceiveTimeout" = "60" }
    { "TestDatabases" = "yes" }
    { "SubmitDetectionStats" = "/etc/clamd.conf" }
    { "DetectionStatsCountry" = "za" }
    { "DetectionStatsCountry" = "zw" }
    { "SafeBrowsing" = "yes" }
    { "Bytecode" = "yes" }
    { "ExtraDatabase" = "dbname1" }
    { "ExtraDatabase" = "dbname2" }
