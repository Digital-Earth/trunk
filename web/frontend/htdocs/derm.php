<?php 

  $title = "DERM";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="technology"; 
  $thissubPage="derm"
?>

</head>
<body>
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">
    
    <div class="contents">
      <div id="left_col">
        <div class="inner">
          <p id="more">Experience PYXIS technology in action</p>
          <a id="worldview-small" class="download-button" href="downloads.php">GeoWeb Browser<br/>Download<span class="background"></span></a>
          <a id="streamserver" class="download-button" href="geoweb-streamserver.php">GeoWeb Streamserver<br/>publish your Geospacial data<span class="background"></span></a>
          
        </div>
      </div>
      <div id="main_content">
        
        <h1>PYXIS Digital Earth<br/>Reference Model<br/><span id="subtitle">geometric coordinate system & digital indexing on an optimized discrete global grid</span></h1>
        <div id="intro">
          <p>Integrating geographic information on-demand is a critical challenge. The PYXIS DERM&#153 provides the solution.</p>
          <ul>
            <li>The PYXIS DERM&#153 is a data structure that renders disconnected silos of data into ubiquitous and searchable assets, on-the-fly.
            <li>The PYXIS DERM&#153 is a powerful Earth reference that overcomes the bottleneck of GIS pre-processing when real-time geospatial data integration is required.
            <li>The PYXIS DERM&#153 is a virtual spreadsheet of cells over the Earth with special properties to aggregate and decompose information provided by anyone, available to everyone, describing anything.
          </ul>
          <p><a href="worldview-geoweb-browser.php">Put the power of digital behind your geospatial application now >></a></p>
        </div>
        <div id="features">
          <h2>Features</h2>
          <ul>
            <li>A global reference model uniform over the entire planet at any resolution;</li>
            <li>Fast, seamless integration of multiple datasets on-demand - regardless of scale, origin, resolution, legacy formats, data, projection;</li>
            <li>Error-free two-way conversion with any conventional geometric coordinates allows legacy formats to be preserved entirely;</li>
            <li>Meaningful use of data stored at different resolutions without the need to conflate between scales, from continents to birdbaths;</li>
            <li>Database ready for complex multi-source spatial analysis using simple set theory; </li>
            <li>A generalized discrete global grid optimized for multi-source data simulation and modeling.</li>
            
          </ul>
        </div>
        <div id="frame">
          <div id="inner-top">
            <div id="inner-bottom">
              <img src="images/DERM-image-.jpg" /><!--image width is 668 px -->
              <div id="frame_content"><!--<p>content text goes here</p>--></div>
              <!-- use this div to put any text that is in the image for indexing -->
              <div id="hidden">
                <ul>
                  <li>Database</li>
                  <li>Coverage</li>
                  <li>Feature</li>
                  <li>Bitmap</li>
                  <li>Text</li>
                </ul>
                <p>Pyxis Digital Earth Reference Model&#153</p>
                <ul>
                  <li>Database</li>
                  <li>Coverage</li>
                  <li>Feature</li>
                  <li>Bitmap</li>
                  <li>Text files and feeds</li>
                  <li>Sensor Webs</li>
                </ul>
                
              </div>
            </div>
          </div>
        </div>
        
        
      </div>
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
