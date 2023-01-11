<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?php

  $title = "Download WorldView";
  $description = "PYXIS | ";
  $keywords = "";
?>
<?php $thisPage="iframe"; 
  $thissubPage="iframe"
?>
<style>
body {
  margin-left: 0px;
  margin-top: 0px;
  padding-left: 0px;
  padding-top: 0px;
}
#shadow { 
  box-shadow: 5px 5px 5px #666;
  -moz-box-shadow: 5px 5px 5px #666;
  -webkit-box-shadow: 5px 5px 5px #666;
  margin: 0 0;
  background: #ccc;
}
</style>
<script type="text/javascript" src="http://code.jquery.com/jquery-1.5.2.js"></script>
<script type="text/javascript">
<!--
function doDownload(href)
{
  var params={}
  clientKey=getURLParameter('key');
  if(clientKey!="null")
    params['key']=clientKey;	

	$.post("../data/admin/logDownload.php", params, function(data) {
		//need to launch the download after the async event or the response was overwritten
		window.location = href;
	})
  .error( function() {
    // still allow them to download if something is wrong
    window.location = href;
  });
}

function initPage()
{
  clientKey=getURLParameter('key');
  if(clientKey!="null")
        {
                var href = $('a').attr('href');
                $('a').attr('href', href + '?key=' + clientKey);
        }
  
  $('a').click(function(event) { 
    var href = this.href;
    event.preventDefault();
    doDownload(href); 
    });
}

function getURLParameter(name) {
    return decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(location.search)||[,null])[1]
    );
}
//-->
</script>

</head>
<body onload="initPage()">
<!--  CONTENTS  -->
  <div id="shadow">
      <a href="../Downloads/WorldViewInstaller.exe" type="application/octet-stream">
        <img border="0" src="../images/wvbutton.png">
      </a>
  </div>
</body>
</html>
