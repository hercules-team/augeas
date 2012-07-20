(* Test for keepalived lens *)

module Test_mke2fs =

   let conf = "# This is a comment 
; and another comment

[defaults]
	base_features = sparse_super,filetype,resize_inode,dir_index,ext_attr
	default_mntopts = acl,user_xattr
	enable_periodic_fsck = 0
	blocksize = 4096
	inode_size = 256
        ; here goes inode_ratio
	inode_ratio = 16384

[fs_types]
 ; here we have fs_types
	ext4dev = {
                # this is ext4dev conf

		features = has_journal,^extent
		auto_64-bit_support = 1
		inode_size = 256
		options = test_fs=1
	}
	small = {
		blocksize = 1024
		inode_size = 128
		inode_ratio = 4096
	}
	largefile = {
		inode_ratio = 1048576
		blocksize = -1
	}
"

   test Mke2fs.lns get conf =
     { "#comment" = "This is a comment" }
     { "#comment" = "and another comment" }
     {}
     { "defaults"
        { "base_features"
             { "sparse_super" }
             { "filetype" }
             { "resize_inode" }
             { "dir_index" }
             { "ext_attr" } }
        { "default_mntopts"
             { "acl" }
             { "user_xattr" } }
        { "enable_periodic_fsck" = "0" }
        { "blocksize" = "4096" }
        { "inode_size" = "256" }
        { "#comment" = "here goes inode_ratio" }
        { "inode_ratio" = "16384" }
        {} }
     { "fs_types"
        { "#comment" = "here we have fs_types" } 
        { "filesystem" = "ext4dev"
             { "#comment" = "this is ext4dev conf" }
             {}
             { "features"
                { "has_journal" }
                { "extent"
                   { "disable" } } }
             { "auto_64-bit_support" = "1" }
             { "inode_size" = "256" }
             { "options"
                { "test_fs" = "1" } } }
        { "filesystem" = "small"
             { "blocksize" = "1024" }
             { "inode_size" = "128" }
             { "inode_ratio" = "4096" } }
        { "filesystem" = "largefile"
             { "inode_ratio" = "1048576" }
             { "blocksize" = "-1" } } }


test Mke2fs.fs_types_entry
   put "features = has_journal,^extent\n"
   after set "/features/has_journal/disable" "";
   rm "/features/extent/disable" = "features = ^has_journal,extent\n"

