module Test_cobblersettings =

test Cobblersettings.lns get "Simple_Setting: Value \n" =
  { "Simple_Setting" = "Value" }

test Cobblersettings.lns get "Simple_Setting2: 'Value2@acme.com' \n" =
  { "Simple_Setting2" = "'Value2@acme.com'" }

test Cobblersettings.lns get "Simple_Setting3: ''\n" =
  { "Simple_Setting3" = "''" }

test Cobblersettings.lns get "Simple_Setting4: \"\"\n" =
  { "Simple_Setting4" = "\"\"" }

test Cobblersettings.lns get "Simple_Setting_Trailing_Space : Value \n" =
  { "Simple_Setting_Trailing_Space" = "Value" }

test Cobblersettings.lns get "Setting_List:[Value1, Value2, Value3]\n" =
  { "Setting_List"
      { "sequence"
          { "item" = "Value1" }
          { "item" = "Value2" }
          { "item" = "Value3" } } }

test Cobblersettings.lns get "Empty_Setting_List: []\n" =
  { "Empty_Setting_List"
      { "sequence" } }

test Cobblersettings.lns get "# Commented_Out_Setting: 'some value'\n" =
  { "#comment" = "Commented_Out_Setting: 'some value'" }

test Cobblersettings.lns get "---\n" =
  { "---" = "---"}

test Cobblersettings.lns get "Nested_Setting:\n Test: Value\n" =
  { "Nested_Setting"
      { "Test" = "Value" } }

test Cobblersettings.lns get "Nested_Setting:\n - Test \n" =
  { "Nested_Setting"
      { "list"
          { "value" = "Test" } } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
