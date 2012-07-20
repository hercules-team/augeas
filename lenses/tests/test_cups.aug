(*
Module: Test_Cups
  Provides unit tests and examples for the <Cups> lens.
*)

module Test_Cups =

(* Variable: conf *)
let conf = "# Sample configuration file for the CUPS scheduler.
LogLevel warn

# Deactivate CUPS' internal logrotating, as we provide a better one, especially
# LogLevel debug2 gets usable now
MaxLogSize 0

# Administrator user group...
SystemGroup lpadmin


# Only listen for connections from the local machine.
Listen localhost:631
Listen /var/run/cups/cups.sock

# Show shared printers on the local network.
BrowseOrder allow,deny
BrowseAllow all
BrowseLocalProtocols CUPS dnssd
BrowseAddress @LOCAL

# Default authentication type, when authentication is required...
DefaultAuthType Basic

# Web interface setting...
WebInterface Yes

# Restrict access to the server...
<Location />
  Order allow,deny
</Location>

# Restrict access to the admin pages...
<Location /admin>
  Order allow,deny
</Location>

# Restrict access to configuration files...
<Location /admin/conf>
  AuthType Default
  Require user @SYSTEM
  Order allow,deny
</Location>

# Set the default printer/job policies...
<Policy default>
  # Job/subscription privacy...
  JobPrivateAccess default
  JobPrivateValues default
  SubscriptionPrivateAccess default
  SubscriptionPrivateValues default

  # Job-related operations must be done by the owner or an administrator...
  <Limit Create-Job Print-Job Print-URI Validate-Job>
    Order deny,allow
  </Limit>

  <Limit Send-Document Send-URI Hold-Job Release-Job Restart-Job Purge-Jobs Set-Job-Attributes Create-Job-Subscription Renew-Subscription Cancel-Subscription Get-Notifications Reprocess-Job Cancel-Current-Job Suspend-Current-Job Resume-Job Cancel-My-Jobs Close-Job CUPS-Move-Job CUPS-Get-Document>
    Require user @OWNER @SYSTEM
    Order deny,allow
  </Limit>

  # All administration operations require an administrator to authenticate...
  <Limit CUPS-Add-Modify-Printer CUPS-Delete-Printer CUPS-Add-Modify-Class CUPS-Delete-Class CUPS-Set-Default CUPS-Get-Devices>
    AuthType Default
    Require user @SYSTEM
    Order deny,allow
  </Limit>

  # All printer operations require a printer operator to authenticate...
  <Limit Pause-Printer Resume-Printer Enable-Printer Disable-Printer Pause-Printer-After-Current-Job Hold-New-Jobs Release-Held-New-Jobs Deactivate-Printer Activate-Printer Restart-Printer Shutdown-Printer Startup-Printer Promote-Job Schedule-Job-After Cancel-Jobs CUPS-Accept-Jobs CUPS-Reject-Jobs>
    AuthType Default
    Require user @SYSTEM
    Order deny,allow
  </Limit>

  # Only the owner or an administrator can cancel or authenticate a job...
  <Limit Cancel-Job CUPS-Authenticate-Job>
    Require user @OWNER @SYSTEM
    Order deny,allow
  </Limit>

  <Limit All>
    Order deny,allow
  </Limit>
</Policy>

# Set the authenticated printer/job policies...
<Policy authenticated>
  # Job/subscription privacy...
  JobPrivateAccess default
  JobPrivateValues default
  SubscriptionPrivateAccess default
  SubscriptionPrivateValues default

  # Job-related operations must be done by the owner or an administrator...
  <Limit Create-Job Print-Job Print-URI Validate-Job>
    AuthType Default
    Order deny,allow
  </Limit>

  <Limit Send-Document Send-URI Hold-Job Release-Job Restart-Job Purge-Jobs Set-Job-Attributes Create-Job-Subscription Renew-Subscription Cancel-Subscription Get-Notifications Reprocess-Job Cancel-Current-Job Suspend-Current-Job Resume-Job Cancel-My-Jobs Close-Job CUPS-Move-Job CUPS-Get-Document>
    AuthType Default
    Require user @OWNER @SYSTEM
    Order deny,allow
  </Limit>

  # All administration operations require an administrator to authenticate...
  <Limit CUPS-Add-Modify-Printer CUPS-Delete-Printer CUPS-Add-Modify-Class CUPS-Delete-Class CUPS-Set-Default>
    AuthType Default
    Require user @SYSTEM
    Order deny,allow
  </Limit>

  # All printer operations require a printer operator to authenticate...
  <Limit Pause-Printer Resume-Printer Enable-Printer Disable-Printer Pause-Printer-After-Current-Job Hold-New-Jobs Release-Held-New-Jobs Deactivate-Printer Activate-Printer Restart-Printer Shutdown-Printer Startup-Printer Promote-Job Schedule-Job-After Cancel-Jobs CUPS-Accept-Jobs CUPS-Reject-Jobs>
    AuthType Default
    Require user @SYSTEM
    Order deny,allow
  </Limit>

  # Only the owner or an administrator can cancel or authenticate a job...
  <Limit Cancel-Job CUPS-Authenticate-Job>
    AuthType Default
    Require user @OWNER @SYSTEM
    Order deny,allow
  </Limit>

  <Limit All>
    Order deny,allow
  </Limit>
</Policy>
"

(* Test: Simplevars.lns *)
test Cups.lns get conf =
  { "#comment" = "Sample configuration file for the CUPS scheduler." }
  { "directive" = "LogLevel"
    { "arg" = "warn" }
  }
  {  }
  { "#comment" = "Deactivate CUPS' internal logrotating, as we provide a better one, especially" }
  { "#comment" = "LogLevel debug2 gets usable now" }
  { "directive" = "MaxLogSize"
    { "arg" = "0" }
  }
  {  }
  { "#comment" = "Administrator user group..." }
  { "directive" = "SystemGroup"
    { "arg" = "lpadmin" }
  }
  {  }
  {  }
  { "#comment" = "Only listen for connections from the local machine." }
  { "directive" = "Listen"
    { "arg" = "localhost:631" }
  }
  { "directive" = "Listen"
    { "arg" = "/var/run/cups/cups.sock" }
  }
  {  }
  { "#comment" = "Show shared printers on the local network." }
  { "directive" = "BrowseOrder"
    { "arg" = "allow,deny" }
  }
  { "directive" = "BrowseAllow"
    { "arg" = "all" }
  }
  { "directive" = "BrowseLocalProtocols"
    { "arg" = "CUPS" }
    { "arg" = "dnssd" }
  }
  { "directive" = "BrowseAddress"
    { "arg" = "@LOCAL" }
  }
  {  }
  { "#comment" = "Default authentication type, when authentication is required..." }
  { "directive" = "DefaultAuthType"
    { "arg" = "Basic" }
  }
  {  }
  { "#comment" = "Web interface setting..." }
  { "directive" = "WebInterface"
    { "arg" = "Yes" }
  }
  {  }
  { "#comment" = "Restrict access to the server..." }
  { "Location"
    { "arg" = "/" }
    { "directive" = "Order"
      { "arg" = "allow,deny" }
    }
  }
  {  }
  { "#comment" = "Restrict access to the admin pages..." }
  { "Location"
    { "arg" = "/admin" }
    { "directive" = "Order"
      { "arg" = "allow,deny" }
    }
  }
  {  }
  { "#comment" = "Restrict access to configuration files..." }
  { "Location"
    { "arg" = "/admin/conf" }
    { "directive" = "AuthType"
      { "arg" = "Default" }
    }
    { "directive" = "Require"
      { "arg" = "user" }
      { "arg" = "@SYSTEM" }
    }
    { "directive" = "Order"
      { "arg" = "allow,deny" }
    }
  }
  {  }
  { "#comment" = "Set the default printer/job policies..." }
  { "Policy"
    { "arg" = "default" }
    { "#comment" = "Job/subscription privacy..." }
    { "directive" = "JobPrivateAccess"
      { "arg" = "default" }
    }
    { "directive" = "JobPrivateValues"
      { "arg" = "default" }
    }
    { "directive" = "SubscriptionPrivateAccess"
      { "arg" = "default" }
    }
    { "directive" = "SubscriptionPrivateValues"
      { "arg" = "default" }
    }
    {  }
    { "#comment" = "Job-related operations must be done by the owner or an administrator..." }
    { "Limit"
      { "arg" = "Create-Job" }
      { "arg" = "Print-Job" }
      { "arg" = "Print-URI" }
      { "arg" = "Validate-Job" }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "Limit"
      { "arg" = "Send-Document" }
      { "arg" = "Send-URI" }
      { "arg" = "Hold-Job" }
      { "arg" = "Release-Job" }
      { "arg" = "Restart-Job" }
      { "arg" = "Purge-Jobs" }
      { "arg" = "Set-Job-Attributes" }
      { "arg" = "Create-Job-Subscription" }
      { "arg" = "Renew-Subscription" }
      { "arg" = "Cancel-Subscription" }
      { "arg" = "Get-Notifications" }
      { "arg" = "Reprocess-Job" }
      { "arg" = "Cancel-Current-Job" }
      { "arg" = "Suspend-Current-Job" }
      { "arg" = "Resume-Job" }
      { "arg" = "Cancel-My-Jobs" }
      { "arg" = "Close-Job" }
      { "arg" = "CUPS-Move-Job" }
      { "arg" = "CUPS-Get-Document" }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@OWNER" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "All administration operations require an administrator to authenticate..." }
    { "Limit"
      { "arg" = "CUPS-Add-Modify-Printer" }
      { "arg" = "CUPS-Delete-Printer" }
      { "arg" = "CUPS-Add-Modify-Class" }
      { "arg" = "CUPS-Delete-Class" }
      { "arg" = "CUPS-Set-Default" }
      { "arg" = "CUPS-Get-Devices" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "All printer operations require a printer operator to authenticate..." }
    { "Limit"
      { "arg" = "Pause-Printer" }
      { "arg" = "Resume-Printer" }
      { "arg" = "Enable-Printer" }
      { "arg" = "Disable-Printer" }
      { "arg" = "Pause-Printer-After-Current-Job" }
      { "arg" = "Hold-New-Jobs" }
      { "arg" = "Release-Held-New-Jobs" }
      { "arg" = "Deactivate-Printer" }
      { "arg" = "Activate-Printer" }
      { "arg" = "Restart-Printer" }
      { "arg" = "Shutdown-Printer" }
      { "arg" = "Startup-Printer" }
      { "arg" = "Promote-Job" }
      { "arg" = "Schedule-Job-After" }
      { "arg" = "Cancel-Jobs" }
      { "arg" = "CUPS-Accept-Jobs" }
      { "arg" = "CUPS-Reject-Jobs" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "Only the owner or an administrator can cancel or authenticate a job..." }
    { "Limit"
      { "arg" = "Cancel-Job" }
      { "arg" = "CUPS-Authenticate-Job" }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@OWNER" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "Limit"
      { "arg" = "All" }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
  }
  {  }
  { "#comment" = "Set the authenticated printer/job policies..." }
  { "Policy"
    { "arg" = "authenticated" }
    { "#comment" = "Job/subscription privacy..." }
    { "directive" = "JobPrivateAccess"
      { "arg" = "default" }
    }
    { "directive" = "JobPrivateValues"
      { "arg" = "default" }
    }
    { "directive" = "SubscriptionPrivateAccess"
      { "arg" = "default" }
    }
    { "directive" = "SubscriptionPrivateValues"
      { "arg" = "default" }
    }
    {  }
    { "#comment" = "Job-related operations must be done by the owner or an administrator..." }
    { "Limit"
      { "arg" = "Create-Job" }
      { "arg" = "Print-Job" }
      { "arg" = "Print-URI" }
      { "arg" = "Validate-Job" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "Limit"
      { "arg" = "Send-Document" }
      { "arg" = "Send-URI" }
      { "arg" = "Hold-Job" }
      { "arg" = "Release-Job" }
      { "arg" = "Restart-Job" }
      { "arg" = "Purge-Jobs" }
      { "arg" = "Set-Job-Attributes" }
      { "arg" = "Create-Job-Subscription" }
      { "arg" = "Renew-Subscription" }
      { "arg" = "Cancel-Subscription" }
      { "arg" = "Get-Notifications" }
      { "arg" = "Reprocess-Job" }
      { "arg" = "Cancel-Current-Job" }
      { "arg" = "Suspend-Current-Job" }
      { "arg" = "Resume-Job" }
      { "arg" = "Cancel-My-Jobs" }
      { "arg" = "Close-Job" }
      { "arg" = "CUPS-Move-Job" }
      { "arg" = "CUPS-Get-Document" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@OWNER" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "All administration operations require an administrator to authenticate..." }
    { "Limit"
      { "arg" = "CUPS-Add-Modify-Printer" }
      { "arg" = "CUPS-Delete-Printer" }
      { "arg" = "CUPS-Add-Modify-Class" }
      { "arg" = "CUPS-Delete-Class" }
      { "arg" = "CUPS-Set-Default" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "All printer operations require a printer operator to authenticate..." }
    { "Limit"
      { "arg" = "Pause-Printer" }
      { "arg" = "Resume-Printer" }
      { "arg" = "Enable-Printer" }
      { "arg" = "Disable-Printer" }
      { "arg" = "Pause-Printer-After-Current-Job" }
      { "arg" = "Hold-New-Jobs" }
      { "arg" = "Release-Held-New-Jobs" }
      { "arg" = "Deactivate-Printer" }
      { "arg" = "Activate-Printer" }
      { "arg" = "Restart-Printer" }
      { "arg" = "Shutdown-Printer" }
      { "arg" = "Startup-Printer" }
      { "arg" = "Promote-Job" }
      { "arg" = "Schedule-Job-After" }
      { "arg" = "Cancel-Jobs" }
      { "arg" = "CUPS-Accept-Jobs" }
      { "arg" = "CUPS-Reject-Jobs" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "#comment" = "Only the owner or an administrator can cancel or authenticate a job..." }
    { "Limit"
      { "arg" = "Cancel-Job" }
      { "arg" = "CUPS-Authenticate-Job" }
      { "directive" = "AuthType"
        { "arg" = "Default" }
      }
      { "directive" = "Require"
        { "arg" = "user" }
        { "arg" = "@OWNER" }
        { "arg" = "@SYSTEM" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
    {  }
    { "Limit"
      { "arg" = "All" }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
    }
  }

