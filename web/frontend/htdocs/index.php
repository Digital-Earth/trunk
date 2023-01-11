<?php 

  $title = "home";
  $description = "PYXIS | ";
  $keywords = "";
  include('includes/head.php');
  
?>
<?php $thisPage="home"; 
  $thissubPage=""
?>

</head>
<body class="home">
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">
    <img id="home-bg" src="images/content-bg.jpg" alt="PYXIS WolrdView" />
    <div class="contents">
      <div id="banner">
        <div id="inner">
          <h1>WorldView&#153</h1>
          <p>Our Globe, Your World...<br/>Explore the GeoWeb, make decisions, share with others.<br/>Click <a href="worldview-geoweb-browser.php">here</a> to learn more and get started.</p>
        </div>
      </div>
      
      <div id="content-top">
        
        <div id="buttons">
          <a href="downloads.php" id="worldview" class="download-button">Worldview<span>TM</span> GeoWeb Browser<br/>Free Download<span class="background"></span></a>
          <a href="geoweb-streamserver.php" id="streamserver" class="download-button">GeoWeb StreamServer<span>TM</span><br/>coming soon!<span class="background"></span></a>
        </div>
        <div id="intro"> 
          
          <p>Everyone can participate in the next digital media experience with the PYXIS innovation's&#153 suite of GeoWeb technologies and products. You can navigate and explore Digital Earth using location-based search, complete sophisticated geographic data analysis, develop mash-ups and applications, or stream geospatial content into the Internet. <br/><br/>Welcome to a Common Ground for Digital Earth.</p>
        </div>
      </div>
      
      <div id="site_sections">
        <div class="section">
          <h2>GeoWeb</h2>
          <p>Like the World Wide Web, the Geospatial Web is a digital medium designed to allow rapid distribution of content from many sources. The GeoWeb is also a participatory Web - everyone can contribute. While the Web uses text to organize content, the GeoWeb uses Earth location.</p>
          <a href="docs/What_is_the_GeoWeb_modified.pdf">Learn more about the GeoWeb >></a>
        </div>
        
        <div class="section">
          <h2>PYXIS</h2>
          <p>the PYXIS innovation&#153 brings a world full of information right from content providers directly to the desktop of data hungry decision makers. Our breakthrough technologies allow the GeoWeb to work; our products allow the GeoWeb to work for you. We power the GeoWeb; we have since the beginning.</p>
          <a href="derm.php">Learn more about PYXIS technology >></a>
        </div>
        
        <div class="section last">
          <h2>And You</h2>
          <p>Your world is complex. Your decisions are important. Information is critical to your success. You have value to contribute. Find content quickly, share it seamlessly and make decisions in real-time.<br/> <br/><a href="downloads.php">Download WorldView&#153 now</a> and experience the power of the GeoWeb.</p>
          
        </div>    
      </div>
      
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
