module Test_json =

let lns = Json.lns

test lns get "\"menu\"" = { "string" = "menu" }

test lns get "true" = { "const" = "true" }

test lns get "3.141" = { "number" = "3.141" }

test lns get "{ \"key\" : 666 }" =
  { "dict" { "entry" = "key" { "number" = "666" } } }

test lns get "[true, 0, \"yo\"]" =
  { "array" { "const" = "true" } { "number" = "0" } { "string" = "yo" } }

test lns get "{\"a\" : true}" =
  { "dict" { "entry" = "a" { "const" = "true" } } }

test lns get "{ \"0\":true, \"1\":false }" =
  { "dict" { "entry" = "0" { "const" = "true" } }
             { "entry" = "1" { "const" = "false" } } }


test lns get "{ \"0\": true, \"1\":false }" =
  { "dict"
    { "entry" = "0" { "const" = "true" } }
    { "entry" = "1" { "const" = "false" } } }

test lns get "{\"menu\": \"entry one\"}" =
  { "dict" { "entry" = "menu" { "string" = "entry one" } } }

test lns get "[ ]" =
  { "array" }

test lns get "{}" =
  { "dict" }

let s = "{\"menu\": {
  \"id\": \"file\",
  \"value\": \"File\",
  \"popup\": {
    \"menuitem\": [
      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},
      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},
      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}
    ]
  }
}}"

test lns get s =
  { "dict"
    { "entry" = "menu"
      { "dict"
        { "entry" = "id" { "string" = "file" } }
        { "entry" = "value" { "string" = "File" } }
        { "entry" = "popup"
          { "dict"
            { "entry" = "menuitem"
              { "array"
                { "dict"
                  { "entry" = "value" { "string" = "New" } }
                  { "entry" = "onclick"
                    { "string" = "CreateNewDoc()" } } }
                { "dict"
                  { "entry" = "value" { "string" = "Open" } }
                  { "entry" = "onclick" { "string" = "OpenDoc()" } } }
                { "dict"
                  { "entry" = "value" { "string" = "Close" } }
                  { "entry" = "onclick" { "string" = "CloseDoc()" } }
                } } } } } } } }

let t = "
{\"web-app\": {
  \"servlet\": [
    {
      \"servlet-name\": \"cofaxCDS\",
      \"servlet-class\": \"org.cofax.cds.CDSServlet\",
      \"init-param\": {
        \"configGlossary:installationAt\": \"Philadelphia, PA\",
        \"configGlossary:adminEmail\": \"ksm@pobox.com\",
        \"configGlossary:poweredBy\": \"Cofax\",
        \"configGlossary:poweredByIcon\": \"/images/cofax.gif\",
        \"configGlossary:staticPath\": \"/content/static\",
        \"templateProcessorClass\": \"org.cofax.WysiwygTemplate\",
        \"templateLoaderClass\": \"org.cofax.FilesTemplateLoader\",
        \"templatePath\": \"templates\",
        \"templateOverridePath\": \"\",
        \"defaultListTemplate\": \"listTemplate.htm\",
        \"defaultFileTemplate\": \"articleTemplate.htm\",
        \"useJSP\": false,
        \"jspListTemplate\": \"listTemplate.jsp\",
        \"jspFileTemplate\": \"articleTemplate.jsp\",
        \"cachePackageTagsTrack\": 200,
        \"cachePackageTagsStore\": 200,
        \"cachePackageTagsRefresh\": 60,
        \"cacheTemplatesTrack\": 100,
        \"cacheTemplatesStore\": 50,
        \"cacheTemplatesRefresh\": 15,
        \"cachePagesTrack\": 200,
        \"cachePagesStore\": 100,
        \"cachePagesRefresh\": 10,
        \"cachePagesDirtyRead\": 10,
        \"searchEngineListTemplate\": \"forSearchEnginesList.htm\",
        \"searchEngineFileTemplate\": \"forSearchEngines.htm\",
        \"searchEngineRobotsDb\": \"WEB-INF/robots.db\",
        \"useDataStore\": true,
        \"dataStoreClass\": \"org.cofax.SqlDataStore\",
        \"redirectionClass\": \"org.cofax.SqlRedirection\",
        \"dataStoreName\": \"cofax\",
        \"dataStoreDriver\": \"com.microsoft.jdbc.sqlserver.SQLServerDriver\",
        \"dataStoreUrl\": \"jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon\",
        \"dataStoreUser\": \"sa\",
        \"dataStorePassword\": \"dataStoreTestQuery\",
        \"dataStoreTestQuery\": \"SET NOCOUNT ON;select test='test';\",
        \"dataStoreLogFile\": \"/usr/local/tomcat/logs/datastore.log\",
        \"dataStoreInitConns\": 10,
        \"dataStoreMaxConns\": 100,
        \"dataStoreConnUsageLimit\": 100,
        \"dataStoreLogLevel\": \"debug\",
        \"maxUrlLength\": 500}},
    {
      \"servlet-name\": \"cofaxEmail\",
      \"servlet-class\": \"org.cofax.cds.EmailServlet\",
      \"init-param\": {
      \"mailHost\": \"mail1\",
      \"mailHostOverride\": \"mail2\"}},
    {
      \"servlet-name\": \"cofaxAdmin\",
      \"servlet-class\": \"org.cofax.cds.AdminServlet\"},

    {
      \"servlet-name\": \"fileServlet\",
      \"servlet-class\": \"org.cofax.cds.FileServlet\"},
    {
      \"servlet-name\": \"cofaxTools\",
      \"servlet-class\": \"org.cofax.cms.CofaxToolsServlet\",
      \"init-param\": {
        \"templatePath\": \"toolstemplates/\",
        \"log\": 1,
        \"logLocation\": \"/usr/local/tomcat/logs/CofaxTools.log\",
        \"logMaxSize\": \"\",
        \"dataLog\": 1,
        \"dataLogLocation\": \"/usr/local/tomcat/logs/dataLog.log\",
        \"dataLogMaxSize\": \"\",
        \"removePageCache\": \"/content/admin/remove?cache=pages&id=\",
        \"removeTemplateCache\": \"/content/admin/remove?cache=templates&id=\",
        \"fileTransferFolder\": \"/usr/local/tomcat/webapps/content/fileTransferFolder\",
        \"lookInContext\": 1,
        \"adminGroupID\": 4,
        \"betaServer\": true}}],
  \"servlet-mapping\": {
    \"cofaxCDS\": \"/\",
    \"cofaxEmail\": \"/cofaxutil/aemail/*\",
    \"cofaxAdmin\": \"/admin/*\",
    \"fileServlet\": \"/static/*\",
    \"cofaxTools\": \"/tools/*\"},

  \"taglib\": {
    \"taglib-uri\": \"cofax.tld\",
    \"taglib-location\": \"/WEB-INF/tlds/cofax.tld\"}}}"

test lns get t =
  { "dict"
    { "entry" = "web-app"
      { "dict"
        { "entry" = "servlet"
          { "array"
            { "dict"
              { "entry" = "servlet-name" { "string" = "cofaxCDS" } }
              { "entry" = "servlet-class"
                { "string" = "org.cofax.cds.CDSServlet" } }
              { "entry" = "init-param"
                { "dict"
                  { "entry" = "configGlossary:installationAt"
                    { "string" = "Philadelphia, PA" } }
                  { "entry" = "configGlossary:adminEmail"
                    { "string" = "ksm@pobox.com" } }
                  { "entry" = "configGlossary:poweredBy"
                    { "string" = "Cofax" } }
                  { "entry" = "configGlossary:poweredByIcon"
                    { "string" = "/images/cofax.gif" } }
                  { "entry" = "configGlossary:staticPath"
                    { "string" = "/content/static" } }
                  { "entry" = "templateProcessorClass"
                    { "string" = "org.cofax.WysiwygTemplate" } }
                  { "entry" = "templateLoaderClass"
                    { "string" = "org.cofax.FilesTemplateLoader" } }
                  { "entry" = "templatePath"
                    { "string" = "templates" } }
                  { "entry" = "templateOverridePath"
                    { "string" = "" } }
                  { "entry" = "defaultListTemplate"
                    { "string" = "listTemplate.htm" } }
                  { "entry" = "defaultFileTemplate"
                    { "string" = "articleTemplate.htm" } }
                  { "entry" = "useJSP"
                    { "const" = "false" } }
                  { "entry" = "jspListTemplate"
                    { "string" = "listTemplate.jsp" } }
                  { "entry" = "jspFileTemplate"
                    { "string" = "articleTemplate.jsp" } }
                  { "entry" = "cachePackageTagsTrack"
                    { "number" = "200" } }
                  { "entry" = "cachePackageTagsStore"
                    { "number" = "200" } }
                  { "entry" = "cachePackageTagsRefresh"
                    { "number" = "60" } }
                  { "entry" = "cacheTemplatesTrack"
                    { "number" = "100" } }
                  { "entry" = "cacheTemplatesStore"
                    { "number" = "50" } }
                  { "entry" = "cacheTemplatesRefresh"
                    { "number" = "15" } }
                  { "entry" = "cachePagesTrack"
                    { "number" = "200" } }
                  { "entry" = "cachePagesStore"
                    { "number" = "100" } }
                  { "entry" = "cachePagesRefresh"
                    { "number" = "10" } }
                  { "entry" = "cachePagesDirtyRead"
                    { "number" = "10" } }
                  { "entry" = "searchEngineListTemplate"
                    { "string" = "forSearchEnginesList.htm" } }
                  { "entry" = "searchEngineFileTemplate"
                    { "string" = "forSearchEngines.htm" } }
                  { "entry" = "searchEngineRobotsDb"
                    { "string" = "WEB-INF/robots.db" } }
                  { "entry" = "useDataStore"
                    { "const" = "true" } }
                  { "entry" = "dataStoreClass"
                    { "string" = "org.cofax.SqlDataStore" } }
                  { "entry" = "redirectionClass"
                    { "string" = "org.cofax.SqlRedirection" } }
                  { "entry" = "dataStoreName"
                    { "string" = "cofax" } }
                  { "entry" = "dataStoreDriver"
                    { "string" = "com.microsoft.jdbc.sqlserver.SQLServerDriver" } }
                  { "entry" = "dataStoreUrl"
                    { "string" = "jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon" } }
                  { "entry" = "dataStoreUser"
                    { "string" = "sa" } }
                  { "entry" = "dataStorePassword"
                    { "string" = "dataStoreTestQuery" } }
                  { "entry" = "dataStoreTestQuery"
                    { "string" = "SET NOCOUNT ON;select test='test';" } }
                  { "entry" = "dataStoreLogFile"
                    { "string" = "/usr/local/tomcat/logs/datastore.log" } }
                  { "entry" = "dataStoreInitConns"
                    { "number" = "10" } }
                  { "entry" = "dataStoreMaxConns"
                    { "number" = "100" } }
                  { "entry" = "dataStoreConnUsageLimit"
                    { "number" = "100" } }
                  { "entry" = "dataStoreLogLevel"
                    { "string" = "debug" } }
                  { "entry" = "maxUrlLength"
                    { "number" = "500" } } } } }
            { "dict"
              { "entry" = "servlet-name"
                { "string" = "cofaxEmail" } }
              { "entry" = "servlet-class"
                { "string" = "org.cofax.cds.EmailServlet" } }
              { "entry" = "init-param"
                { "dict"
                  { "entry" = "mailHost"
                    { "string" = "mail1" } }
                  { "entry" = "mailHostOverride"
                    { "string" = "mail2" } } } } }
            { "dict"
              { "entry" = "servlet-name"
                { "string" = "cofaxAdmin" } }
              { "entry" = "servlet-class"
                { "string" = "org.cofax.cds.AdminServlet" } } }
            { "dict"
              { "entry" = "servlet-name"
                { "string" = "fileServlet" } }
              { "entry" = "servlet-class"
                { "string" = "org.cofax.cds.FileServlet" } } }
            { "dict"
              { "entry" = "servlet-name"
                { "string" = "cofaxTools" } }
              { "entry" = "servlet-class"
                { "string" = "org.cofax.cms.CofaxToolsServlet" } }
              { "entry" = "init-param"
                { "dict"
                  { "entry" = "templatePath"
                    { "string" = "toolstemplates/" } }
                  { "entry" = "log"
                    { "number" = "1" } }
                  { "entry" = "logLocation"
                    { "string" = "/usr/local/tomcat/logs/CofaxTools.log" } }
                  { "entry" = "logMaxSize"
                    { "string" = "" } }
                  { "entry" = "dataLog"
                    { "number" = "1" } }
                  { "entry" = "dataLogLocation"
                    { "string" = "/usr/local/tomcat/logs/dataLog.log" } }
                  { "entry" = "dataLogMaxSize"
                    { "string" = "" } }
                  { "entry" = "removePageCache"
                    { "string" = "/content/admin/remove?cache=pages&id=" } }
                  { "entry" = "removeTemplateCache"
                    { "string" = "/content/admin/remove?cache=templates&id=" } }
                  { "entry" = "fileTransferFolder"
                    { "string" = "/usr/local/tomcat/webapps/content/fileTransferFolder" } }
                  { "entry" = "lookInContext"
                    { "number" = "1" } }
                  { "entry" = "adminGroupID"
                    { "number" = "4" } }
                  { "entry" = "betaServer"
                    { "const" = "true" } } } } } } }
        { "entry" = "servlet-mapping"
          { "dict"
            { "entry" = "cofaxCDS"
              { "string" = "/" } }
            { "entry" = "cofaxEmail"
              { "string" = "/cofaxutil/aemail/*" } }
            { "entry" = "cofaxAdmin"
              { "string" = "/admin/*" } }
            { "entry" = "fileServlet"
              { "string" = "/static/*" } }
            { "entry" = "cofaxTools"
              { "string" = "/tools/*" } } } }
        { "entry" = "taglib"
          { "dict"
            { "entry" = "taglib-uri"
              { "string" = "cofax.tld" } }
            { "entry" = "taglib-location"
              { "string" = "/WEB-INF/tlds/cofax.tld" } } } } } } }
