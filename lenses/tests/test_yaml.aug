module Test_YAML =

(* Inherit test *)
test YAML.lns get "host1:
  <<: *production\n" =
{ "host1"
  { "<<" = "production" }
}

(* top level sequence *)
test YAML.lns get "
- foo: 1
  bar: 2

- baz: 3
  gee: 4
" =
{  }
{ "@sequence"
  { "foo" = "1" }
  { "bar" = "2" }
}
{  }
{ "@sequence"
  { "baz" = "3" }
  { "gee" = "4" }
}

test YAML.lns get "
defaults: &defaults
  repo1: master
  repo2: master

# Live
production: &production
  # repo3: dc89d7a
  repo4:   2d39995
  # repo5: bc4a40d

host1:
  <<: *production

host2:
  <<: *defaults
  repo6: branch1

host3:
  <<: *defaults
  # repo7: branch2
  repo8:   branch3
" =
{}
{ "defaults" = "defaults"
  { "repo1" = "master" }
  { "repo2" = "master" }
}
{}
{ "#comment" = "Live" }
{ "production" = "production"
  { "#comment" = "repo3: dc89d7a" }
  { "repo4" = "2d39995" }
  { "#comment" = "repo5: bc4a40d" }
}
{}
{ "host1"
  { "<<" = "production" }
}
{}
{ "host2"
  { "<<" = "defaults" }
  { "repo6" = "branch1" }
}
{}
{ "host3"
  { "<<" = "defaults" }
  { "#comment" = "repo7: branch2" }
  { "repo8" = "branch3" }
}

(* Ruby YAML header *)
test YAML.lns get "--- !ruby/object:Puppet::Node::Factspress RETURN)\n" =
  { "@yaml" = "!ruby/object:Puppet::Node::Factspress RETURN)" }


(* Continued lines *)
test YAML.lns get "abc:
  def: |-
  ghi
\n" =
  { "abc"
    { "def"
      { "@mval"
        { "@line" = "ghi" } } } }

