module Test_oz =

   let conf = "
[paths]
output_dir = /var/lib/libvirt/images
data_dir = /var/lib/oz

[libvirt]
uri = qemu:///system
image_type = raw
"

   test Oz.lns get conf =
      {}
      { "paths"
         { "output_dir" = "/var/lib/libvirt/images" }
         { "data_dir" = "/var/lib/oz" }
         {} }
      { "libvirt"
         { "uri" = "qemu:///system" }
         { "image_type" = "raw" }
         }

    test Oz.lns put conf after
       set "libvirt/cpus" "2"
    = "
[paths]
output_dir = /var/lib/libvirt/images
data_dir = /var/lib/oz

[libvirt]
uri = qemu:///system
image_type = raw
cpus=2
"

