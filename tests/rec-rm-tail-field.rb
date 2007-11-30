# -*- ruby -*-
commands="
rm /system/config/pam/newrole/0/opts
save
"
    
diff = {}
diff["/etc/pam.d/newrole"] = ""
