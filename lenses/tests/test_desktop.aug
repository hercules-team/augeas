module Test_desktop =

let conf = "# A comment
[Desktop Entry]
Version=1.0
Type=Application
Name=Foo Viewer
# another comment
Comment=The best viewer for Foo objects available!
TryExec=fooview
Exec=fooview %F
Icon=fooview
MimeType=image/x-foo;
X-KDE-Library=libfooview
X-KDE-FactoryName=fooviewfactory
X-KDE-ServiceType=FooService
"

test Desktop.lns get conf =
   { "#comment" = "A comment" }
   { "Desktop Entry"
      { "Version" = "1.0" }
      { "Type" = "Application" }
      { "Name" = "Foo Viewer" }
      { "#comment" = "another comment" }
      { "Comment" = "The best viewer for Foo objects available!" }
      { "TryExec" = "fooview" }
      { "Exec" = "fooview %F" }
      { "Icon" = "fooview" }
      { "MimeType" = "image/x-foo;" }
      { "X-KDE-Library" = "libfooview" }
      { "X-KDE-FactoryName" = "fooviewfactory" }
      { "X-KDE-ServiceType" = "FooService" } }

(* Entries with square brackets *)
test Desktop.lns get "[Desktop Entry]
X-GNOME-FullName[ca]=En canadien
" =
    { "Desktop Entry"
      { "X-GNOME-FullName[ca]" = "En canadien" } }

(* Test: Desktop.lns
     Allow @ in setting (GH issue #92) *)
test Desktop.lns get "[Desktop Entry]
Name[sr@latin] = foobar\n" =
    { "Desktop Entry"
      { "Name[sr@latin]" = "foobar" } }
