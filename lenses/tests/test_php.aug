module Test_php =

   let conf = "[PHP]

;;;;;;;;;;;;;;;;;;;;
; Language Options ;
;;;;;;;;;;;;;;;;;;;;

; Enable the PHP scripting language engine under Apache.
engine = On

; Enable compatibility mode with Zend Engine 1 (PHP 4.x)
zend.ze1_compatibility_mode = Off

; Allow the <? tag.  Otherwise, only <?php and <script> tags are recognized.
; NOTE: Using short tags should be avoided when developing applications or
; libraries that are meant for redistribution, or deployment on PHP
; servers which are not under your control, because short tags may not
; be supported on the target server. For portable, redistributable code,
; be sure not to use short tags.
short_open_tag = On

; Allow ASP-style <% %> tags.
asp_tags = Off

; The number of significant digits displayed in floating point numbers.
precision    =  12

; Enforce year 2000 compliance (will cause problems with non-compliant browsers)
y2k_compliance = On

; Output buffering allows you to send header lines (including cookies) even
; after you send body content, at the price of slowing PHP's output layer a
; bit.  You can enable output buffering during runtime by calling the output
; buffering functions.  You can also enable output buffering for all files by
; setting this directive to On.  If you wish to limit the size of the buffer
; to a certain size - you can use a maximum number of bytes instead of 'On', as
; a value for this directive (e.g., output_buffering=4096).
output_buffering = Off
; You can redirect all of the output of your scripts to a function.  For
; example, if you set output_handler to \"mb_output_handler\", character
; encoding will be transparently converted to the specified encoding.
; Setting any output handler automatically turns on output buffering.
; Note: People who wrote portable scripts should not depend on this ini
;       directive. Instead, explicitly set the output handler using ob_start().
;       Using this ini directive may cause problems unless you know what script
;       is doing.
; Note: You cannot use both \"mb_output_handler\" with \"ob_iconv_handler\"
;       and you cannot use both \"ob_gzhandler\" and \"zlib.output_compression\".
; Note: output_handler must be empty if this is set 'On' !!!!
;       Instead you must use zlib.output_handler.
;output_handler =

; Transparent output compression using the zlib library
; Valid values for this option are 'off', 'on', or a specific buffer size
; to be used for compression (default is 4KB)
; Note: Resulting chunk size may vary due to nature of compression. PHP
;       outputs chunks that are few hundreds bytes each as a result of
;       compression. If you prefer a larger chunk size for better
;       performance, enable output_buffering in addition.
; Note: You need to use zlib.output_handler instead of the standard
;       output_handler, or otherwise the output will be corrupted.
zlib.output_compression = Off
;zlib.output_compression_level = -1

; You cannot specify additional output handlers if zlib.output_compression
; is activated here. This setting does the same as output_handler but in
; a different order.
;zlib.output_handler =

; Implicit flush tells PHP to tell the output layer to flush itself
; automatically after every output block.  This is equivalent to calling the
; PHP function flush() after each and every call to print() or echo() and each
; and every HTML block.  Turning this option on has serious performance
; implications and is generally recommended for debugging purposes only.
implicit_flush = Off

; The unserialize callback function will be called (with the undefined class'
; name as parameter), if the unserializer finds an undefined class
; which should be instantiated.
; A warning appears if the specified function is not defined, or if the
; function doesn't include/implement the missing class.
; So only set this entry, if you really want to implement such a
; callback-function.
unserialize_callback_func=

;extension=mysql.so
;extension=gd.so

;;;;;;;;;;;;;;;;;;;
; Module Settings ;
;;;;;;;;;;;;;;;;;;;

[Date]
; Defines the default timezone used by the date functions
;date.timezone =

;date.default_latitude = 31.7667
;date.default_longitude = 35.2333

;date.sunrise_zenith = 90.583333
;date.sunset_zenith = 90.583333

[filter]
;filter.default = unsafe_raw
;filter.default_flags =

[iconv]
;iconv.input_encoding = ISO-8859-1
;iconv.internal_encoding = ISO-8859-1
;iconv.output_encoding = ISO-8859-1

[sqlite]
;sqlite.assoc_case = 0


"


   test PHP.lns get conf = ?
