<?php 

  $title = "Downloads";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="downloads"; 
  $thissubPage="downloads"
?>
<script type="text/javascript" src="http://code.jquery.com/jquery-1.5.2.js"></script>
<script type="text/javascript">
function doDownload(href)
{
  var params={}
  params['key']='cb640524-df44-11e2-9cdd-002590a57c44';
	$.post("data/admin/logDownload.php", params, function(data) {
		//need to launch the download after the async event or the response was overwritten
		window.location = href;
	})
	.error( function() {
		window.location = href;
	});
}

function initPage()
{
	$('a[id=worldview-download]').click(function(event) {
		var href = this.href;
		event.preventDefault();
		doDownload(href);
	});
}
</script>
</head>
<body  onload="initPage()">
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">  
    <div class="contents downloads">
      <div id="main_content">  
        <h1>Downloads</h1>
        <h2>Download the latest version of WorldView&#153 GeoWeb Browser.</h2>
	<h2>Note: this is Beta software demonstrating data publication capabilities on the PYXNet P2P GeoWeb platform.</h2>
        <dl>
          <dt>Version &#946;eta 0.10.0.484</dt>
          <dd>3.55MB</dd>
          <dt>(For Windows Vista, 7, and 8)</dt>
        </dl>
        <a href="/Downloads/WorldViewInstaller.exe" id="worldview-download" class="download-main">Download</a>
        <!--<p class="top-border">License info goes here.</p>-->
        <br/><br/><br/>
      </div> 
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
