module Test_xymon =

let conf = "
#atest comment

title test title
page page1 'This is a test page'
1.1.1.1 testhost.localdomain # test1 test2 http:443 ldaps=testhost.localdomain http://testhost.localdomain
2.2.2.2 testhost2.local.domain # COMMENT:stuff apache=wow
#test comment

page newpage
1.1.1.1  testhost.localdomain # test1 test2 http:443 ldaps=testhost.localdomain http://testhost.localdomain
2.2.2.2     testhost2.local.domain # COMMENT:stuff apache=wow

title test title
group group1
3.3.3.3 host1 #
4.4.4.4 host2 #

subparent page1 page2 This is after page 1
10.0.0.1 router1.loni.org #
10.0.0.2 sw1.localdomain #

"
test Xymon.lns get conf =
  {  }
  { "#comment" = "atest comment" }
  {  }
  { "title" = "test title" }
  { "page" = "page1"
    { "pagetitle" = "'This is a test page'" }
    { "host"
      { "ip" = "1.1.1.1" }
      { "fqdn" = "testhost.localdomain" }
      { "tag" = "test1" }
      { "tag" = "test2" }
      { "tag" = "http:443" }
      { "tag" = "ldaps=testhost.localdomain" }
      { "tag" = "http://testhost.localdomain" }
    }
    { "host"
      { "ip" = "2.2.2.2" }
      { "fqdn" = "testhost2.local.domain" }
      { "tag" = "COMMENT:stuff" }
      { "tag" = "apache=wow" }
    }
    { "#comment" = "test comment" }
    {  }
  }
  { "page" = "newpage"
    { "host"
      { "ip" = "1.1.1.1" }
      { "fqdn" = "testhost.localdomain" }
      { "tag" = "test1" }
      { "tag" = "test2" }
      { "tag" = "http:443" }
      { "tag" = "ldaps=testhost.localdomain" }
      { "tag" = "http://testhost.localdomain" }
    }
    { "host"
      { "ip" = "2.2.2.2" }
      { "fqdn" = "testhost2.local.domain" }
      { "tag" = "COMMENT:stuff" }
      { "tag" = "apache=wow" }
    }
    {  }
    { "title" = "test title" }
    { "group" = "group1"
      { "host"
        { "ip" = "3.3.3.3" }
        { "fqdn" = "host1" }
      }
      { "host"
        { "ip" = "4.4.4.4" }
        { "fqdn" = "host2" }
      }
      {  }
    }
  }
  { "subparent" = "page2"
    { "parent" = "page1" }
    { "pagetitle" = "This is after page 1" }
    { "host"
      { "ip" = "10.0.0.1" }
      { "fqdn" = "router1.loni.org" }
    }
    { "host"
      { "ip" = "10.0.0.2" }
      { "fqdn" = "sw1.localdomain" }
    }
    {  }
  }


  test Xymon.host get "192.168.1.1 server1.test.example.com # tag1 tag2 CLASS:classname CLIENT:clienthostname NOCOLUMNS:column1,column2\n" =
  { "host"
    { "ip" = "192.168.1.1" }
    { "fqdn" = "server1.test.example.com" }
    { "tag" = "tag1" }
    { "tag" = "tag2" }
    { "tag" = "CLASS:classname" }
    { "tag" = "CLIENT:clienthostname" }
    { "tag" = "NOCOLUMNS:column1,column2" }
  }

  test Xymon.host get "192.168.1.1 test.example.com # \n" = 
  { "host"
    { "ip" = "192.168.1.1" }
    { "fqdn" = "test.example.com" }
  }
  
  test Xymon.host get "192.168.1.1 test.example.com # http://google.com COMMENT:asdf\n" = 
  { "host"
    { "ip" = "192.168.1.1" }
    { "fqdn" = "test.example.com" }
    { "tag" = "http://google.com" }
    { "tag" = "COMMENT:asdf" }
  }

  test Xymon.include get "include file1.txt\n" = 
  { "include" = "file1.txt" }
  
  test Xymon.include get "directory dir2\n" = 
  { "directory" = "dir2" }

  test Xymon.page get "page page1 page 1 title is here\n" =
  { "page" = "page1"
    { "pagetitle" = "page 1 title is here" }
  }
  
  test Xymon.page get "page page2\n" =
  { "page" = "page2"
  }
  
  test Xymon.subparent get "subparent page1 page2 PAGETITLE 1\n1.1.1.1 host1.lan #\n2.2.2.2 host2.lan #   \n" =
  { "subparent" = "page2"
    { "parent" = "page1" }
    { "pagetitle" = "PAGETITLE 1" }
    { "host"
      { "ip" = "1.1.1.1" }
      { "fqdn" = "host1.lan" }
    }
    { "host"
      { "ip" = "2.2.2.2" }
      { "fqdn" = "host2.lan" }
    }
  }

  test Xymon.title get "title title 1 goes here\n" =
  { "title" = "title 1 goes here" }

  test Xymon.lns get "page page1\ninclude file1.cfg\nsubparent page1 page2\ninclude page2.cfg\n" =
  { "page" = "page1"
    { "include" = "file1.cfg" }
  }
  { "subparent" = "page2"
    { "parent" = "page1" }
    { "include" = "page2.cfg" }
  }


