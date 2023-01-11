<?php 

  $title = "PIPE";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="technology"; 
  $thissubPage="pipe"
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
        
        <h1>PYXIS Information<br/>Processing Engine<br/><span id="subtitle">mash-up engine for multi-source geospatial content</span></h1>
        <div id="intro">
          <p>Transforming streams of geospatial content into innovative insights requires substantial creativity. The PYXIS Information Processing Engine&#153 makes it rewarding.</p>
          <ul>
            <li>The PIPE&#153 is a pipeline
   architecture for building advanced data processes such as
   transformations used in spatial analysis, image processing and
   simulations;</li>
            <li>The PIPE&#153 is a graphical user interface for
   building and publishing geospatial data mash-ups by aggregating
   geospatial data sources and services;</li>
            <li>The PIPE&#153 is a cloud
   computing work flow application that will manage geospatial
   information streams over distributed data sources, stores and computer
   processors.</li>
          </ul>
        <p><a href="downloads.php">Create a simple mash-up now >></a></p>
        </div>
        <div id="features">
            <h2>Features</h2>
            <ul>
              <li>A mechanism to access and combine multiple sources of geodata;</li>
              <li>Image processing algorithms for standard sampling;</li>
              <li>Boolean query wizard to customize based on geospatial data attributes;</li>
              <li>Extensible through pipeline process editor.</li>
              
            </ul>
          </div>
        <div id="frame">
          <div id="inner-top">
            <div id="inner-bottom">
              <img src="images/PIPE-image.jpg" /><!--image width is 668 px -->
              <div id="frame_content"><!--<p>content text goes here</p>--></div>
              <!-- use this div to put any text that is in the image for indexing -->
              <div id="hidden">
                <p>Search and Discover<br/>Have you ever wondered: ...What is HERE?</p>
                <p>UNDERSTAND & ILLUMINATE<br/>...What's going on? Where is this leading?
                </p>
                <p>ORGANIZE & SHARE<br/>...How can I contribute and engage others?
                </p>
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
