<?php 

  $title = "WorldView";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="products"; 
  $thissubPage="worldview"
?>

</head>
<body>
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">
    
    <div class="contents">
      <div id="left_col">
        <div class="inner">
          <p id="more">Haven't found the solution that you're looking for?<br/>Take a look at our upcoming products</p>
          <a id="streamserver" class="download-button" href="geoweb-streamserver.php">GeoWeb Streamserver<br/>publish your Geospacial data<span class="background"></span></a>
          <!--<a id="earthengine" class="download-button" href=".php">GeoWeb Streamserver<br/>manage<span class="background"></span></a>-->
        </div>
      </div>
      <div id="main_content">
        <a id="worldview" class="download-button" href="downloads.php">Worldview<span class="sup">TM</span>GeoWeb Browser<br/>Free Download<span class="background"></span></a>
        <h1>WorldView&#153<br/>GeoWeb Browser<br/><span id="subtitle">integrated geospatial intelligence for everyone</span></h1>
        
        <div id="main_content_top">
          <div id="intro">

            <p>Information is critical to your success. WorldView<span class="sup">TM</span> opens a world of new possibilities</p> 
            <ul>
              <li>WorldView&#153 is a browser for geographic data</li>
              <li>WorldView&#153 is a search engine driven by location</li> 
              <li>WorldView&#153 uses Earth to display web content</li>
            </ul>
            <p><a href="downloads.php">Build your own worldview now >></a></p>
        
          </div>
          <div id="features">
            <h2>Features</h2>
            <ul>
              <li>Free Download</li>
              <li>Multi-source Virtual Globe</li>
              <li>Location-based Search Engine</li>
              <li>Secure Data Publication Service</li>
              <li>GeoWeb Mashup Engine</li>
              <li>Geospatial Analysis and Modeling Tools</li>
              <li>Real-time GeoData Management System</li>
              <li>Plug-in and Scripting Capabilities</li>
            </ul>
          </div>
        </div>  
        
        <div id="frame">
          <div id="inner-top">
            <div id="inner-bottom">
              
              <!--url's used in the movie-->
              <!--<a href="http://www.pyxisinnovation.com/Downloads/WorldViewInstaller.exe"></a>-->
              <script type='text/javascript' src='scripts/swfobject.js'></script>

              <script type="text/javascript">
              var so = new SWFObject('worldview.swf','wv','668','189','9',"#e9eef0");     
              so.write('inner-bottom');
              </script>
              
              <div id="frame_content">
              <!--<p>Visible content text goes here</p>--></div>
              
              <!-- use this div to put any text that is in the image for indexing -->
              <!--<div id="hidden">
                <p>Search and Discover<br/>Have you ever wondered: ...What is HERE?</p>
                <p>UNDERSTAND & ILLUMINATE<br/>...What's going on? Where is this leading?
                </p>
                <p>ORGANIZE & SHARE<br/>...How can I contribute and engage others?
                </p>
              </div>-->
            </div>
          </div>
        </div>
        
      </div>
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
