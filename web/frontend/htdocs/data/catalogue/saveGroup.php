<?php
include 'imageTools.php';

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type');

$maxImageFileSize = 4000000;
$minThumbnailWidth = 300;
$minThumbnailHeight = 300;
$uploadDir = '../../images/groups/';

processImage($uploadDir, $maxImageFileSize, $minThumbnailWidth, $minThumbnailHeight);
?>
