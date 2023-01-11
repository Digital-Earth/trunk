
  
  <div id="header-wrapper">
    <div class="contents">
      <div id="header">
        <h1><a href="index.php">the PYXIS innovation<br/>commonground for digitalearth</a><span><a href="index.php"></a></span></h1>
        <!--<p id="top-menu"><a href="#">terms</a> <span>|</span> <a href="#">privacy</a> <span>|</span> <a href="contact.php">contact</a></p>-->
        
        <div id="search">
          <form action="/search-results.php" id="cse-search-box">
            <div>
              <input type="hidden" name="cx" value="000905071606433559325:1hw5myuxunc" />
              <input type="hidden" name="cof" value="FORID:10" />
              <input type="hidden" name="ie" value="UTF-8" />
              <input id="searchBox" type="text" name="q" />
              <input id="searchButton" type="submit" name="sa" value="Search" />
            </div>
          </form>
        </div>
<script type="text/javascript" src="http://www.google.com/coop/cse/brand?form=cse-search-box&lang=en&sitesearch=true"></script>
        
      </div>
      
      <ul id="nav">
        <li <?php if ($thisPage=="home") echo " class=\"current\" "; ?>><a href="index.php">Home</a></li>
        <li <?php if ($thisPage=="products") echo " class=\"current\" "; ?>><a class="nolink" href="#">Products</a>
          <ul>
            <li><a <?php if ($thissubPage=="worldview") echo " class=\"currentsubpage\" "; ?> href="worldview-geoweb-browser.php">WorldView<span class="sup">TM</span></a></li>
            <li><a <?php if ($thissubPage=="streamserver") echo " class=\"currentsubpage\" "; ?> href="geoweb-streamserver.php">GeoWeb<span class="sup">TM</span> StreamServer</a></li>
            <!--<li><a <?php if ($thissubPage=="earth-engine") echo " class=\"currentsubpage\" "; ?> href=".php">Earth Engine<span class="sup">TM</span> SDE</a></li>-->
          </ul>
        </li>
        
        <li <?php if ($thisPage=="technology") echo " class=\"current\" "; ?>><a class="nolink" href="#">Technology</a>
          <ul>
              <li><a <?php if ($thissubPage=="derm") echo " class=\"currentsubpage\" "; ?> href="derm.php">DERM</a></li>
              <li><a <?php if ($thissubPage=="pipe") echo " class=\"currentsubpage\" "; ?> href="pipe.php">PIPE</a></li>
              <li><a <?php if ($thissubPage=="pyxnet") echo " class=\"currentsubpage\" "; ?> href="pyxnet.php">PyxNet</a></li> 
          </ul>
        </li>
        
        <li <?php if ($thisPage=="support") echo " class=\"current\" "; ?>><a class="nolink" href="#">Support</a>
          <ul>
	    <li><a href="http://www.pyxisinnovation.com/pyxwiki/index.php?title=FAQ">FAQ</a></li>
            <li><a <?php if ($thissubPage=="highway") echo " class=\"currentsubpage\" "; ?> href="http://www.pyxisinnovation.com/pyxwiki/index.php?title=Main_Page">WIKI</a></li>
            <li><a href="http://www.pyxisinnovation.com/pyxwiki/index.php?title=How_PYXIS_Works">How PYXIS Works</a></li>
          </ul>
        </li>
        
        <li <?php if ($thisPage=="company") echo " class=\"current\" "; ?>><a class="nolink "href="#">Company</a>
          <ul>
            <li><a <?php if ($thissubPage=="news") echo " class=\"currentsubpage\" "; ?> href="news-events.php">News</a></li>
            <!--<li><a <?php if ($thissubPage=="careers") echo " class=\"currentsubpage\" "; ?> href="careers.php">Careers</a></li>-->
            <li><a <?php if ($thissubPage=="contact") echo " class=\"currentsubpage\" "; ?> href="contact.php">Contact</a></li>
          </ul>
        </li>
        <li <?php if ($thisPage=="downloads") echo " class=\"current\" "; ?>><a href="downloads.php" class="link">Downloads</a>
        </li>
      
      </ul>
      
     </div>
   </div>
