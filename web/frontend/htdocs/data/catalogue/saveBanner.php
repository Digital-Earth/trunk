<?php
include 'imageTools.php';

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type');

$maxImageFileSize = 4000000;
$minThumbnailWidth = 1320;
$minThumbnailHeight = 168;
$uploadDir = '../../images/galleries/';

processImage($uploadDir, $maxImageFileSize, $minThumbnailWidth, $minThumbnailHeight);
?>
