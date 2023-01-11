<?php
include 'imageTools.php';

header('Access-Control-Allow-Origin: *'); 
header('Access-Control-Allow-Headers: Content-Type');

$maxImageFileSize = 4000000;
$minThumbnailWidth = 220;
$minThumbnailHeight = 0;
$uploadDir = '../../images/pipelines/';

processImage($uploadDir, $maxImageFileSize, $minThumbnailWidth, $minThumbnailHeight);
?>
