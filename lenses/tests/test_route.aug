module Test_route =
	test Route.lns get "10.40.11.102/32 via 10.40.8.1\n10.1.8.0/24 via 10.40.8.254\n" =
	{ "10.40.8.1" = "10.40.11.102/32" }
	{ "10.40.8.254" = "10.1.8.0/24" }

	test Route.lns get "10.40.11.102/32 via 10.40.8.1\n10.1.8.0/24 via 10.40.8.1\n" =
	{ "10.40.8.1" = "10.40.11.102/32" }
	{ "10.40.8.1" = "10.1.8.0/24" }
