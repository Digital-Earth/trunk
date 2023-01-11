<!DOCTYPE html>
<html>
<head>
<title>the PYXIS innovation | Decision-Makers | Our Executive and Senior Management Team</title>
<?php require "../Content/Templates/meta.php" ?>
</head>
<body>
<?php require "../Content/Templates/header.php" ?> 

<div class="team-img">
	<h1 class="center">Management<br>Team</h1>
</div>
<div class="about-message">
	Our experience has shaped us with two shared beliefs that drive us every day:<br>We need to make a difference and we disdain mediocrity.
</div>

<div class="team-list">

<?php
$root = json_decode(file_get_contents("People/team.json"));
//var_dump($people->People);

foreach($root->People as $person)
{
	if(!isset($person->Bio) || !isset($person->Title) || $person->Title == "")
		continue;
		
	echo "<div class='team-person-link'>";
	echo "<a class='nolink' href='Team/Management#".$person->Id."'>";
	echo "<span class='title'>".$person->Title."</span><span class='name'>".$person->Name;
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
	if(!isset($person->Bio) || !isset($person->Title) || $person->Title == "")
		continue;

	echo "<div class='team-person'>";
	
	if(!isset($person->PictureURL))
		$person->PictureURL="dummy.jpg";
	echo "<img src='Content/Images/People/".$person->PictureURL."' />";
	
    echo "<h2 id='".$person->Id."'>".$person->Name;
	foreach($person->PostNominals as $title)
		echo ", ".$title;
	echo "</h2><h2>".$person->Title."</h2>";
	
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
