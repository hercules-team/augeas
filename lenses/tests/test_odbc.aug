module Test_odbc =

   let conf = "
# Example driver definitinions
#
#

# Included in the unixODBC package
[PostgreSQL]
Description = ODBC for PostgreSQL
Driver    = /usr/lib/libodbcpsql.so
Setup   = /usr/lib/libodbcpsqlS.so
FileUsage = 1

[MySQL]
# Driver from the MyODBC package
# Setup from the unixODBC package
Description  = ODBC for MySQL
Driver   = /usr/lib/libmyodbc.so
Setup    = /usr/lib/libodbcmyS.so
FileUsage  = 1
"

test Odbc.lns get conf =
    {  }
    { "#comment" = "Example driver definitinions" }
    {}
    {}
    {  }
    { "#comment" = "Included in the unixODBC package" }

    { "PostgreSQL"
        { "Description" = "ODBC for PostgreSQL" }
        { "Driver" = "/usr/lib/libodbcpsql.so" }
        { "Setup" = "/usr/lib/libodbcpsqlS.so" }
        { "FileUsage" = "1" }
        {}
    }
    { "MySQL"
        { "#comment" = "Driver from the MyODBC package" }
        { "#comment" = "Setup from the unixODBC package" }
        { "Description" = "ODBC for MySQL" }
        { "Driver" = "/usr/lib/libmyodbc.so" }
        { "Setup" = "/usr/lib/libodbcmyS.so" }
        { "FileUsage" = "1" }
    }

test Odbc.lns put conf after
    set "MySQL/Driver" "/usr/lib64/libmyodbc3.so";
    set "MySQL/Driver/#comment" "note the path" = "
# Example driver definitinions
#
#

# Included in the unixODBC package
[PostgreSQL]
Description = ODBC for PostgreSQL
Driver    = /usr/lib/libodbcpsql.so
Setup   = /usr/lib/libodbcpsqlS.so
FileUsage = 1

[MySQL]
# Driver from the MyODBC package
# Setup from the unixODBC package
Description  = ODBC for MySQL
Driver   = /usr/lib64/libmyodbc3.so#note the path
Setup    = /usr/lib/libodbcmyS.so
FileUsage  = 1
"
