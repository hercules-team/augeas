module Test_spacevars = 

    let conf ="# This a spaced key/value configuration file.
keyword value

# I like comments
very.useful-key my=value

		
"

    test Spacevars.lns get conf =
        { "#comment" = "This a spaced key/value configuration file."}
        { "keyword" = "value" }
        {}
        { "#comment" = "I like comments"}
        { "very.useful-key" = "my=value" }
	{}
	{}
