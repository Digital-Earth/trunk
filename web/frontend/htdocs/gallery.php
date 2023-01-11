<?php

  $title = "Download Button Gallery";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="gallery"; 
  $thissubPage="gallery"
?>

<script type="text/javascript">
<!--
function getURLParameter(name) {
    return decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(location.search)||[,null])[1]
    );
}

settings = new Object;
settings['default']={'width':339,'height':76,'radius':0};

function writeEmbeddedText()
{
	selectedSize='default';
	clientKey=getURLParameter('key');
	text="<textarea readonly rows=\"5\" style=\"width:561px\"><div style=\"overflow:hidden;";
	text += "border-radius:" + settings[selectedSize].radius + "px;width:" + settings[selectedSize].width + "px;height:" + settings[selectedSize].height + "px;\">";
	text += "<iframe src=\"http://www.pyxisinnovation.com/hosting/iframe.php";
	if(clientKey != "null")
  {
    text += "?key=" + clientKey + "\"";
    $('iframe').attr('src',$('iframe').attr('src') + '?key=' + clientKey)
	}
	text += " name=\"iframe_pyxis\" ";
	text += "width=\"" + settings[selectedSize].width + "\" height=\"" + settings[selectedSize].height + "\"";
 	text = text +  " frameborder=\"0\" scrolling=\"no\">";
	text=text+"</iframe></div></textarea>";
	
	$('#embedText').html(text);
	$('#embedCode').show();
}
//-->
</script>

</head>
<body onload="writeEmbeddedText()">
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">  
    <div class="contents downloads">
      <div id="main_content">  
        <h1>Download Button Gallery</h1>
        <h4>This is the official PYXIS WorldView&#153 GeoWeb Browser download button:</h4>
              <br />
              <div align="center">
	      <div style="overflow:hidden;border-radius:0px;width:339px;height:76px;"><iframe src="http://www.pyxisinnovation.com/hosting/iframe.php" name="iframe_pyxis" width="339px" height="76px" frameborder="0" scrolling="no"></iframe></div>
              </div>
              <br />
              <div id="embedCode" style="display:none">
	      <h4>Embed the following code into your website to include the official download button in your site:</h4>
              <br />
	      <div id="embedText">
	      <textarea rows="5" cols="110">
	      </textarea>
	      </div>
	      </div>
	<!--<p class="top-border">License info goes here.</p>-->
        <br/><br/><br/>
      </div> 
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
