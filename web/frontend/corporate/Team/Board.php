<!DOCTYPE html>
<html>
<head>
<title>the PYXIS innovation | Thought Leaders | Our Board of Directors and Senior Technical Advisors</title>
<?php require "../Content/Templates/meta.php" ?>
</head>
<body>
<?php require "../Content/Templates/header.php" ?> 
<div class="team-img">
	<h1 class="center">Board &amp;<br>Advisors</h1>
</div>
<div class="about-message">Responsibility is an action word at the PYXIS innovation&#8482;.</div>

<div class="team-list">

<?php
$root = json_decode(file_get_contents("People/team.json"));
//var_dump($people->People);

foreach($root->People as $person)
{
	if(!isset($person->Bio) || !isset($person->BoardTitle) || $person->BoardTitle == "")
		continue;
		
	echo "<div class='team-person-link'>";
	echo "<a class='nolink' href='Team/Board#".$person->Id."'>";
	echo "<span class='title'>".$person->BoardTitle."</span><span class='name'>".$person->Name;
	foreach($person->PostNominals as $title)
		echo ", ".$title;
	echo "</span>";
	echo "</a>";
	echo "</div>";
}

?>

<div class='team-person-seperator'></div>
</div>

<?php

foreach($root->People as $person)
{
	if(!isset($person->Bio) || !isset($person->BoardTitle) || $person->BoardTitle == "")
		continue;

	echo "<div class='team-person'>";
	
	if(!isset($person->PictureURL))
		$person->PictureURL="dummy.jpg";
	echo "<img src='Content/Images/People/".$person->PictureURL."' />";
	
    echo "<h2 id='".$person->Id."'>".$person->Name;
	foreach($person->PostNominals as $title)
		echo ", ".$title;
	echo "</h2><h2>".$person->BoardTitle."</h2>";
	
	echo "<div class='bio'>";	
	
	foreach($person->Bio as $bio)
		echo "<p>".$bio."</p>";
	
	echo "</div>";
	
	echo "</div>";
}      
?>

<?php require "../Content/Templates/footer.php" ?>
<?php require "../Content/Templates/scripts.php" ?>
</body>
</html>
