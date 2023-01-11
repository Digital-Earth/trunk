<?php
include 'imageTools.php';

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type');

$maxImageFileSize = 4000000;
$minThumbnailWidth = 96;
$minThumbnailHeight = 96;
$uploadDir = '../../images/avatars/';

processImage($uploadDir, $maxImageFileSize, $minThumbnailWidth, $minThumbnailHeight);
?>
