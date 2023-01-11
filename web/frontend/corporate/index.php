<!DOCTYPE html>
<html>
<head>
<title>the PYXIS innovation</title>
<?php require "./Content/Templates/meta.php" ?>
</head>
<body>
<?php require "./Content/Templates/header.php" ?>
<div class="load-fade">
<div class="main-img">
	<div class="animation">
		<div class="white-circle" style="top:200px;left:200px;"><div class="inner"></div></div>
	</div>
	<div class="content">
		<h1 class="center">There is a World of <br> Information Flowing to You</h1>
		<div class="center normal">Within millions of organizations, silos of data that describe places, things, and events are growing and growing. The WorldView&#8482; Platform provides an unprecedented ability to release the value of this data helping individuals and organizations to make better decisions.  Search, combine, analyze and share the world’s information on-demand.  It is the next-generation Web experience.</div>
		<div class="center welcome">Welcome to Digital Earth.</div>
		<div class="center button-div"><a href="https://worldview.gallery" class="button nolink">SEARCH EARTH NOW</a></div>
	</div>
</div>

<div class="main-features">
	<a href="Products/Studio#search" class="search">Search</a>
	<a href="Products/Studio#combine" class="combine">Combine</a>
	<a href="Products/Studio#analyze" class="analyze">Analyze</a>
	<a href="Products/Studio#share" class="share">Share</a>
</div>

<div class="main-question">
	<div class="background first"></div>	
	<div class="background second"></div>	
	<div class="content">
		<div class="title"><span>Earth: a Medium that Delivers Answers</span></div>
		<div class="question first"></div>
		<div class="question second"></div>
		<div class="question-selectors"></div>
	</div>
</div>

<div class="main-question-footer">
Digital Earth is all about asking questions.  “Where is it?” and “What is here?”<br>The WorldView&#8482; Platform communicates complex ideas to support informed and evidence-based decision-making.
</div>

<div class="main-platform">
	<div class="title"><span>The WorldView&#8482; Platform</span></div>
	<div>the PYXIS innovation&#8482; solves the complexity of using geospatial data so it is easy for anyone to participate.</div>
</div>


<div class="products">
	<a href="Products/Studio" class="product nolink">
		<div class="name">WorldView&#8482; Studio</div>
		<div class="icon studio"></div>
		<div class="description">Support your decisions with information on a globe.</div>		
		<div class="learn">LEARN MORE</div>
	</a>
	<div class="product-space"></div>	
	<a href="Products/Gallery" class="product nolink">
		<div class="name">WorldView&#8482; Gallery</div>
		<div class="icon gallery"></div>
		<div class="description">Deliver location-based content to more customers.</div>		
		<div class="learn">LEARN MORE</div>
	</a>
	<div class="product-space"></div>
	<a href="Products/Channel" class="product nolink">
		<div class="name">WorldView&#8482; Channel</div>
		<div class="icon channel"></div>
		<div class="description">Organize your enterprise’s geospatial assets.</div>		
		<div class="learn">LEARN MORE</div>
	</a>
</div>
<?php require "./Content/Templates/footer.php" ?>
</div>
<?php require "./Content/Templates/scripts.php" ?>
<script src="./Scripts/jquery.parallax-1.1.3.js" type="text/javascript"></script>
<script type="text/javascript">
	/******************************************************************************/
	/*********************************** EASING ***********************************/
	/******************************************************************************/

	(function() {

	// based on easing equations from Robert Penner (http://www.robertpenner.u/easing)

	var baseEasings = {};

	$.each( [ "Quad", "Cubic", "Quart", "Quint", "Expo" ], function( i, name ) {
		baseEasings[ name ] = function( p ) {
			return Math.pow( p, i + 2 );
		};
	});

	$.extend( baseEasings, {
		Sine: function( p ) {
			return 1 - Math.cos( p * Math.PI / 2 );
		},
		Circ: function( p ) {
			return 1 - Math.sqrt( 1 - p * p );
		},
		Elastic: function( p ) {
			return p === 0 || p === 1 ? p :
				-Math.pow( 2, 8 * (p - 1) ) * Math.sin( ( (p - 1) * 80 - 7.5 ) * Math.PI / 15 );
		},
		Back: function( p ) {
			return p * p * ( 3 * p - 2 );
		},
		Bounce: function( p ) {
			var pow2,
				bounce = 4;

			while ( p < ( ( pow2 = Math.pow( 2, --bounce ) ) - 1 ) / 11 ) {}
			return 1 / Math.pow( 4, 3 - bounce ) - 7.5625 * Math.pow( ( pow2 * 3 - 2 ) / 22 - p, 2 );
		}
	});

	$.each( baseEasings, function( name, easeIn ) {
		$.easing[ "easeIn" + name ] = easeIn;
		$.easing[ "easeOut" + name ] = function( p ) {
			return 1 - easeIn( 1 - p );
		};
		$.easing[ "easeInOut" + name ] = function( p ) {
			return p < 0.5 ?
				easeIn( p * 2 ) / 2 :
				1 - easeIn( p * -2 + 2 ) / 2;
		};
	});

	})();


	$(document).ready(function(){
		
		var initAllAnimations = function() {
			var img = new Image();
			var canvas = $('<canvas/>')[0];
			
			img.onload = function() {
				$('.main-img').addClass('ready');
				canvas.width = img.width;
				canvas.height = img.height;
				canvas.getContext('2d').drawImage(img, 0, 0, img.width, img.height);
										
				$('.main-img').find('.animation').animate({'top':'0px'},2000,'easeOutQuart',function() {
					$('.main-img').find('.animation').parallax("50%", 1, false, true);
					
					startNewCircle(200);
					startNewCircle(600);
					startNewCircle(1000);
					startNewCircle(1500);
					startNewCircle(2200);
					startNewCircle(3000);
				});
			}
			
			var height = $('.main-img').height();
			var clone = $('.main-img').find('.animation').find('.white-circle');
			
			var startNewCircle = function(delay) {
				var isPinkish = false;
				
				var newCircle = clone.clone();
				
				//try to random the circles where it visible 
				var offset = -$('.main-img').find('.animation').position().top;
				offset = Math.min(offset,img.height-height);
				
				while(!isPinkish) {
					var cord = { x: Math.floor(Math.random()*img.width),
								 y: Math.floor(Math.random()*height+offset) };
								 
					var pixelData = canvas.getContext('2d').getImageData(cord.x, cord.y, 1, 1).data;
					
					isPinkish = Math.abs(pixelData[0]-pixelData[1]) > 20;
					
					if (isPinkish) {			
						$('.main-img').find('.animation').append(newCircle);
						newCircle.css({'top':cord.y+"px",'left':cord.x+"px"});
						break;
					}
				}
				
				window.setTimeout(function(){		
					var pos = newCircle.position();
					var maxSize = Math.floor(Math.random()*80)+20;
					newCircle.animate({opacity:'1'},
						{
							duration:700,
							easing:'easeOutExpo',
							step:function(p) {
								var d = Math.round(p*maxSize);
								newCircle.css({'top':(pos.top-d)+"px",'left':(pos.left-d)+"px",'padding':d+"px"});
							}
						}).delay(1000).fadeOut(2000,function() {
							newCircle.remove();
							startNewCircle(200+Math.floor(Math.random()*1000));
						});
					},delay);
			}
			
			img.src = "Content/Images/main_bg_parallax.jpg";
		}
				
		var allQuestions = [
			{
				text: [
					"What are the number, types and average income<br>of the farms in this watershed?",
					"What type of eco friendly purchasers live<br>within 20 kilometers of my business?",
					"Where in north america can I find places with<br>the ideal growing conditions for truffles?"
					],
				image: "Content/Images/slider_bg_wheat.jpg"
			},
			{ 
				text: [
					"What is the risk of developing this land for commercial use?",
					"What are the results of the hurricane<br>on property and personal loss?"
					],
				image: "Content/Images/slider_bg_home.jpg"
			},
			{ 
				text: [
					"Where can I find a new source of paint in town<br>as our critical supplier has had a fire?",
					"Where is the best place to locate my new business?",
					"Where in my town are the best rental rates<br>in an area of high income and low unemployment?",
					],
				image: "Content/Images/slider_bg_work.jpg"
			},
			{ 
				text: [
					"What happened in my neighbourhood yesterday?",
					"Where are there other places on the globe<br>that this crime might be committed?"
					],
				image: "Content/Images/slider_bg_crime.jpg"
			},
			{ 
				text: [
					"What ground water recharge is necessary to bring<br>the water levels up to normal?",
					"Where are the best lakes near a road for fishing<br>speckled trout in June?",
					"What conditions have changed within the watershed<br>over the past 10 years?"
					],
				image: "Content/Images/slider_bg_water.jpg"
			}
		];
		
		var questions = [];
		
		for(var i=0;i<5;i++){
			var j = Math.min(Math.floor( Math.random() * ( allQuestions.length ) ) , allQuestions.length-1); 
			var q = allQuestions[j];
			q.text = q.text[Math.min(Math.floor( Math.random() * ( q.text.length ) ) , q.text.length - 1)];
			questions.push(q);
			allQuestions.splice(j,1);
		}
		
		var qArea = $(".main-question");
		var qBackground = qArea.find(".background.first");
		var qBackground2 = qArea.find(".background.second");
		var qText = qArea.find(".question.first");
		var qText2 = qArea.find(".question.second");
		var qSelectors = qArea.find(".question-selectors");
		
		var selectedQuestion = -1;
		
		var timeoutId = undefined;
		var timoutFunc = function() {
			selectQuestion((selectedQuestion+1) % questions.length);
		}
				
		var selectQuestion = function(i) {
			var q = questions[i];
			
			window.clearTimeout(timeoutId);
			timeoutId = window.setTimeout(timoutFunc,7000);
			
			qSelectors.find(".selector").removeClass('active');			
			qSelectors.find(".selector:eq("+i+")").addClass('active');
			
			if (selectedQuestion == i ) {
				return;
			}
			
			var changeImageFunc = function() {			
				if (selectedQuestion == -1) {
					//simply set the text
					qText.html(q.text);
					qBackground.css({'background-image':'url('+q.image+')','opacity':0});
					qBackground.animate({'opacity':1},500);
				} else {
					//make nice animation
					qText.animate({left:'150%'}, 2000, 'swing', function() { 
						qText.html(q.text);
						qText.css('left','50%');
					});
					qText2.html(q.text);
					qText2.animate({left:'50%'},2000, 'swing' , function() {
						qText2.css('left','-50%');
					});
					qBackground.animate({'opacity':0,'left':'+=175px'}, 2000, 'swing' , function() {
						qBackground.css({'background-image':'url('+q.image+')','opacity':1,'left':'-75px'});
					});
					qBackground2.css({'background-image':'url('+q.image+')','opacity':0,'left':'-250px'});					
					qBackground2.animate({'opacity':1,'left':'+=175px'}, 2000, 'swing', function () {
						window.setTimeout(function() {
							//we hide qBackground2 just little after to avoid flickering
							qBackground2.css({'opacity':0,'left':'-150px'});
						},100);
					} );
				}
				q.loaded = true;
			};
			
			if (q.loaded) { 
				changeImageFunc(); 
			} else {
				var tempImage = new Image();
				tempImage.onload = changeImageFunc;
				tempImage.src = q.image;
			}
			
			selectedQuestion = i;
		}
		
		var createSelector = function(i) {
			var selector = $("<span class='selector'></span>");			
			selector.click(function() {
				selectQuestion(i);
			});
			qSelectors.append(selector);
		}
		
		for(var i=0;i<questions.length;i++) {
			createSelector(i);
		}
		
		selectQuestion(0);
		
		$(".load-fade").animate({opacity:1},500,function() {
			initAllAnimations();
		});
	});
	
</script>
</body>
</html>
